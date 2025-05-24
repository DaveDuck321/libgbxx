#include <libgb/std/array.hpp>
#include <libgb/std/memcpy.hpp>

#include <stdint.h>

enum class CartridgeType : uint8_t {
  rom_only = 0x00,
  mbc1 = 0x01,
  mbc1_ram = 0x02,
  mbc1_ram_and_battery = 0x03,
};

enum class RomSize : uint8_t {
  rom_32k = 0x00,
  rom_64k = 0x01,
  rom_128 = 0x02,
};

enum class RamSize : uint8_t {
  no_ram = 0x00,
  ram_8k = 0x02,
  ram_32k = 0x03,
};

enum class Locale : uint8_t {
  japan = 0x00,
  overseas = 0x01,
};

enum class Licensee : uint8_t {
  none = 0x00,
  nintendo = 0x01,
};

struct Meta {
  libgb::Array<char, 16> title;
  libgb::Array<uint8_t, 2> new_licensee_code;
  uint8_t sgb_flag;
  CartridgeType cartridge_type;
  RomSize rom_size;
  RamSize ram_size;
  Locale locale;
  Licensee licensee;
  uint8_t version_number;
  uint8_t checksum;
};
static_assert(sizeof(Meta) == 26);

[[gnu::section(".logo")]]
constinit libgb::Array<uint8_t, 48> nintendo_logo = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
    0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
    0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
};

constexpr auto generate_meta(char const *name) -> Meta {
  Meta meta = {};
  for (size_t i = 0; i < meta.title.size() && name[i] != '\0'; i += 1) {
    meta.title[i] = name[i];
  }
  meta.cartridge_type = CartridgeType::rom_only;
  meta.rom_size = RomSize::rom_32k;
  meta.locale = Locale::overseas;
  meta.licensee = Licensee::none;

  uint8_t checksum = 0;
  auto meta_bytes = libgb::bit_cast<libgb::Array<uint8_t, sizeof(Meta)>>(meta);
  for (uint8_t byte : meta_bytes) {
    checksum -= byte + 1;
  }
  meta.checksum = checksum + 1;
  return meta;
}

[[gnu::section(".meta")]]
constinit Meta meta = generate_meta("libgb game");
