#!/usr/bin/env python3
"""Split infra/db/latest.sql into scope-specific baseline migration files."""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LATEST_SQL = ROOT / "latest.sql"
MIGRATIONS_DIR = ROOT / "migrations"
BASELINE_VERSION = "20260630220000"

UNIFIED_TABLES = {
    "marketplace_listing",
    "marketplace_listing_archive",
    "marketplace_purchase",
    "marketplace_statistics",
}

WORLD_GLOBAL_TABLES = {
    "name_registry",
    "clan_name",
}

WORLD_LOG_TABLES = {
    "log",
}

WORLD_GLOBAL_PROCEDURES = {
    "USP_NAME_GET_ID",
    "USP_NAME_SET",
    "USP_CLAN_NAME_DELETE",
}

WORLD_DATA_PROCEDURES = {
    "USP_BULLETIN_ADD",
    "USP_BULLETIN_DELETE",
    "USP_BULLETIN_UPDATE",
    "USP_BULLETIN_GET",
    "USP_BULLETIN_GET_LIST",
    "USP_MAIL_GET_SUMMARY_LIST",
    "USP_MAIL_GET_LIST",
    "USP_MAIL_COUNT_BY_USER",
    "USP_MAIL_READ",
    "USP_MAIL_WRITE",
    "USP_MAIL_WRITE_MANY",
    "USP_MAIL_DELIVER_SYSTEM_MANY",
}


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def extract_create_tables(sql: str) -> dict[str, str]:
    pattern = re.compile(
        r"CREATE TABLE `(?P<name>[^`]+)` \([\s\S]*?\) ENGINE=\w+[^\n]*;",
        re.MULTILINE,
    )
    return {match.group("name"): match.group(0) for match in pattern.finditer(sql)}


def extract_procedures(sql: str) -> dict[str, str]:
    procedures: dict[str, str] = {}
    pattern = re.compile(
        r"DELIMITER ;;\s*"
        r"CREATE (?:DEFINER=`[^`]+`@`[^`]+` )?PROCEDURE `(?P<name>[^`]+)`"
        r"(?P<signature_and_body>[\s\S]*?)"
        r"END ;;\s*"
        r"DELIMITER ;",
        re.MULTILINE,
    )

    for match in pattern.finditer(sql):
        name = match.group("name")
        signature_and_body = match.group("signature_and_body").strip()
        procedures[name] = (
            f"DROP PROCEDURE IF EXISTS `{name}`;\n"
            "DELIMITER $$\n"
            f"CREATE PROCEDURE `{name}`{signature_and_body}\nEND$$\n"
            "DELIMITER ;"
        )

    return procedures


def table_scope(table_name: str) -> str:
    if table_name in UNIFIED_TABLES:
        return "unified"
    if table_name in WORLD_GLOBAL_TABLES:
        return "world-global"
    if table_name in WORLD_LOG_TABLES:
        return "world-log-data"
    return "world-data"


def procedure_scope(procedure_name: str) -> str:
    if procedure_name in WORLD_GLOBAL_PROCEDURES:
        return "world-global"
    if procedure_name in WORLD_DATA_PROCEDURES:
        return "world-data"
    raise RuntimeError(f"Unknown procedure scope for {procedure_name}")


def world_data_table_order() -> list[str]:
    return [
        "user",
        "achievement",
        "ban",
        "bulletin",
        "bulletin_sequence",
        "character_realtime_state",
        "clan",
        "clan_member",
        "group",
        "item",
        "mail",
        "mail_sequence",
        "marketplace_pending",
        "option",
        "quest",
        "spell",
        "system_mail",
        "storage_box",
        "system_storage_box",
        "marriage",
        "write_back_failure",
    ]


def build_scope_sql(
    scope: str,
    tables: dict[str, str],
    procedures: dict[str, str],
) -> str:
    lines = [
        "-- @transaction off",
        "SET FOREIGN_KEY_CHECKS=0;",
        "SET NAMES utf8mb4;",
        "",
    ]

    if scope == "unified":
        table_names = sorted(UNIFIED_TABLES)
    elif scope == "world-global":
        table_names = sorted(WORLD_GLOBAL_TABLES)
    elif scope == "world-log-data":
        table_names = sorted(WORLD_LOG_TABLES)
    elif scope == "world-data":
        ordered = world_data_table_order()
        table_names = [name for name in ordered if name in tables]
        remaining = sorted(set(tables) - set(table_names) - UNIFIED_TABLES - WORLD_GLOBAL_TABLES - WORLD_LOG_TABLES)
        table_names.extend(remaining)
    else:
        raise RuntimeError(f"Unsupported scope {scope}")

    for table_name in table_names:
        if table_name not in tables:
            raise RuntimeError(f"Missing CREATE TABLE for {table_name} in latest.sql")
        lines.append(tables[table_name])
        lines.append("")

    if scope == "world-global":
        proc_names = sorted(WORLD_GLOBAL_PROCEDURES)
    elif scope == "world-data":
        proc_names = sorted(WORLD_DATA_PROCEDURES)
    else:
        proc_names = []

    for procedure_name in proc_names:
        if procedure_name not in procedures:
            raise RuntimeError(f"Missing procedure {procedure_name} in latest.sql")
        lines.append(procedures[procedure_name])
        lines.append("")

    lines.append("SET FOREIGN_KEY_CHECKS=1;")
    lines.append("")
    return "\n".join(lines)


def main() -> None:
    sql = read_text(LATEST_SQL)
    tables = extract_create_tables(sql)
    procedures = extract_procedures(sql)

    scopes = {
        "unified": build_scope_sql("unified", tables, procedures),
        "world-global": build_scope_sql("world-global", tables, procedures),
        "world-data": build_scope_sql("world-data", tables, procedures),
        "world-log-data": build_scope_sql("world-log-data", tables, procedures),
    }

    for scope, content in scopes.items():
        scope_dir = MIGRATIONS_DIR / scope
        scope_dir.mkdir(parents=True, exist_ok=True)
        for path in scope_dir.glob("*.sql"):
            path.unlink()

        output_path = scope_dir / f"{BASELINE_VERSION}_baseline.sql"
        output_path.write_text(content, encoding="utf-8", newline="\n")
        print(f"Wrote {output_path}")


if __name__ == "__main__":
    main()
