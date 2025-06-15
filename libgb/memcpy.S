	.text

.global memcpy
memcpy:								// @memcpy(hl = void *dst, bc = void const* src, de = size_t count)
	inc d								// Pre-increment MSB to avoid edge-casing 0

	ld a, e
	or a								// Is LSB already 0?
	jr z, .Lmemcpy_msb_loop_entry
.Lmemcpy_lsb_loop_entry:
	ld a, (bc)
	inc bc
	ldi (hl), a
	dec e
	jr nz, .Lmemcpy_lsb_loop_entry
.Lmemcpy_msb_loop_entry:
										// We've decremented the LSB to 0, decrement the MSB too
	dec d
	ret z
	jr .Lmemcpy_lsb_loop_entry


.global memset
memset:								// @memset(hl = void *dst, bc = int byte, de = size_t count)
	inc d								// Pre-increment MSB to avoid edge-casing 0

	ld a, e
	or a								// Is LSB already 0?

	ld a, b								// From now on "byte" lives in a
	jr z, .Lmemset_msb_loop_entry
.Lmemset_lsb_loop_entry:
										// Happy loop: entirely 8-bit math, compare is free
	ldi (hl), a
	dec e
	jr nz, .Lmemset_lsb_loop_entry
.Lmemset_msb_loop_entry:
										// We've decremented the LSB to 0, decrement the MSB too
	dec d
	ret z
	jr .Lmemset_lsb_loop_entry
