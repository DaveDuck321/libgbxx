; Routine from: https://gbdev.io/pandocs/OAM_DMA_Transfer.html
.section .text.hram
.global __libgb_do_dma
__libgb_do_dma:                 ; void do_dma(uint8_t high)
    ld a, b
    ldh (0x46), a
    ld a, 40
__libgb_dma_wait:
    dec a
    jr nz, __libgb_dma_wait
    ret
