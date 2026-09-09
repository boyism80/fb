#!/usr/bin/env python3
"""Generate integration protocol registry from game protocol headers."""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
PROTO_ROOT = ROOT / "include" / "fb" / "game" / "protocol"
OUT_CPP = ROOT / "server" / "bot" / "src" / "integration" / "protocol_registry.generated.cpp"
OUT_H = ROOT / "include" / "fb" / "bot" / "integration" / "protocol_registry.generated.h"
OUT_LUA_CPP = ROOT / "server" / "bot" / "src" / "integration" / "protocol_lua.generated.cpp"
OUT_LUA_H = ROOT / "include" / "fb" / "bot" / "integration" / "protocol_lua.generated.h"

CLASS_RE = re.compile(
    r"(?:template\s*<[^>]+>\s*)?"
    r"class\s+(\w+)\s*:\s*public\s+fb::protocol::header"
)
OPCODE_RE = re.compile(
    r"static\s+constexpr\s+uint8_t\s+opcode\s*=\s*([^;]+);"
)
TEMPLATE_RE = re.compile(r"template\s*<([^>]+)>\s*class\s+(\w+)\s*:")
# Template header that immediately precedes the matched class declaration.
LEADING_TEMPLATE_RE = re.compile(r"template\s*<([^>]+)>\s*$")

# The bot speaks a single C2S layout; versioned requests are instantiated for it.
BOT_CLIENT_VERSION_ALIAS = "BOT_CLIENT_VERSION"
BOT_CLIENT_VERSION_VALUE = "fb::protocol::CLIENT_VERSION::v550"

FIELD_RE = re.compile(
    r"^\s*(?:(?:const|static|volatile)\s+)*"
    r"(std::string|uint32_t|uint16_t|uint8_t|int32_t|int16_t|int8_t|bool|"
    r"ACTION|CHAT_TYPE|DURATION|MESSAGE_TYPE|SPELL_TYPE|STATE|DIRECTION|"
    r"UPDATE_STATE_LEVEL|OPTION|EQUIPMENT_PARTS|SWAP_TYPE|"
    r"fb::model::point<uint16_t>)"
    r"\s+(\w+)\s*(?:=\s*[^;]+)?;",
    re.MULTILINE,
)

ENUM_TYPES = {
    "ACTION",
    "CHAT_TYPE",
    "DURATION",
    "MESSAGE_TYPE",
    "SPELL_TYPE",
    "STATE",
    "DIRECTION",
    "SWAP_TYPE",
    "DIALOG_RESULT",
    "BULLETIN_BUTTON_ENABLE",
    "UPDATE_STATE_LEVEL",
    "OPTION",
    "EQUIPMENT_PARTS",
}

PRIMITIVE_FIELD_TYPES = {
    "std::string",
    "uint32_t",
    "uint16_t",
    "uint8_t",
    "int32_t",
    "int16_t",
    "int8_t",
    "bool",
    "fb::model::point<uint16_t>",
}


@dataclass
class ProtocolField:
    cpp_type: str
    name: str


@dataclass
class ProtocolType:
    direction: str  # request | response
    alias: str      # game_reqs | game_resp
    class_name: str
    cpp_type: str   # C++ expression, may carry template arguments
    opcode: int
    type_key: str   # stable registry key, without CLIENT_VERSION arguments
    name: str
    has_default_ctor: bool = False
    fields: list[ProtocolField] = field(default_factory=list)
    ctor_params: list[tuple[str, str]] = field(default_factory=list)


def parse_opcode(expr: str) -> int | None:
    expr = expr.strip()
    if "?" in expr:
        return None
    m = re.match(r"0x([0-9A-Fa-f]+)", expr)
    if m:
        return int(m.group(1), 16)
    m = re.match(r"(\d+)", expr)
    if m:
        return int(m.group(1))
    return None


def is_client_version_template(template_params: str | None) -> bool:
    return bool(template_params) and "CLIENT_VERSION" in template_params


