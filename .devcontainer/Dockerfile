# stackman development container
# Includes cross-compilation toolchains for all supported platforms

ARG VARIANT="jammy"
FROM mcr.microsoft.com/devcontainers/base:ubuntu-${VARIANT}

# Install build tools and cross-compilation toolchains
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
        build-essential \
        gcc-multilib \
        g++-multilib \
        gcc-arm-linux-gnueabi \
        g++-arm-linux-gnueabi \
        gcc-aarch64-linux-gnu \
        g++-aarch64-linux-gnu \
        gcc-riscv64-linux-gnu \
        g++-riscv64-linux-gnu \
        qemu-user \
        python3 \
        python3-pip \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*
