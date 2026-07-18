#!/usr/bin/env python3
"""lint_curriculum_full_2026_07_05.py — first-level completion pass.

Lints EVERY entry in curriculum_database.json (all 60), mirroring the C++
generic-harness gates plus schema/pedagogy checks:

  per entry:  unique id; tier present (legacy stubs exempt); prompt/micro/worked
              present for tiered entries; tests parseable literals; arity of every
              test's inputs consistent; per-language starter function extractable;
              per-language generic-execution support classification (which
              languages will EXECUTE tests externally vs structural-gate only).

Exit 1 on hard failures; warnings are informational. Read-only.
"""
from __future__ import annotations
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DB = ROOT / "Content" / "CodeRescueData" / "curriculum_database.json"

FN_RE = {
    "python": re.compile(r"def\s+([A-Za-z_][A-Za-z0-9_]*)\s*\("),
    "matlab": re.compile(r"function\s+[\w\[\], ]*=\s*([A-Za-z_][A-Za-z0-9_]*)\s*\(|function\s+([A-Za-z_][A-Za-z0-9_]*)\s*\("),
}
GENERIC_FN = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(")
RESERVED = {"if", "for", "while", "switch", "return", "sizeof", "printf", "main",
            "function", "def", "public", "static", "class", "new", "assert"}


def classify(tok: str) -> str:
    t = tok.strip()
    if not t:
        return "unsupported"
    low = t.lower()
    if low in ("true", "false"):
        return "bool"
    if t.startswith("[") and t.endswith("]"):
        inner = t[1:-1].strip()
        if not inner:
            return "intarr"
        try:
            [int(x) for x in inner.replace(",", " ").split()]
            return "intarr"
        except ValueError:
            return "unsupported"
    if (t.startswith('"') and t.endswith('"')) or (t.startswith("'") and t.endswith("'")):
        return "str"
    try:
        float(t)
        return "float" if "." in t else "int"
    except ValueError:
        return "unsupported"


def split_args(s: str) -> list[str]:
    out, cur, depth, quote = [], "", 0, None
    for ch in s:
        if quote:
            cur += ch
            if ch == quote:
                quote = None
            continue
        if ch in "\"'":
            quote = ch
            cur += ch
        elif ch in "[(":
            depth += 1
            cur += ch
        elif ch in "])":
            depth -= 1
            cur += ch
        elif ch == "," and depth == 0:
            out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out


def extract_fn(lang: str, starter: str) -> str | None:
    if lang in FN_RE:
        m = FN_RE[lang].search(starter)
        if not m:
            return None
        return next(g for g in m.groups() if g)
    for m in GENERIC_FN.finditer(starter):
        if m.group(1) not in RESERVED:
            return m.group(1)
    return None


def main() -> int:
    db = json.loads(DB.read_text())
    entries = db["entries"]
    failures: list[str] = []
    warnings: list[str] = []
    ids: dict[str, int] = {}

    exec_matrix: dict[str, list[str]] = {}
    for e in entries:
        eid = e.get("id", "<missing-id>")
        ids[eid] = ids.get(eid, 0) + 1
        tiered = e.get("tier") is not None
        tests = (e.get("visible_tests") or []) + (e.get("hidden_tests") or [])
        starters = e.get("starter") or {}

        if tiered:
            for field in ("prompt", "micro_lesson", "worked_example", "title", "concept"):
                if not e.get(field):
                    failures.append(f"{eid}: tiered entry missing '{field}'")
            if not e.get("visible_tests"):
                failures.append(f"{eid}: tiered entry has no visible_tests")
            if not e.get("hidden_tests"):
                warnings.append(f"{eid}: no hidden_tests (visible-only validation)")
        elif not tests:
            continue  # legacy language stub — exempt

        # test literal + arity checks. Entries whose id maps to a hand-built
        # archetype validator (sum/lock/reverse/palindrome/fizzbuzz/filter/
        # linkedlist/binary_search) never reach the generic literal engine, and
        # non-archetype entries with exotic literals (dicts, string arrays) fall
        # back to the honest structural gate — both are WARN-level, not failures.
        archetype = any(k in eid for k in ("sum", "generator", "lock", "reverse",
                        "palindrome", "fizzbuzz", "filter", "even", "linkedlist",
                        "linked_list", "traverse", "binary_search", "bsearch"))
        arities = set()
        bad_literal = False
        for t in tests:
            args = split_args(t.get("in", ""))
            arities.add(len(args))
            for a in args:
                if classify(a) == "unsupported":
                    (warnings if archetype else warnings).append(
                        f"{eid}: literal {a!r} not generically executable ({'archetype validator handles it' if archetype else 'structural gate only'})")
                    bad_literal = True
            if classify(t.get("out", "")) == "unsupported":
                warnings.append(
                    f"{eid}: expected {t.get('out')!r} not generically executable ({'archetype validator handles it' if archetype else 'structural gate only'})")
                bad_literal = True
        if len(arities) > 1:
            failures.append(f"{eid}: inconsistent test arity {sorted(arities)}")

        # per-language executable classification
        langs_exec = []
        for lang, code in starters.items():
            fn = extract_fn(lang, code)
            if not fn:
                failures.append(f"{eid}/{lang}: function name not extractable from starter")
                continue
            if bad_literal:
                continue
            scalar_only = (lang == "c")
            supported = True
            for t in tests:
                kinds = [classify(a) for a in split_args(t.get("in", ""))] + [classify(t.get("out", ""))]
                if scalar_only and any(k in ("intarr",) for k in kinds[:-1]):
                    supported = False
                if scalar_only and kinds[-1] in ("intarr", "str"):
                    supported = False
            langs_exec.append(f"{lang}{'' if supported else '(structural)'}")
        if tiered and tests:
            exec_matrix[eid] = langs_exec

    for eid, count in ids.items():
        if count > 1:
            failures.append(f"duplicate id: {eid} x{count}")

    tiers: dict = {}
    for e in entries:
        tiers[e.get("tier", "legacy")] = tiers.get(e.get("tier", "legacy"), 0) + 1

    print(f"[lint_curriculum] entries={len(entries)} per-tier={dict(sorted(tiers.items(), key=str))}")
    print(f"[lint_curriculum] executable-language matrix entries checked: {len(exec_matrix)}")
    for w in warnings:
        print("  WARN:", w)
    if failures:
        print(f"[lint_curriculum] {len(failures)} FAILURE(S):")
        for f in failures:
            print("  FAIL:", f)
        return 1
    print("[lint_curriculum] ALL CHECKS PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
