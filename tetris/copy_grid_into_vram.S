#include "shared.defs"

#if BOARD_WIDTH != 10
#error "Please update the COPY_ROW macro"
#endif

#if BOARD_HEIGHT != 16
#error "Please update the copy_grid_into_vram definition"
#endif

#define VRAM_STRIDE 0x20
#define VRAM_BASE 0x9800

#define VRAM_ADDR(row) VRAM_BASE + VRAM_STRIDE * (BOARD_HEIGHT - 1 - row)

#define COPY_ROW(row)            \
    ld hl, row * BOARD_STRIDE;   \
    add hl, bc;                  \
    ldi a, (hl);                 \
    ld (VRAM_ADDR(row) + 0), a;  \
    ldi a, (hl);                 \
    ld (VRAM_ADDR(row) + 1), a;  \
    ldi a, (hl);                 \
    ld (VRAM_ADDR(row) + 2), a;  \
    ldi a, (hl);                 \
    ld (VRAM_ADDR(row) + 3), a;  \
    ldi a, (hl);                 \
    ld (VRAM_ADDR(row) + 4), a;  \
    ldi a, (hl);                 \
    ld (VRAM_ADDR(row) + 5), a;  \
    ldi a, (hl);                 \
    ld (VRAM_ADDR(row) + 6), a;  \
    ldi a, (hl);                 \
    ld (VRAM_ADDR(row) + 7), a;  \
    ldi a, (hl);                 \
    ld (VRAM_ADDR(row) + 8), a;  \
    ldi a, (hl);                 \
    ld (VRAM_ADDR(row) + 9), a;  \



// TODO: the compile should be good enough to generate this routine (and it almost is)
.global copy_grid_into_vram_map_0
copy_grid_into_vram_map_0:        # @copy_grid_into_vram_map_0(hl = void *grid)
    ld b, h
    ld c, l
    COPY_ROW(0)
    COPY_ROW(1)
    COPY_ROW(2)
    COPY_ROW(3)
    COPY_ROW(4)
    COPY_ROW(5)
    COPY_ROW(6)
    COPY_ROW(7)
    COPY_ROW(8)
    COPY_ROW(9)
    COPY_ROW(10)
    COPY_ROW(11)
    COPY_ROW(12)
    COPY_ROW(13)
    COPY_ROW(14)
    COPY_ROW(15)
    ret
