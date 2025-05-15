    .data

.section .text.__gb_int_vblank
int_vblank:
    push af
    push hl
    ld hl,  __libgb_handle_vblank_interrupt
    jp dispatch_handle_interrupt

.section .text.__gb_int_lcd_status
int_lcd_status:
    push af
    push hl
    ld hl,  __libgb_handle_lcd_status_interrupt
    jp dispatch_handle_interrupt

.section .text.__gb_int_timer
int_timer:
    push af
    push hl
    ld hl,  __libgb_handle_timer_interrupt
    jp dispatch_handle_interrupt

.section .text.__gb_int_serial
int_serial:
    push af
    push hl
    ld hl,  __libgb_handle_serial_interrupt
    jp dispatch_handle_interrupt

.section .text.__gb_int_input
int_input:
    push af
    push hl
    ld hl,  __libgb_handle_input_interrupt
    jp dispatch_handle_interrupt


    .text
dispatch_handle_interrupt:
                    ; Save the remaining working registers
    push bc
    push de

                    ; The interrupt handler has pointed HL to the user's callback
    ldi a, (hl)
    ld h, (hl)
    ld l, a
    call (hl)       ; Call into the user-defined handler

    pop de
    pop bc
    pop hl
    pop af
    reti