def expand_conditional_opcode(
    direction: str,
    alias: str,
    class_name: str,
    opcode_expr: str,
    template_params: str | None,
) -> list[ProtocolType]:
    if template_params and "bool Detailed" in template_params and "?" in opcode_expr:
        parts = [p.strip() for p in opcode_expr.split("?")[1].split(":")]
        true_opcode = parse_opcode(parts[0])
        false_opcode = parse_opcode(parts[1])
        if true_opcode is None or false_opcode is None:
            raise ValueError(f"cannot parse conditional opcode: {opcode_expr}")
        return [
            make_type(direction, alias, f"{class_name}<true>", true_opcode),
            make_type(direction, alias, f"{class_name}<false>", false_opcode),
        ]

    opcode = parse_opcode(opcode_expr)
    if opcode is None:
        raise ValueError(f"unsupported opcode expression: {opcode_expr}")

    # Registry keys stay version agnostic; the C++ type is instantiated for the
    # single layout the bot speaks.
    if is_client_version_template(template_params):
        return [make_type(direction, alias, class_name, opcode, versioned=True)]

    if template_params:
        raise ValueError(f"unsupported template protocol: {class_name} <{template_params}>")

    return [make_type(direction, alias, class_name, opcode)]


def make_lua_name(cpp_name: str) -> str:
    if "<" not in cpp_name:
        return cpp_name

    base, params = cpp_name.split("<", 1)
    params = params.rstrip(">")

    if base == "update_external":
        if params == "true":
            return "update_external_detailed"
        if params == "false":
            return "update_external_brief"

    return f"{base}_{params.replace(',', '_').replace(' ', '')}"


def make_type(
    direction: str,
    alias: str,
    cpp_name: str,
    opcode: int,
    versioned: bool = False,
) -> ProtocolType:
    type_key = f"{alias}::{cpp_name}"
    name = make_lua_name(cpp_name)
    cpp_type = f"{type_key}<{BOT_CLIENT_VERSION_ALIAS}>" if versioned else type_key
    return ProtocolType(
        direction=direction,
        alias=alias,
        class_name=cpp_name,
        cpp_type=cpp_type,
        opcode=opcode,
        type_key=type_key,
        name=name,
    )


def preprocess_for_bot(text: str) -> str:
    text = re.sub(
        r"#ifndef\s+BOT\b.*?#else\b(.*?)#endif",
        r"\1",
        text,
        flags=re.DOTALL,
    )
    text = re.sub(r"#ifndef\s+BOT\b.*?#endif", "", text, flags=re.DOTALL)
    text = re.sub(r"#if\s+BOT\b(.*?)#else\b.*?#endif", r"\1", text, flags=re.DOTALL)
    text = re.sub(r"#ifdef\s+BOT\b(.*?)#else\b.*?#endif", r"\1", text, flags=re.DOTALL)
    text = re.sub(r"#if\s+BOT\b(.*?)#endif", r"\1", text, flags=re.DOTALL)
    text = re.sub(r"#ifdef\s+BOT\b(.*?)#endif", r"\1", text, flags=re.DOTALL)
    return text


def extract_class_body(ns_body: str, match: re.Match[str]) -> str:
    brace = ns_body.find("{", match.end())
    if brace < 0:
        return ns_body[match.start() : match.end() + 500]

    depth = 0
    for i in range(brace, len(ns_body)):
        ch = ns_body[i]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return ns_body[match.start() : i + 1]

    return ns_body[match.start() : match.end() + 2000]


def has_bot_default_constructor(class_body: str, class_name: str) -> bool:
    bot_view = preprocess_for_bot(class_body)
    return bool(
        re.search(rf"\b{re.escape(class_name)}\s*\(\s*\)\s*=\s*default\b", bot_view)
    )


def strip_nested_definitions(text: str) -> str:
    result: list[str] = []
    i = 0
    n = len(text)
    while i < n:
        m = re.search(r"\b(?:struct|class|union)\s+[\w:]+", text[i:])
        if not m:
            result.append(text[i:])
            break

        result.append(text[i : i + m.start()])
        search_from = i + m.end()
        brace = text.find("{", search_from)
        if brace < 0:
            i = search_from
            continue

        depth = 0
        j = brace
        while j < n:
            if text[j] == "{":
                depth += 1
            elif text[j] == "}":
                depth -= 1
                if depth == 0:
                    i = j + 1
                    break
            j += 1
        else:
            break

    return "".join(result)


