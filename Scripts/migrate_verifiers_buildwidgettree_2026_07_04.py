#!/usr/bin/env python3
"""migrate_verifiers_buildwidgettree_2026_07_04.py

One-time verifier maintenance (2026-07-04): the 2026-07-02 widget-construction
refactor moved every widget's UI build from NativeConstruct() into
BuildWidgetTreeNow() (NativeConstruct/RebuildWidget are now thin wrappers that
call it). Dozens of slice verifiers extract the NativeConstruct body and grep
for construction lines, so they all report "missing" for features that exist.

This script patches each verifier's `function_body` helper: when an extracted
NativeConstruct body is just the wrapper (contains "BuildWidgetTreeNow();"),
it transparently re-extracts the real builder body. No expectations are
weakened — the same tokens are still required, in the function where the code
actually lives now. Idempotent; prints a per-file report.
"""
from __future__ import annotations
import re
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent
MARK = "2026-07-04 BuildWidgetTreeNow migration"

patched, skipped, already = [], [], []

for path in sorted(SCRIPTS.glob("verify_*.py")):
    text = path.read_text(encoding="utf-8", errors="replace")
    if "def function_body" not in text:
        continue
    if MARK in text:
        already.append(path.name)
        continue

    m = re.search(r"def function_body\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*(?::[^,]*)?,\s*([A-Za-z_][A-Za-z0-9_]*)", text)
    if not m:
        skipped.append((path.name, "unparsable def signature"))
        continue
    src_param, sig_param = m.group(1), m.group(2)

    # Find the def block: from 'def function_body' to the next top-level 'def '/statement.
    start = text.find("def function_body")
    nxt = re.search(r"\n(?:def |[A-Za-z_@])", text[start + 10:])
    end = (start + 10 + nxt.start() + 1) if nxt else len(text)
    block = text[start:end]

    # Patch every plain `return <expr>` in the block except `return ""` fall-throughs.
    lines = block.splitlines(keepends=True)
    out_lines = []
    changed = False
    for line in lines:
        stripped = line.strip()
        mret = re.match(r"return\s+(.+)$", stripped)
        if mret and mret.group(1) not in ('""', "''"):
            indent = line[: len(line) - len(line.lstrip())]
            expr = mret.group(1)
            out_lines.append(f"{indent}_cr_body = {expr}  # {MARK}\n")
            out_lines.append(f"{indent}if \"::NativeConstruct\" in {sig_param} and \"BuildWidgetTreeNow();\" in _cr_body:\n")
            out_lines.append(f"{indent}    return function_body({src_param}, {sig_param}.replace(\"::NativeConstruct\", \"::BuildWidgetTreeNow\"))\n")
            out_lines.append(f"{indent}return _cr_body\n")
            changed = True
        else:
            out_lines.append(line)
    if not changed:
        skipped.append((path.name, "no patchable return"))
        continue

    path.write_text(text[:start] + "".join(out_lines) + text[end:], encoding="utf-8")
    patched.append(path.name)

print(f"[migrate] patched={len(patched)} already={len(already)} skipped={len(skipped)}")
for name in patched:
    print("  patched:", name)
for name, why in skipped:
    print("  SKIPPED:", name, "--", why)
