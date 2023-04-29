; specs2 Initial System Loader (ISL)
;
; this program loads the BIOS into system memory.

start:
  set i
  clr d
  mov y, #$8000
read:
  movc a, core_BIOSSTAT
  bmi done
  beq read
  movc b, core_BIOSREAD
  movc (y), b
  bra read
done:
  jmp $8000

; e5
; e9
; a1 06 00 80 00 00
; 22 00 9e 80 ff 00 
; c2 0d 00
; c0 f4 ff
; 22 01 9f 80 ff 00
; 2f 61
; cf e9 ff
; d0 00 80 00 00