def normalize_param_type(cpp_type: str) -> str:
    t = re.sub(r"\bconst\b", "", cpp_type).strip()
    t = re.sub(r"&$", "", t).strip()
    return t


def looks_like_enum_type(cpp_type: str) -> bool:
    norm = normalize_param_type(cpp_type)
    if norm in ENUM_TYPES:
        return True
    return bool(re.match(r"^[A-Z][A-Z0-9_]*$", norm))


def is_supported_field_type(cpp_type: str) -> bool:
    if cpp_type in PRIMITIVE_FIELD_TYPES:
        return True
    if cpp_type.startswith("std::vector") or cpp_type.startswith("std::list"):
        return False
    if cpp_type.startswith("std::"):
        return False
    if "::" in cpp_type:
        return False
    return looks_like_enum_type(cpp_type)


def is_supported_ctor_param(cpp_type: str) -> bool:
    norm = normalize_param_type(cpp_type)
    if norm in PRIMITIVE_FIELD_TYPES or norm == "std::string_view":
        return True
    if norm == "fb::model::point<uint16_t>":
        return True
    if "::" in norm:
        return False
    return looks_like_enum_type(norm)


def enum_type_for_codegen(cpp_type: str) -> str:
    norm = normalize_param_type(cpp_type)
    if norm in ENUM_TYPES or re.match(r"^[A-Z][A-Z0-9_]*$", norm):
        return f"fb::model::enum_value::{norm}"
    return norm


def can_generate_builder(protocol: ProtocolType) -> bool:
    if protocol.direction != "request":
        return False
    if protocol.has_default_ctor and not protocol.ctor_params:
        return True
    if not protocol.ctor_params:
        return False
    return all(is_supported_ctor_param(cpp_type) for cpp_type, _ in protocol.ctor_params)


def class_body_interior(class_body: str) -> str:
    preprocessed = preprocess_for_bot(class_body)
    start = preprocessed.find("{")
    end = preprocessed.rfind("}")
    if start < 0 or end <= start:
        return preprocessed
    return preprocessed[start + 1 : end]


def extract_bot_fields(class_body: str) -> list[ProtocolField]:
    bot_view = strip_nested_definitions(class_body_interior(class_body))
    fields: list[ProtocolField] = []
    seen: set[str] = set()
    for match in FIELD_RE.finditer(bot_view):
        cpp_type = match.group(1)
        name = match.group(2)
        if name in seen:
            continue
        if is_supported_field_type(cpp_type) == False:
            continue
        seen.add(name)
        fields.append(ProtocolField(cpp_type=cpp_type, name=name))
    return fields


def split_ctor_params(params: str) -> list[tuple[str, str]]:
    if not params.strip():
        return []

    parts: list[str] = []
    current: list[str] = []
    angle_depth = 0
    paren_depth = 0
    for ch in params:
        if ch == "<":
            angle_depth += 1
        elif ch == ">":
            angle_depth -= 1
        elif ch == "(":
            paren_depth += 1
        elif ch == ")":
            paren_depth -= 1
        elif ch == "," and angle_depth == 0 and paren_depth == 0:
            parts.append("".join(current).strip())
            current = []
            continue
        current.append(ch)
    tail = "".join(current).strip()
    if tail:
        parts.append(tail)

    parsed: list[tuple[str, str]] = []
    for part in parts:
        part = normalize_param_type(part)
        if "=" in part:
            part = part.split("=", 1)[0].strip()
        tokens = part.split()
        if len(tokens) < 2:
            continue
        name = tokens[-1]
        cpp_type = " ".join(tokens[:-1])
        parsed.append((cpp_type, name))
    return parsed


def extract_parameter_list(text: str, open_paren_index: int) -> tuple[str, int] | None:
    if open_paren_index >= len(text) or text[open_paren_index] != "(":
        return None

    depth = 0
    angle = 0
    for i in range(open_paren_index, len(text)):
        ch = text[i]
        if ch == "<":
            angle += 1
        elif ch == ">" and angle > 0:
            angle -= 1
        elif ch == "(" and angle == 0:
            depth += 1
        elif ch == ")" and angle == 0:
            depth -= 1
            if depth == 0:
                return text[open_paren_index + 1 : i], i + 1
    return None


