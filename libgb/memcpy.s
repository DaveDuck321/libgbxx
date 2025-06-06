	.text

.global memcpy
memcpy:								; @memcpy(hl = void *dst, bc = void const* src, de = size_t count)
	inc d								; Pre-increment MSB to avoid edge-casing 0

	ld a, e
	or a								; Is LSB already 0?
	jr z, .memcpy_msb_loop_entry
.memcpy_lsb_loop_entry:
	ld a, (bc)
	inc bc
	ldi (hl), a
	dec e
	jr nz, .memcpy_lsb_loop_entry
.memcpy_msb_loop_entry:
										; We've decremented the LSB to 0, decrement the MSB too
	dec d
	ret z
	jr .memcpy_lsb_loop_entry


.global memset
memset:								; @memset(hl = void *dst, bc = int byte, de = size_t count)
	inc d								; Pre-increment MSB to avoid edge-casing 0

	ld a, e
	or a								; Is LSB already 0?

	ld a, b								; From now on "byte" lives in a
	jr z, .memset_msb_loop_entry
.memset_lsb_loop_entry:
										; Happy loop: entirely 8-bit math, compare is free
	ldi (hl), a
	dec e
	jr nz, .memset_lsb_loop_entry
.memset_msb_loop_entry:
										; We've decremented the LSB to 0, decrement the MSB too
	dec d
	ret z
	jr .memset_lsb_loop_entry
