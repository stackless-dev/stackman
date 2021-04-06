; Assembly code for Windows.
; run either by ml.exe (32 bit) or ml64.exe

IFDEF RAX
  INCLUDE "platforms/switch_x64_msvc.asm"
ELSE
  INCLUDE "platforms/switch_x86_msvc.asm"
ENDIF