def extract_bot_ctor_params(class_body: str, class_name: str) -> list[tuple[str, str]]:
    # After ctors moved into .cpp, BOT constructors are declarations that end
    # with ';' rather than an inline ': member()' / '{ body }'.
    bot_view = preprocess_for_bot(class_body)
    pattern = rf"\b{re.escape(class_name)}\s*\("
    for match in re.finditer(pattern, bot_view):
        extracted = extract_parameter_list(bot_view, match.end() - 1)
        if extracted is None:
            continue

        params, end = extracted
        rest = bot_view[end:].lstrip()
        if rest.startswith("= default"):
            continue
        if not rest or rest[0] not in ";:{,":
            continue
        if not params.strip():
            continue
        return split_ctor_params(params)
    return []


def extract_namespace_body(text: str, direction: str) -> str | None:
    marker = f"namespace fb::protocol::game::{direction}"
    start = text.find(marker)
    if start < 0:
        return None

    brace = text.find("{", start)
    if brace < 0:
        return None

    depth = 0
    for i in range(brace, len(text)):
        ch = text[i]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1 : i]

    return None


def scan_protocols() -> list[ProtocolType]:
    results: list[ProtocolType] = []
    seen: set[str] = set()

    for path in sorted(PROTO_ROOT.rglob("*.h")):
        text = path.read_text(encoding="utf-8")
        for direction, alias in (("request", "game_reqs"), ("response", "game_resp")):
            sub = extract_namespace_body(text, direction)
            if sub is None:
                continue

            for m in CLASS_RE.finditer(sub):
                class_name = m.group(1)
                start = m.start()
                leading = sub[max(0, start - 120) : start]
                # Skip explicit specializations (v651 overlays). The bot speaks
                # the primary CLIENT_VERSION template instantiated as v550.
                if re.search(r"template\s*<>\s*$", leading):
                    continue

                template_params = None
                tm = TEMPLATE_RE.match(m.group(0))
                if tm and tm.group(2) == class_name:
                    template_params = tm.group(1)
                else:
                    lm = LEADING_TEMPLATE_RE.search(leading)
                    if lm:
                        template_params = lm.group(1)

                chunk = sub[m.end() : m.end() + 500]
                om = OPCODE_RE.search(chunk)
                if not om:
                    continue

                class_body = extract_class_body(sub, m)
                has_default_ctor = (
                    direction == "request"
                    and has_bot_default_constructor(class_body, class_name)
                )
                fields = (
                    extract_bot_fields(class_body)
                    if direction == "response"
                    else []
                )
                ctor_params = (
                    extract_bot_ctor_params(class_body, class_name)
                    if direction == "request"
                    else []
                )

                expanded = expand_conditional_opcode(
                    direction,
                    alias,
                    class_name,
                    om.group(1),
                    template_params,
                )
                for entry in expanded:
                    if entry.type_key in seen:
                        continue
                    seen.add(entry.type_key)
                    results.append(
                        ProtocolType(
                            direction=entry.direction,
                            alias=entry.alias,
                            class_name=entry.class_name,
                            cpp_type=entry.cpp_type,
                            opcode=entry.opcode,
                            type_key=entry.type_key,
                            name=entry.name,
                            has_default_ctor=has_default_ctor,
                            fields=fields,
                            ctor_params=ctor_params,
                        )
                    )

    results.sort(key=lambda t: (t.direction, t.type_key))
    return results


def mangle_symbol(type_key: str) -> str:
    return re.sub(r"[^A-Za-z0-9]", "_", type_key)


