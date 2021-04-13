# nulls the timestamp filed in a windows .lib archive,
# making the lib reproducable.
# the time is the TimeDateStamp in the COFF file header, four bytes at offset 4
# See https://blog.conan.io/2019/09/02/Deterministic-builds-with-C-C++.html
# also: https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#archive-library-file-format
#
# There are some additional fixes added for reproducability, such as fixing the zero-padding of names in the coff
# section headers.

import sys
import struct

verbose = True

libheader = b"!<arch>\n"


def main():
    infilename = sys.argv[1]
    if len(sys.argv) > 2:
        outfilename = sys.argv[2]
    else:
        outfilename = infilename

    with open(infilename, "rb") as fp:
        lib = read_lib(fp)
    strip_lib_timestamp(lib)
    with open(outfilename, "wb") as fp:
        write_lib(fp, lib)


def read_lib(fp):
    """
    read microsoft .lib file,
    """
    # lib file header
    h = fp.read(len(libheader))
    assert h == libheader

    # read first and second link members
    h1 = header_read(fp)
    p = fp.tell()
    if verbose:
        print("header", h1)
    m1 = first_lm_read(fp)
    assert fp.tell() - p == h1["size"]
    if verbose:
        print("first linker member", m1)

    h2 = header_read(fp)
    if verbose:
        print("header", h2)
    p = fp.tell()
    m2 = second_lm_read(fp)
    assert fp.tell() - p == h2["size"]
    if verbose:
        print("second linker member", m2)

    result = {
        "h1": h1,
        "m1": m1,
        "h2": h2,
        "m2": m2,
        "hl": None,
        "longnames": [],
        "ho": [],
        "o": [],
    }

    # now we might have an optional longnames member
    h = header_read(fp)
    if not h:
        return result

    if h["name"] == "//":
        result["hl"] = h
        p = fp.tell()
        while fp.tell() < p + h["size"]:
            result["longnames"].append(readcstr(fp))
        if verbose:
            print("header", h)
            print("longnames", result["longnames"])
        h = None

    # now read the headers, possibly we alread read one above.
    while True:
        if h is None:
            h = header_read(fp)
            if h is None:
                return result

        result["ho"].append(h)
        result["o"].append(fp.read(h["size"]))
        if verbose:
            print("header:", result["ho"][-1])
            print("coff length:", len(result["o"][-1]))
        h = None

    return result


def write_lib(fp, lib):
    fp.write(libheader)
    header_write(fp, lib["h1"])
    first_lm_write(fp, lib["m1"])
    header_write(fp, lib["h2"])
    second_lm_write(fp, lib["m2"])

    if lib["hl"]:
        header_write(fp, lib["hl"])
        for s in lib["longnames"]:
            writecstr(fp, s)

    for h, c in zip(lib["ho"], lib["o"]):
        header_write(fp, h)
        fp.write(c)


def strip_lib_timestamp(lib):
    def fix_header(h):
        h["date"] = "-1"

    fix_header(lib["h1"])
    fix_header(lib["h2"])
    if lib["hl"]:
        fix_header(lib["hl"])
    for h in lib["ho"]:
        fix_header(h)
    lib["o"] = [strip_coff_timestamp(c) for c in lib["o"]]
    lib["o"] = [fix_coff_null_padding(c) for c in lib["o"]]


def header_read(fp):
    """
    read a header entry from a microsoft archive
    """

    # header can start with optional newline
    optnl = read_optional_nl(fp)

    name = fp.read(16)
    if len(name) < 16:
        return None  # eof
    name = name.decode("ascii").strip()
    date = fp.read(12).decode("ascii").strip()
    uid = fp.read(6).decode("ascii").strip()
    gid = fp.read(6).decode("ascii").strip()
    mode = fp.read(8).decode("ascii").strip()
    size = fp.read(10).decode("ascii").strip()
    size = eval(size)
    eoh = fp.read(2)
    assert eoh == b"\x60\x0a"
    return {
        "optnl": optnl,
        "name": name,
        "date": date,
        "uid": uid,
        "gid": gid,
        "mode": mode,
        "size": size,
    }


