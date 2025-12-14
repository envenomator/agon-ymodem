	.assume	adl=1
  .include "agon/mos.inc"
	.section	.text
  .global _sysvar_init
  .global _getbyte
  .global _getblock
  .global _putblock
  .text

; set up the sysvar pointer
; void receive_init(void)
; only trashes af, ix
_sysvar_init:
  push    ix
  ld      a, mos_sysvars
  rst.lil 08h             ; MOS call for mos_sysvars
  push    ix
  pop     hl              ; ld ix, hl
  ld      de, sysvar_vkeycount
  add     hl, de          ; HL now holds pointer to sysvar_vkeycount
  ld      (sysvar_vkeycount_ptr), hl

  ld      a, (hl)
  ld      (keycount), a

  pop     ix
  ret

; get a block of data from the VDP
; void getblock(char *buffer, uint24_t length)
; call _receive_init first
_getblock:
  push    ix
  ld      ix, 0
  add     ix, sp
  ld      bc, (ix+6)      ; pointer to buffer
  ld      de, (ix+9)      ; length
  ld      hl, (sysvar_vkeycount_ptr); HL now contains the pointer to sysvar_vkeycount, as it was stored in variable sysvar_vkeycount_ptr
  push    hl
  pop     ix
bufferloop:
  ld      a, (keycount)
byteloop:
  cp      (hl)
  jr      z, byteloop
  ld      a, (hl)
  ld      (keycount), a         ; store the new value of keycount
  ld      a, (ix+sysvar_keyascii-sysvar_vkeycount) ; Get the key code
  ld      (bc), a         ; store in buffer
  inc     bc              ; next buffer location
  dec     de              ; decrease counter
  ld      (zero_test), de ; Test DE for zero
  ld      a, (zero_test+2)
  or      d
  or      e
  jr      nz, bufferloop
  pop     ix
  ret

; put a block of data to the VDP
; void putblock(char *buffer, uint24_t length)
; call _receive_init first
_putblock:
  push    ix
  ld      ix, 0
  add     ix, sp
  ld      hl, (ix+6) ; pointer to buffer
  ld      de, (ix+9) ; get length
  ld      (zero_test), de ; Test DE for zero
  ld      a, (zero_test+2)
  or      d
  or      e
  jr      z, .end          ; if length == 0
  
.loop:
  ld      a, (hl)    ; get byte from buffer
  inc     hl         ; increment pointer in buffer
  rst.lil 10h        ; sent to VDP
  dec     de

  ld      (zero_test), de ; Test DE for zero
  ld      a, (zero_test+2)
  or      d
  or      e
  jr      nz, .loop

.end:
  pop     ix
  ret

_getbyte:
  push    ix
  ld      hl, (sysvar_vkeycount_ptr)
  push    hl
  pop     ix

  ld      a, (keycount)         ; wait for a change
get:
  cp      (hl)
  jr      z, get
  ld      a, (hl)
  ld      (keycount), a         ; store the new value of keycount
  ld      a, (ix+sysvar_keyascii-sysvar_vkeycount); get the byte
  pop     ix
  ret

  .data
keycount:
  .space 1
sysvar_vkeycount_ptr:  
  .space 3
zero_test:
  .space 3