def append_marshal_field(lines: list[str], fld: ProtocolField, var: str = "resp") -> None:
    name = fld.name
    ctype = fld.cpp_type
    if ctype in PRIMITIVE_FIELD_TYPES:
        if ctype == "bool":
            lines.append(f'    lua->pushstring("{name}");')
            lines.append(f"    lua->pushboolean({var}.{name});")
            lines.append("    lua->settable(-3);")
        elif ctype == "std::string":
            lines.append(f'    lua->pushstring("{name}");')
            lines.append(f"    lua->pushstring({var}.{name});")
            lines.append("    lua->settable(-3);")
        elif ctype == "fb::model::point<uint16_t>":
            lines.append(f'    lua->pushstring("{name}");')
            lines.append("    lua->new_table();")
            lines.append(f"    lua->pushinteger(1); lua->pushinteger({var}.{name}.x); lua->settable(-3);")
            lines.append(f"    lua->pushinteger(2); lua->pushinteger({var}.{name}.y); lua->settable(-3);")
            lines.append("    lua->settable(-3);")
        else:
            lines.append(f'    lua->pushstring("{name}");')
            lines.append(f"    lua->pushinteger({var}.{name});")
            lines.append("    lua->settable(-3);")
    elif looks_like_enum_type(ctype):
        lines.append(f'    lua->pushstring("{name}");')
        lines.append("    try")
        lines.append("    {")
        lines.append(f"        lua->pushstring(fb::model::enum_value::enum_tostring({var}.{name}));")
        lines.append("    }")
        lines.append("    catch (...)")
        lines.append("    {")
        lines.append("        lua->pushnil();")
        lines.append("    }")
        lines.append("    lua->settable(-3);")
        lines.append(f'    lua->pushstring("{name}_id");')
        lines.append(f"    lua->pushinteger(static_cast<lua_Integer>({var}.{name}));")
        lines.append("    lua->settable(-3);")


SPECIAL_MARSHAL_EXTRAS: dict[str, list[str]] = {
    "game_resp::update_cc": [
        "    const auto cc = static_cast<uint32_t>(resp.cc);",
        '    lua->pushstring("cc");',
        "    lua->pushinteger(cc);",
        "    lua->settable(-3);",
        '    lua->pushstring("direction");',
        "    lua->pushboolean((cc & static_cast<uint32_t>(fb::model::enum_value::CROWD_CONTROL::DIRECTION)) != 0);",
        "    lua->settable(-3);",
        '    lua->pushstring("sight");',
        "    lua->pushboolean((cc & static_cast<uint32_t>(fb::model::enum_value::CROWD_CONTROL::SIGHT)) != 0);",
        "    lua->settable(-3);",
        '    lua->pushstring("hear");',
        "    lua->pushboolean((cc & static_cast<uint32_t>(fb::model::enum_value::CROWD_CONTROL::HEAR)) != 0);",
        "    lua->settable(-3);",
        '    lua->pushstring("chat");',
        "    lua->pushboolean((cc & static_cast<uint32_t>(fb::model::enum_value::CROWD_CONTROL::CHAT)) != 0);",
        "    lua->settable(-3);",
    ],
    "game_resp::update": [
        '    lua->pushstring("objects_data");',
        "    lua->new_table();",
        "    for (size_t i = 0; i < resp.objects_data.size(); ++i)",
        "    {",
        "        const auto& object = resp.objects_data[i];",
        "        lua->pushinteger(static_cast<lua_Integer>(i + 1));",
        "        lua->new_table();",
        '        lua->pushstring("x");',
        "        lua->pushinteger(object.x);",
        "        lua->settable(-3);",
        '        lua->pushstring("y");',
        "        lua->pushinteger(object.y);",
        "        lua->settable(-3);",
        '        lua->pushstring("oid");',
        "        lua->pushinteger(object.oid);",
        "        lua->settable(-3);",
        '        lua->pushstring("look");',
        "        lua->pushinteger(object.look);",
        "        lua->settable(-3);",
        '        lua->pushstring("color");',
        "        lua->pushinteger(object.color);",
        "        lua->settable(-3);",
        '        lua->pushstring("direction");',
        "        lua->pushinteger(object.direction);",
        "        lua->settable(-3);",
        "        lua->settable(-3);",
        "    }",
        "    lua->settable(-3);",
    ],
    "game_resp::user_list": [
        '    lua->pushstring("users");',
        "    lua->new_table();",
        "    for (size_t i = 0; i < resp.users.size(); ++i)",
        "    {",
        "        const auto& user = resp.users[i];",
        "        lua->pushinteger(static_cast<lua_Integer>(i + 1));",
        "        lua->new_table();",
        '        lua->pushstring("nation");',
        "        lua->pushinteger(user.nation);",
        "        lua->settable(-3);",
        '        lua->pushstring("cls");',
        "        lua->pushinteger(user.cls);",
        "        lua->settable(-3);",
        '        lua->pushstring("promotion");',
        "        lua->pushinteger(user.promotion);",
        "        lua->settable(-3);",
        '        lua->pushstring("level");',
        "        lua->pushinteger(user.level);",
        "        lua->settable(-3);",
        '        lua->pushstring("color");',
        "        lua->pushinteger(user.color);",
        "        lua->settable(-3);",
        '        lua->pushstring("name");',
        "        lua->pushstring(user.name);",
        "        lua->settable(-3);",
        "        lua->settable(-3);",
        "    }",
        "    lua->settable(-3);",
    ],
}