def header_write(fp, h):
    def writestr(s, n):
        """helper to write space padded string of fixed length"""
        e = s.encode("ascii") + b" " * n
        fp.write(e[:n])

    if h["optnl"]:
        fp.write(h["optnl"])
    writestr(h["name"], 16)
    writestr(h["date"], 12)
    writestr(h["uid"], 6)
    writestr(h["gid"], 6)
    writestr(h["mode"], 8)
    writestr(str(h["size"]), 10)
    fp.write(b"\x60\x0a")


def first_lm_read(fp):
    nos = fp.read(4)
    nos = struct.unpack(">L", nos)[0]  # unsigned long, big-endian

    offsets = []
    strings = []
    for i in range(nos):
        offset = fp.read(4)
        offsets.append(struct.unpack(">L", offset)[0])
    for i in range(nos):
        strings.append(readcstr(fp))
    return {"offsets": offsets, "strings": strings}
    # sometimes there is an extra \0a after the strings
    p = peek(fp)
    return zip(offsets, strings)


def first_lm_write(fp, lm):
    nos = len(lm["offsets"])
    fp.write(struct.pack(">L", nos))
    for o in lm["offsets"]:
        fp.write(struct.pack(">L", o))
    for s in lm["strings"]:
        writecstr(fp, s)


def second_lm_read(fp):
    # number of members
    m = struct.unpack("<L", fp.read(4))[0]  # unsigned long, big-endian
    offsets = []
    for i in range(m):
        offsets.append(struct.unpack("<L", fp.read(4))[0])

    # number of symbols
    n = struct.unpack("<L", fp.read(4))[0]  # unsigned long, big-endian
    indices = []
    for i in range(n):
        indices.append(struct.unpack("<H", fp.read(2))[0])  # unsigned short
    strings = []
    for i in range(n):
        strings.append(readcstr(fp))

    return {"offsets": offsets, "indices": indices, "strings": strings}


def second_lm_write(fp, lm):
    m = len(lm["offsets"])
    fp.write(struct.pack("<L", m))
    for o in lm["offsets"]:
        fp.write(struct.pack("<L", o))
    n = len(lm["indices"])
    fp.write(struct.pack("<L", n))
    for i in lm["indices"]:
        fp.write(struct.pack("<H", i))
    for s in lm["strings"]:
        writecstr(fp, s)


def readcstr(f):
    buf = []
    while True:
        b = f.read(1)
        if b is None or b == b"\0":
            return b"".join(buf)
        else:
            buf.append(b)


def writecstr(f, s):
    f.write(s)
    f.write(b"\0")


def read_optional_nl(fp):
    t = fp.tell()
    c = fp.read(1)
    if c == b"\n":
        return c
    else:
        fp.seek(t)


def peek(fp):
    """ check the next char """
    t = fp.tell()
    c = fp.read(1)
    fp.seek(t)
    return c


def strip_coff_timestamp(coff, timestamp=0):
    old = struct.unpack("<L", coff[4:8])[0]
    if timestamp != old:
        ts = struct.pack("<L", timestamp)
        coff = coff[:4] + ts + coff[8:]
        if verbose:
            print("replaced coff timestamp %r with %r" % (old, timestamp))
    return coff


def fix_coff_null_padding(coff):
    """
    Section headers in coff files start with a 8 byte null padded field.
    Some compilers don't set all the nulls to zero
    """
    header = coff[:20]
    n_sections = struct.unpack("<H", header[2:4])[0]
    sections = []
    for i in range(n_sections):
        # section headers start after header, each is 40 bytes
        start = 20 + i * 40
        sections.append(coff[start : start + 40])

    modified = False
    for n, s in enumerate(sections):
        name = s[:8]
        # find first null in name
        i = name.find(b"\0")
        if i >= 0:
            # everything after first null is null
            shortname = name[:i]
            namenew = (shortname + b"\0" * 8)[:8]
            if name != namenew:
                sections[n] = namenew + s[8:]
                modified = True
                if verbose:
                    print(
                        "Fixed null padding of COFF section header name %r" % shortname
                    )
    if modified:
        start = header + b"".join(sections)
        coff = start + coff[len(start) :]
    return coff


if __name__ == "__main__":
    main()
