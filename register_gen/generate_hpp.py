from __future__ import annotations

from argparse import ArgumentParser
from pathlib import Path
from dataclasses import dataclass, field

import json


def parse_int(value: int | str) -> int:
    if isinstance(value, int):
        return value
    return int(value, base=0)


def indent(line: str, amount: int = 1) -> str:
    return " " * (amount * 4) + line


def to_struct_name(name: str) -> str:
    return "".join(part.title() for part in name.split("_"))


@dataclass
class Type:
    name: str
    width: int


@dataclass
class IntType(Type):
    @staticmethod
    def from_width(width: int) -> IntType:
        assert width % 8 == 0
        return IntType(name=f"uint{width}_t", width=width)

    def to_cxx(self) -> str:
        return self.name


@dataclass
class EnumType(Type):
    def to_cxx(self) -> str:
        return self.name


@dataclass
class GeneratedEnumMember:
    name: str
    value: int
    comment: str | None

    def get_lines(self, indent_amount=1) -> list[str]:
        lines: list[str] = []
        if self.comment:
            lines.append(indent(f"// {self.comment}", indent_amount))

        lines.append(indent(f"{self.name} = {hex(self.value)},", indent_amount))
        return lines


@dataclass
class GeneratedEnum:
    name: str
    width: int
    members: list[GeneratedEnumMember]

    def get_lines(self) -> list[str]:
        lines: list[str] = []
        lines.append(
            f"enum class {self.name} : {IntType.from_width(self.width).to_cxx()} {{"
        )
        for member in self.members:
            lines.extend(member.get_lines())
        lines.append("};")
        return lines


@dataclass
class GeneratedRegisterField:
    name: str
    width: int
    this_type: Type
    is_read_volatile: bool
    can_read: bool
    can_write: bool


@dataclass
class Padding:
    width: int


def generate_getters_and_setters(
    name: str, addr: str, field: GeneratedRegisterField
) -> list[str]:
    lines: list[str] = []
    if field.can_read:
        lines.append("")
        lines.append(
            f"[[nodiscard]] inline auto get_{name}() -> {field.this_type.name} {{"
        )

        dereference: str
        if field.is_read_volatile:
            dereference = f"*({field.this_type.name} volatile*){addr}"
        else:
            dereference = f"*({field.this_type.name}*){addr}"

        lines.append(indent(f"return {dereference};"))
        lines.append("}")

    if field.can_write:
        lines.append("")
        lines.append(f"inline auto set_{name}({field.this_type.name} value) -> void {{")

        dereference = f"*({field.this_type.name} volatile*){addr}"
        lines.append(indent(f"{dereference} = value;"))
        lines.append("}")

    return lines


@dataclass
class GeneratedRegister:
    name: str
    address: int
    fields: list[GeneratedRegisterField | Padding]
    width: int

    def get_lines(self) -> list[str]:
        lines: list[str] = []
        lines.append(
            f"static constexpr intptr_t {self.name}_addr = {hex(self.address)};"
        )

        # Special case for non-bitfield registers
        if len(self.fields) == 1:
            field = self.fields[0]
            assert field.width == 8
            assert isinstance(field, GeneratedRegisterField)
            lines.extend(
                generate_getters_and_setters(self.name, f"{self.name}_addr", field)
            )
            return lines

        # Generate the bitfield type
        whole_register_type = to_struct_name(self.name)
        lines.append(f"struct [[gnu::packed]] {whole_register_type} {{")

        padding_index = 0
        for field in self.fields:
            if isinstance(field, Padding):
                field = f"{IntType.from_width(8).name} padding_{padding_index} : {field.width};"
                padding_index += 1
            elif isinstance(field, GeneratedRegisterField):
                if field.width == field.this_type.width:
                    field = f"{field.this_type.name} {field.name};"
                else:
                    field = f"{field.this_type.name} {field.name} : {field.width};"
            else:
                raise AssertionError

            lines.append(indent(field, 1))

        lines.append("};")

        # Generate the whole-register getter/ setters
        do_generate_getter = all(
            field.can_read
            for field in self.fields
            if isinstance(field, GeneratedRegisterField)
        )
        if do_generate_getter:
            lines.append("")
            lines.append(
                f"[[nodiscard]] inline auto get_{self.name}() -> {whole_register_type} {{"
            )

            is_read_volatile = any(
                field.is_read_volatile
                for field in self.fields
                if isinstance(field, GeneratedRegisterField)
            )
            if is_read_volatile:
                # We load into an integer to guarantee the volatile load is
                # exactly the correct size + alignment. We then bitcast into the
                # correct type... Note: we can't rely on libc memcpy existing.
                int_type = IntType.from_width(self.width)
                lines.append(
                    indent(
                        f"static_assert(sizeof({whole_register_type}) == sizeof({int_type.name}));"
                    )
                )
                lines.append(
                    indent(
                        f"{int_type.name} loaded = *({int_type.name} volatile*){self.name}_addr;"
                    )
                )
                lines.append(indent(f"{whole_register_type} result;"))
                lines.append(
                    indent(
                        f"__builtin_memcpy(&result, &loaded, sizeof({int_type.name}));"
                    )
                )
                lines.append(indent(f"return result;"))
            else:
                # Non-volatile loads are supported by the default constructors
                lines.append(
                    indent(f"return *({whole_register_type}*){self.name}_addr;")
                )

            lines.append("}")

        # Unlike the getter, we generate the setter if ANY field can be written
        do_generate_setter = any(
            field.can_write
            for field in self.fields
            if isinstance(field, GeneratedRegisterField)
        )
        if do_generate_setter:
            lines.append("")
            lines.append(
                f"inline auto set_{self.name}({whole_register_type} value) -> void {{"
            )

            int_type = IntType.from_width(self.width)
            lines.append(
                indent(
                    f"static_assert(sizeof({whole_register_type}) == sizeof({int_type.name}));"
                )
            )

            lines.append(indent(f"{int_type.name} to_store;"))
            lines.append(
                indent(f"__builtin_memcpy(&to_store, &value, sizeof({int_type.name}));")
            )
            lines.append(
                indent(f"*({int_type.name} volatile*){self.name}_addr = to_store;")
            )
            lines.append("}")

        for field in self.fields:
            if isinstance(field, Padding):
                continue

            if field.can_read:
                lines.append("")
                lines.append(
                    f"inline auto get_{self.name}_{field.name}() -> {field.this_type.name} {{"
                )
                dereference: str
                if field.is_read_volatile:
                    dereference = f"(({whole_register_type} volatile*){self.name}_addr)->{field.name}"
                else:
                    dereference = (
                        f"(({whole_register_type}*){self.name}_addr)->{field.name}"
                    )

                lines.append(indent(f"return {dereference};"))
                lines.append("}")

            if field.can_write:
                lines.append("")
                lines.append(
                    f"inline auto set_{self.name}_{field.name}({field.this_type.name} value) -> void {{"
                )
                dereference = (
                    f"(({whole_register_type} volatile*){self.name}_addr)->{field.name}"
                )
                lines.append(indent(f"{dereference} = value;"))
                lines.append("}")

        return lines