def generate_marshal_functions(types: list[ProtocolType]) -> list[str]:
    lines: list[str] = []
    for t in types:
        if t.direction != "response":
            continue
        sym = mangle_symbol(t.type_key)
        lines.extend(
            [
                f"void marshal_lua_{sym}(lua_State* L, const fb::protocol::header& header)",
                "{",
                "    auto* lua = fb::lua::get(L);",
                "    if (lua == nullptr)",
                "        return;",
                "",
                f"    const auto& resp = static_cast<const {t.cpp_type}&>(header);",
                "    lua->new_table();",
            ]
        )
        for fld in t.fields:
            append_marshal_field(lines, fld)
        extras = SPECIAL_MARSHAL_EXTRAS.get(t.type_key)
        if extras:
            lines.extend(extras)
        lines.extend(["}", ""])
    return lines


def lua_builder_prologue() -> list[str]:
    return [
        "    auto lua = fb::lua::get(L);",
        "    if (lua == nullptr)",
        '        return luaL_error(L, "integration protocol requires active lua context");',
        "",
    ]


def lua_read_arg_lines(cpp_type: str, lua_index: int, var_name: str) -> list[str]:
    norm = normalize_param_type(cpp_type)
    if looks_like_enum_type(norm):
        enum_type = enum_type_for_codegen(cpp_type)
        return [
            f"    const auto {var_name} = fb::model::enum_value::enum_parse<{enum_type}>(lua->tostring({lua_index}));"
        ]
    if norm == "bool":
        return [f"    const auto {var_name} = lua->toboolean({lua_index});"]
    if norm in ("uint32_t", "uint16_t", "uint8_t", "int32_t", "int16_t", "int8_t"):
        return [
            f"    const auto {var_name} = static_cast<{norm}>(lua->tointeger({lua_index}));"
        ]
    if norm in ("std::string", "std::string_view"):
        return [f"    const auto {var_name} = lua->tostring({lua_index});"]
    if norm == "fb::model::point<uint16_t>":
        return [
            f"    fb::model::point<uint16_t> {var_name};",
            f"    if (lua->is_table({lua_index}))",
            "    {",
            f"        lua->rawgeti({lua_index}, 1);",
            f"        {var_name}.x = static_cast<uint16_t>(lua->tointeger(-1));",
            "        lua->pop(1);",
            f"        lua->rawgeti({lua_index}, 2);",
            f"        {var_name}.y = static_cast<uint16_t>(lua->tointeger(-1));",
            "        lua->pop(1);",
            "    }",
            "    else",
            "    {",
            f"        {var_name}.x = static_cast<uint16_t>(lua->tointeger({lua_index}));",
            f"        {var_name}.y = static_cast<uint16_t>(lua->tointeger({lua_index + 1}));",
            "    }",
        ]
    return []


def generate_protocol_builders(types: list[ProtocolType]) -> list[str]:
    lines: list[str] = []
    for t in types:
        if can_generate_builder(t) == False:
            continue

        sym = mangle_symbol(t.type_key)
        if t.has_default_ctor and not t.ctor_params:
            lines.extend(
                [
                    f"int lua_builder_{sym}(lua_State* L)",
                    "{",
                    *lua_builder_prologue(),
                    f"    lua_protocol::push_request(L, std::make_shared<{t.cpp_type}>());",
                    "    return 1;",
                    "}",
                    "",
                ]
            )
            continue
        if not t.ctor_params:
            continue

        arg_lines: list[str] = []
        arg_names: list[str] = []
        for index, (cpp_type, param_name) in enumerate(t.ctor_params, start=1):
            arg_lines.extend(lua_read_arg_lines(cpp_type, index, param_name))
            arg_names.append(param_name)

        ctor_args = ", ".join(arg_names)
        lines.append(f"int lua_builder_{sym}(lua_State* L)")
        lines.append("{")
        lines.extend(lua_builder_prologue())
        lines.extend(arg_lines)
        lines.append(f"    lua_protocol::push_request(L, std::make_shared<{t.cpp_type}>({ctor_args}));")
        lines.append("    return 1;")
        lines.append("}")
        lines.append("")
    return lines


