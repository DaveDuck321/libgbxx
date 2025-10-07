    .data

.section .text.__gb_int_vblank
int_vblank:
    jp __libgb_vblank_interrupt_trampoline

.section .text.__gb_int_lcd_status
int_lcd_status:
    jp __libgb_lcd_status_interrupt_trampoline

.section .text.__gb_int_timer
int_timer:
    jp __libgb_timer_interrupt_trampoline

.section .text.__gb_int_serial
int_serial:
    jp __libgb_serial_interrupt_trampoline

.section .text.__gb_int_input
int_input:
    jp __libgb_input_interrupt_trampoline

    .text

    .section .text
__libgb_default_interrupt_callback:
    stop
    nop

	.section	.text.hram

#define INTERRUPT_TRAMPOLINE(name)              \
	.globl	__libgb_##name##_interrupt_callback;\
__libgb_##name##_interrupt_trampoline:          \
    .byte $C3;                                  \
__libgb_##name##_interrupt_callback:            \
    .short __libgb_default_interrupt_callback;

INTERRUPT_TRAMPOLINE(vblank)
INTERRUPT_TRAMPOLINE(lcd_status)
INTERRUPT_TRAMPOLINE(timer)
INTERRUPT_TRAMPOLINE(serial)
INTERRUPT_TRAMPOLINE(input)

#undef INTERRUPT_TRAMPOLINE
