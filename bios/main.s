; loads at $8000

ready=$0

start:
  set #$80
vcheck:
  ; wait for video to turn on
  movc a, v_STATUS
  and a, #$01
  beq start
memcheck:
  ; check memory
  mov x, #$0
  mov a, #$ffffffff
  mov (x), a
  mov a, (x)
  cmp a, #$ffffffff
  bne memfail
  mov a, #$0
  mov (x), a
  inc x
  jmp memcheck
memfail:
  brk
memdone:
  mov a, memok
  jsr puts
devpoll:
  ; check all 4 devices
  mov w, #$00
devloop:
  movc x, #$00
  mov a, #$01 ; identify
  movc core_PARWRITE+w, a
devident:
  ; read status
  inc x
  beq devskip
  movc a, core_PARREAD+w
  cmp a, #$11 ; signature
  bne devident
  ; code for successful ident here
  inc ready0+w
devskip:
  inc w
  cmp w, #$04
  bne devloop
trydev:
  mov w, #$00
  ; check available media, and try to
  ; boot from them
  jsr checkdsk
  and a, #$ffffffff
  beq tryfail
  jmp loaddsk
tryfail:
  inc w
  cmp w, #$04
  bne trydev
nodev:
  mov a, prompt
  jsr puts
  hlt

loaddsk:
  ; must load bootloader, kill stack and
  ; go there
  jmp $f000

checkdsk:
  ; checks disk if bootable
  ; wait for ready
  movc a, core_PARSTAT+w
  andc a, #$01
  beq checkdsk
  movc a, #$02 ; request seek
  movc core_PARWRITE+w, a
waitack:
  movc a, core_PARSTAT+w
  andc a, #$08
  beq waitack
  movc a, #$00
  movc core_PARWRITE+w, a
check1:
  ; check status
  movc a, core_PARSTAT+w
  ; error?
  bitc a, #$02
  bne return0
  ; ready?
  bitc a, #$01
  beq check1
  ; success. code for checking bootable
  ; to be added
  mov a, #$01
  ret
return0:
  ; failure
  mov a, #$00
  ret

puts:
  ; put string. a=pointer
  movc b, (a)
  cmpc b, #$0
  beq puts_end
  movc ch_PUTCHAR, b
  inc a
  bra puts
puts_end:
  ret

prompt:
  .char "Please insert OS disk, or press any key to launch prompt..."

errorD:
  .char "Error in disk "

errorE:
  .char "Processor raised exception"

errorR:
  .char "Error during read of disk "

errorM:
  .char "Error in memory block "

memok:
  .char "memory pages OK"