def generate_lua_cpp(types: list[ProtocolType]) -> str:
    builder_types = [t for t in types if can_generate_builder(t)]

    lines: list[str] = [
        "// Auto-generated by tools/codegen/integration_protocol/generate.py",
        "// DO NOT EDIT",
        "",
        "#include <fb/bot/integration/lua_integration_protocol.h>",
        "#include <fb/bot/integration/protocol_registry.h>",
        "#include <fb/game/protocol.h>",
        "#include <fb/model/model.h>",
        "#include <fb/lua.h>",
        "",
        "#include <memory>",
        "#include <string>",
        "",
        "namespace fb::bot::integration {",
        "",
        "namespace game_reqs = fb::protocol::game::request;",
        "namespace game_resp = fb::protocol::game::response;",
        "",
        f"constexpr auto {BOT_CLIENT_VERSION_ALIAS} = {BOT_CLIENT_VERSION_VALUE};",
        "",
        "namespace detail {",
        "",
    ]
    lines.extend(generate_marshal_functions(types))
    lines.extend(
        [
            "} // namespace detail",
            "",
            "} // namespace fb::bot::integration",
            "",
            "namespace fb::bot::integration::lua_protocol {",
            "",
        ]
    )
    lines.extend(generate_protocol_builders(types))
    lines.append("void register_builders(lua_State* L)")
    lines.append("{")
    lines.append("    lua_newtable(L);")
    for t in builder_types:
        sym = mangle_symbol(t.type_key)
        lines.append(f'    lua_pushcfunction(L, lua_builder_{sym});')
        lines.append(f'    lua_setfield(L, -2, "{t.name}");')
    lines.append("}")
    lines.append("")
    lines.append("} // namespace fb::bot::integration::lua_protocol")
    lines.append("")
    return "\n".join(lines)


def generate_lua_h(types: list[ProtocolType]) -> str:
    lines: list[str] = [
        "// Auto-generated by tools/codegen/integration_protocol/generate.py",
        "// DO NOT EDIT",
        "",
        "struct lua_State;",
        "",
        "namespace fb::protocol {",
        "class header;",
        "}",
        "",
        "namespace fb::bot::integration::detail {",
        "",
    ]
    for t in types:
        if t.direction != "response":
            continue
        sym = mangle_symbol(t.type_key)
        lines.append(
            f"void marshal_lua_{sym}(lua_State* L, const fb::protocol::header& header);"
        )
    lines.extend(
        [
            "",
            "} // namespace fb::bot::integration::detail",
            "",
            "namespace fb::bot::integration::lua_protocol {",
            "",
            "void register_builders(lua_State* L);",
            "",
            "} // namespace fb::bot::integration::lua_protocol",
            "",
        ]
    )
    return "\n".join(lines)


