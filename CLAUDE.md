# FB Common Rules

C++ 2D MMORPG server with C# services and Lua scripts. Talk to the user in Korean; code, comments and commits are English.

Path-scoped rules live in `.claude/rules/` and load when matching files are read: `code-style.md`, `cpp-standards.md`, `co-builder.md`, `csharp-standard.md`, `lua-standard.md`, `npc-prev-button.md`, `id-based-scripting.md`, `failure-recovery.md`, `protocol-651.md`, `vcxproj.md`. Read the relevant one before work in its area even if it has not loaded yet.

## Generated code — never edit or review by hand
`include/fb/model/*`, `include/fb/protocol/flatbuffer/*`, `server/http/FlatBuffer/protocol.cs`, `server/http/fb/protocol/raw/*`. Change `protocol/*.fbs` and regenerate with `tools/flatbuffer-ex`.

## Limits
- Build or run tests only when the user asks.
- Never `git submodule update` FlatBufferEx. Never edit an applied migration; add a new one.
- Kill only FB processes under `D:\git\fb\server\*`.

## No sub-agents
Do every task with your own tools (Bash/PowerShell, Grep, Read, Edit, MCP). Never use the `Agent` tool or any sub-agent (Explore, general-purpose, Plan, code-review, security-review, workflows, and others) unless the user explicitly asks for a named one.

## Comments
Do not add comments in C++, C#, Lua, Razor or test scripts. This includes constraint notes, explanations of a fix, section labels and doc comments; the commit message carries the reasoning. Keep existing comments when moving code. Before finishing, remove every comment line you added. The only exception is migration column `COMMENT`s, which follow existing migrations.

## Fix design
Before writing a fix:
1. Classify it. Accumulation: find when rows are always safe to delete and prefer a data-driven cleanup job (`.claude/rules/failure-recovery.md`). Ordering: define the comparison key (revision) first. Do not start by plugging the path the reviewer named.
2. For a multi-step change write a failure matrix: the process stops after a step, a concurrent request runs between steps, a step throws. Any permanently wrong outcome rejects the design.
3. Compare every listed option against the matrix, including ones that add a job or table. Pick the one whose correctness does not depend on timing; "simple" means few failure windows, not few lines.
4. A second residual finding on the same issue means stop patching, re-classify and redesign.

When reporting: state the assumptions and the cases not checked (never "all cases checked" without the matrix), and each remaining risk with its effect and whether it is transient or permanent.

Commits: one issue, one commit after this analysis. A follow-up commit only for a finding outside the matrix, and its message says what the matrix missed.

## Commits
- Commit only when the user says "commit"/"커밋". Never commit `server/bot/config/config.dev.json`.
- Before staging run `local-scripts\formatting\clang-format-all.bat`, then `git --no-pager status` and `git --no-pager diff HEAD`. Stage only what belongs to the change (report unrelated formatter or editor changes), then review `git --no-pager diff --staged`.
- Subject `<type>: <imperative subject>` in English, ≤50 chars, no emoji; types feat, fix, refactor, docs, style, test, chore, script. End the subject with every related issue (`#695`), found via `gh issue list --state open`, assigned or not.
- At most two `-m`: the subject, then one string of `- ` bullets naming every changed area.
- One commit per task. To fix commits not yet pushed, amend or fixup instead of adding commits.
- No AI attribution anywhere: no `Co-Authored-By` trailer, no "Generated with Claude Code" line, in commits or PR bodies. The author is the configured git user only.

## Pull requests
1. Compare `develop` with `origin/release/latest` (`git --no-pager log origin/release/latest..HEAD`, `git --no-pager diff --stat`); include every unreleased change.
2. Draft the title and full body in English, no emoji, and show it to the user. Run `gh pr create` (base `origin/release/latest`) only after explicit approval ("생성해줘", "create it").
3. Body: `## Summary` (1-3 sentences), then one `## <Area>` section of bullets per changed area. No test plan, todo or checkbox sections unless asked.

## Shell (PowerShell, non-interactive)
- `git --no-pager` for every git command; nothing may wait for input.
- Read and write files with explicit UTF-8 (`-Encoding UTF8`, `UTF8Encoding($false)`).