@dataclass
class Generated:
    headers: list[str] = field(default_factory=list)
    enums: dict[str, GeneratedEnum] = field(default_factory=dict)
    registers: list[GeneratedRegister] = field(default_factory=list)


def generate_enum(enum_definition: dict) -> GeneratedEnum:
    assert enum_definition["type"] == "enum"

    members: list[GeneratedEnumMember] = []
    for member in enum_definition["values"]:
        members.append(
            GeneratedEnumMember(
                name=member["name"],
                value=parse_int(member["value"]),
                comment=member.get("comment"),
            )
        )

    return GeneratedEnum(
        name=enum_definition["name"],
        width=parse_int(enum_definition["width"]),
        members=members,
    )


def generate_register(register_definition: dict) -> GeneratedRegister:
    fields: list[GeneratedRegisterField | Padding] = []
    for field in register_definition["layout"]:
        match field["type"]:
            case "non_volatile_read_write":
                can_read = True
                can_write = True
                is_read_volatile = False
            case "volatile_read_write":
                can_read = True
                can_write = True
                is_read_volatile = True
            case "volatile_read_only":
                can_read = True
                can_write = False
                is_read_volatile = True
            case "write_only":
                can_read = False
                can_write = True
                is_read_volatile = False
            case "padding":
                fields.append(Padding(parse_int(field["width"])))
                continue
            case _:
                raise AssertionError(f"Unrecognized type: {field['type']}")

        if "enum" in field:
            this_type = EnumType(field["enum"], 8)
        else:
            this_type = IntType.from_width(8)

        fields.append(
            GeneratedRegisterField(
                name=field["name"],
                width=parse_int(field["width"]),
                this_type=this_type,
                is_read_volatile=is_read_volatile,
                can_read=can_read,
                can_write=can_write,
            )
        )

    total_width = 0
    for field in fields:
        total_width += field.width
    assert (
        total_width == 8
    ), f"Fields in {register_definition['name']} do not add to 8-bits"

    return GeneratedRegister(
        name=register_definition["name"],
        address=parse_int(register_definition["address"]),
        fields=fields,
        width=total_width,
    )


def generate_register_headers(
    input: Path, output: Path, generate_enums: bool, generate_accessors: bool
) -> None:
    generated = Generated()
    generated.headers.append("<stdint.h>")
    if not generate_enums:
        generated.headers.append('"enums.hpp"')

    # Parse the register definitions
    definitions = json.load(input.open())
    for definition in definitions:
        match definition["type"]:
            case "enum":
                enum = generate_enum(definition)

                assert enum.name not in generated.enums
                generated.enums[enum.name] = enum
            case "register":
                register = generate_register(definition)
                generated.registers.append(register)
            case _:
                raise AssertionError

    # Dump into a c++ header
    lines: list[str] = []
    lines.append("#pragma once")

    # 1) Includes
    for header in sorted(generated.headers):
        lines.append(f"#include {header}")
    lines.append("")

    # 2) Begin namespace
    lines.append("namespace libgb::arch {")
    lines.append("")

    # 3) Enums
    if generate_enums:
        for enum in generated.enums.values():
            lines.extend(enum.get_lines())
            lines.append("")

    # 4) Accessors
    if generate_accessors:
        for register in generated.registers:
            lines.extend(register.get_lines())
            lines.append("")

    # Do some validation
    seen_addresses: set[int] = set()
    for register in generated.registers:
        if register.address in seen_addresses:
            raise ValueError(f"Saw duplicate register for {seen_addresses:x}")
        seen_addresses.add(register.address)

    # 5) End namespace
    lines.append("} // libgb::arch")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines))


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("--output", "-o", type=Path)
    parser.add_argument("--type", choices=["enum", "accessors", "both"], required=True)
    arg = parser.parse_args()

    generate_enums = arg.type == "enum" or arg.type == "both"
    generate_accessors = arg.type == "accessors" or arg.type == "both"
    generate_register_headers(arg.input, arg.output, generate_enums, generate_accessors)