def generate_cpp(types: list[ProtocolType]) -> str:
    lines: list[str] = [
        "// Auto-generated by tools/codegen/integration_protocol/generate.py",
        "// DO NOT EDIT",
        "",
        "#include <fb/bot/integration/protocol_registry.h>",
        "#include <fb/bot/integration/protocol_lua.generated.h>",
        "#include <fb/bot/game_controller.h>",
        "#include <fb/game/protocol.h>",
        "#include <fb/lua.h>",
        "",
        "namespace fb::bot::integration::detail {",
        "",
        f"constexpr auto {BOT_CLIENT_VERSION_ALIAS} = {BOT_CLIENT_VERSION_VALUE};",
        "",
    ]

    for t in types:
        sym = mangle_symbol(t.type_key)
        if t.direction == "response":
            lines.extend(
                [
                    f"void bind_{sym}(fb::bot::game_bot_controller& controller)",
                    "{",
                    f"    controller.bind<{t.cpp_type}>();",
                    "}",
                    "",
                    f"std::shared_ptr<fb::protocol::header> clone_{sym}(const fb::protocol::header& header)",
                    "{",
                    f"    return std::make_shared<{t.cpp_type}>(static_cast<const {t.cpp_type}&>(header));",
                    "}",
                    "",
                    f"std::shared_ptr<fb::protocol::header> create_{sym}()",
                    "{",
                    "    return nullptr;",
                    "}",
                    "",
                ]
            )
        else:
            lines.extend(
                [
                    f"void bind_{sym}(fb::bot::game_bot_controller&)",
                    "{",
                    "}",
                    "",
                    f"std::shared_ptr<fb::protocol::header> clone_{sym}(const fb::protocol::header&)",
                    "{",
                    "    return nullptr;",
                    "}",
                    "",
                    f"std::shared_ptr<fb::protocol::header> create_{sym}()",
                    "{",
                    (
                        f"    return std::make_shared<{t.cpp_type}>();"
                        if t.has_default_ctor
                        else "    return nullptr;"
                    ),
                    "}",
                    "",
                ]
            )

    lines.append("} // namespace fb::bot::integration::detail")
    lines.append("")
    lines.append("namespace fb::bot::integration {")
    lines.append("")
    lines.append("static const protocol_entry ENTRIES[] = {")

    for t in types:
        sym = mangle_symbol(t.type_key)
        lines.append("    {")
        lines.append(f'        "{t.type_key}",')
        lines.append(f'        "{t.name}",')
        direction = "protocol_direction::response" if t.direction == "response" else "protocol_direction::request"
        lines.append(f"        {direction},")
        lines.append(f"        0x{t.opcode:02X},")
        lines.append(f"        &detail::bind_{sym},")
        lines.append(f"        &detail::clone_{sym},")
        lines.append(f"        &detail::create_{sym},")
        if t.direction == "response":
            lines.append(f"        &detail::marshal_lua_{sym},")
        else:
            lines.append("        &protocol_registry::marshal_lua_stub,")
        lines.append("    },")

    lines.extend(
        [
            "};",
            "",
            "const protocol_entry* protocol_registry::entries()",
            "{",
            "    return ENTRIES;",
            "}",
            "",
            "} // namespace fb::bot::integration",
            "",
        ]
    )

    return "\n".join(lines)


def generate_h(types: list[ProtocolType]) -> str:
    return "\n".join(
        [
            "// Auto-generated by tools/codegen/integration_protocol/generate.py",
            "// DO NOT EDIT",
            "",
            f"#define FB_BOT_INTEGRATION_PROTOCOL_ENTRY_COUNT {len(types)}",
            "",
        ]
    )


def verify_generated(types: list[ProtocolType]) -> None:
    if not OUT_H.exists():
        raise SystemExit(f"missing generated header: {OUT_H}")
    text = OUT_H.read_text(encoding="utf-8")
    m = re.search(r"#define FB_BOT_INTEGRATION_PROTOCOL_ENTRY_COUNT (\d+)", text)
    if not m:
        raise SystemExit("generated header missing entry count macro")
    if int(m.group(1)) != len(types):
        raise SystemExit(
            f"registry out of date: header={m.group(1)} scanned={len(types)}; run generate.py"
        )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="verify generated files are up to date")
    args = parser.parse_args()

    types = scan_protocols()
    if args.check:
        verify_generated(types)
        print(f"integration protocol registry OK ({len(types)} entries)")
        return 0

    OUT_CPP.parent.mkdir(parents=True, exist_ok=True)
    OUT_H.parent.mkdir(parents=True, exist_ok=True)
    OUT_CPP.write_text(generate_cpp(types), encoding="utf-8", newline="\n")
    OUT_H.write_text(generate_h(types), encoding="utf-8", newline="\n")
    OUT_LUA_CPP.write_text(generate_lua_cpp(types), encoding="utf-8", newline="\n")
    OUT_LUA_H.write_text(generate_lua_h(types), encoding="utf-8", newline="\n")
    print(f"generated {len(types)} protocol entries")
    print(f"  {OUT_CPP.relative_to(ROOT)}")
    print(f"  {OUT_H.relative_to(ROOT)}")
    print(f"  {OUT_LUA_CPP.relative_to(ROOT)}")
    print(f"  {OUT_LUA_H.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
