#include <libgb/interrupts.hpp>
#include <libgb/serial.hpp>

using namespace libgb::arch::registers;

auto libgb::serial_write_char(char data) -> void {
  set_seral_transfer_data(data);
  set_seral_transfer_control({
      .clock_select = SerialTransferClockSelect::internal_clock,
      .clock_speed = SerialTransferClockSpeed::normal,
      .padding_0 = 0,
      .enable = 1,
  });
  libgb::wait_for_interrupt<libgb::Interrupt::serial>();
}

static auto write_nibble_as_hex(uint8_t nibble) -> void {
  if (nibble <= 9) {
    libgb::serial_write_char('0' + nibble);
  } else {
    libgb::serial_write_char('a' + (nibble - 10));
  }
}

auto libgb::serial_write(char const *to_write) -> void {
  while (*to_write != '\0') {
    serial_write_char(*to_write);
    to_write += 1;
  }
}

auto libgb::serial_write(StringView to_write) -> void {
  while (to_write.size-- != 0) {
    serial_write_char(*to_write.data);
    to_write.data += 1;
  }
}

auto libgb::serial_write(uint16_t to_write) -> void {
  serial_write_char('$');
  write_nibble_as_hex(to_write >> 12U);
  write_nibble_as_hex((to_write >> 8U) & 0b1111U);
  write_nibble_as_hex((to_write >> 4U) & 0b1111U);
  write_nibble_as_hex(to_write & 0b1111U);
}
