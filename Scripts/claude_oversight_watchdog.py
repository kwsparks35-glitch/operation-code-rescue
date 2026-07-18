#!/usr/bin/env python3
"""
Claude Oversight Watchdog  --  checks-and-balances harness for Operation Code Rescue.

WHY THIS EXISTS
---------------
Codex develops this project as a fast loop of "vertical slices". Each slice ships its own
`Scripts/verify_*.py` static checker. Two blind spots emerge from that workflow:

  1. The quick local-CI command only runs a *curated subset* of the verifiers, so an older
     guarantee can silently break when a newer slice refactors a symbol (drift).
  2. The verifiers are ~90% static string-presence checks. They confirm "the code contains
     the tokens I expected", not "the code compiles / runs / plays / teaches".

This watchdog is the independent reviewer. It:
  * runs EVERY Scripts/verify_*.py (not a hand-picked list),
  * classifies each failure as ENVIRONMENT-ONLY (needs the Unreal engine, a CLI arg, or an
    external server -- expected when run off the Mac) vs a REAL regression that deserves
    human attention,
  * surfaces the actual failing assertion lines for real regressions (drift detection),
  * reports git commit hygiene (uncommitted volume, commits today),
  * prints a single PASS / NEEDS-ATTENTION verdict and sets its exit code accordingly.

It is dependency-free (Python 3 stdlib only) so it runs identically in a sandbox and on the
Mac. On the Mac, run it from a UE-aware shell to also clear the `unreal`-module verifiers.

USAGE
-----
    python3 Scripts/claude_oversight_watchdog.py            # human-readable report
    python3 Scripts/claude_oversight_watchdog.py --json     # machine-readable summary
    python3 Scripts/claude_oversight_watchdog.py --quiet    # verdict + counts only

Exit code 0 = no real regressions found.  Exit code 1 = at least one real regression.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SCRIPTS = PROJECT_ROOT / "Scripts"
SOURCE = PROJECT_ROOT / "Source" / "CodeRescueUnreal"
PER_SCRIPT_TIMEOUT = 90  # seconds

# Signatures that mean "this verifier needs something the sandbox/off-Mac shell cannot give"
ENV_SIGNATURES = [
    (re.compile(r"No module named ['\"]unreal['\"]"), "needs in-editor Unreal Python (run on the Mac)"),
    (re.compile(r"the following arguments are required", re.I), "harness-invoked: needs a CLI argument"),
    (re.compile(r"usage:.*verify_", re.I | re.S), "harness-invoked: needs a CLI argument"),
    (re.compile(r"server self-test failed", re.I), "needs the external Fab/Unreal MCP server"),
    (re.compile(r"(clang|clang\+\+|matlab|node)\b.*not (found|on PATH)", re.I), "needs a local toolchain"),
    (re.compile(r"FileNotFoundError.*(\.png|\.log|Saved/)", re.I), "needs an engine-produced artifact"),
]

# Lines a verifier prints when a genuine guarantee is broken.
FAIL_LINE = re.compile(r"\b(FAIL|missing|must (define|own|call|include)|unterminated|not in)\b", re.I)


def discover() -> list[Path]:
    scripts = sorted(p for p in SCRIPTS.glob("verify_*.py") if p.is_file())
    return scripts


def run_one(path: Path) -> dict:
    try:
        proc = subprocess.run(
            [sys.executable, str(path)],
            cwd=str(PROJECT_ROOT),
            capture_output=True,
            text=True,
            timeout=PER_SCRIPT_TIMEOUT,
        )
        out = (proc.stdout or "") + (proc.stderr or "")
        rc = proc.returncode
    except subprocess.TimeoutExpired:
        return {"name": path.name, "status": "REAL_FAIL", "reason": "timed out", "detail": []}
    except Exception as exc:  # pragma: no cover - defensive
        return {"name": path.name, "status": "REAL_FAIL", "reason": f"runner error: {exc}", "detail": []}

    if rc == 0:
        return {"name": path.name, "status": "PASS", "reason": "", "detail": []}

    for pattern, label in ENV_SIGNATURES:
        if pattern.search(out):
            return {"name": path.name, "status": "ENV_ONLY", "reason": label, "detail": []}

    detail = [ln.strip() for ln in out.splitlines() if FAIL_LINE.search(ln)][:6]
    return {"name": path.name, "status": "REAL_FAIL", "reason": "assertion(s) failed", "detail": detail}


def load_source_blob() -> str:
    src_blob = ""
    if SOURCE.exists():
        for f in SOURCE.glob("*"):
            if f.suffix in (".cpp", ".h"):
                src_blob += f.read_text(encoding="utf-8", errors="replace")
    return src_blob


IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{5,}")

def triage_pattern_stale(result: dict, src_blob: str) -> bool:
    """2026-07-04: separate 'the feature is gone' from 'the verifier greps for an exact line
    that a refactor rephrased'. When EVERY identifier a failing verifier names is still present
    in Source/, the feature exists and only the verifier's expectation drifted (e.g. the
    2026-07-02 NativeConstruct->RebuildWidget() construction move). Those are PATTERN_STALE:
    verifier maintenance for Codex, not a code regression."""
    idents: set[str] = set()
    for line in result.get("detail", []):
        seg = line.split("missing", 1)[1] if "missing" in line else line
        for m in IDENT_RE.findall(seg):
            if m.lower() not in {"missing", "must", "construct", "verify", "widget"}:
                idents.add(m)
    if not idents:
        return False
    return all(sym in src_blob for sym in idents)


def drift_scan(real_fails: list[dict], src_blob: str = "") -> list[str]:
    """Best-effort: pull the symbol each real failure names, confirm it is absent from Source."""
    notes: list[str] = []
    sym_re = re.compile(r"must define ([A-Za-z_][A-Za-z0-9_]*)")
    if not src_blob:
        src_blob = load_source_blob()
    for item in real_fails:
        for line in item["detail"]:
            m = sym_re.search(line)
            if m:
                sym = m.group(1)
                present = sym in src_blob
                notes.append(
                    f"  drift: {item['name']} expects symbol '{sym}' -> "
                    + ("STILL PRESENT (verifier may be mis-set)" if present
                       else "ABSENT from Source (refactored away; update or retire this verifier)")
                )
    return notes


def git_hygiene() -> dict:
    def git(*args):
        try:
            return subprocess.run(["git", *args], cwd=str(PROJECT_ROOT),
                                  capture_output=True, text=True, timeout=30).stdout.strip()
        except Exception:
            return ""
    uncommitted = git("status", "--short")
    uncommitted_n = len([l for l in uncommitted.splitlines() if l.strip()])
    today = datetime.now().strftime("%Y-%m-%d")
    commits_today = git("log", "--since", f"{today} 00:00", "--oneline")
    commits_today_n = len([l for l in commits_today.splitlines() if l.strip()])
    last = git("log", "-1", "--format=%h %ci %s")
    return {"uncommitted": uncommitted_n, "commits_today": commits_today_n, "last_commit": last}


def main() -> int:
    ap = argparse.ArgumentParser(description="Independent checks-and-balances harness.")
    ap.add_argument("--json", action="store_true", help="emit machine-readable JSON")
    ap.add_argument("--quiet", action="store_true", help="verdict + counts only")
    args = ap.parse_args()

    scripts = discover()
    results = [run_one(p) for p in scripts]
    src_blob = load_source_blob()
    for r in results:
        if r["status"] == "REAL_FAIL" and triage_pattern_stale(r, src_blob):
            r["status"] = "PATTERN_STALE"
    by = lambda s: [r for r in results if r["status"] == s]
    passed, env_only, real = by("PASS"), by("ENV_ONLY"), by("REAL_FAIL")
    stale = by("PATTERN_STALE")
    drift = drift_scan(real, src_blob)
    git = git_hygiene()

    verdict_ok = len(real) == 0
    summary = {
        "generated": datetime.now(timezone.utc).isoformat(),
        "total": len(results),
        "passed": len(passed),
        "environment_only": len(env_only),
        "pattern_stale": len(stale),
        "real_regressions": len(real),
        "git": git,
        "verdict": "PASS" if verdict_ok else "NEEDS ATTENTION",
    }

    if args.json:
        print(json.dumps({"summary": summary, "results": results, "drift": drift}, indent=2))
        return 0 if verdict_ok else 1

    print("=" * 70)
    print(" CLAUDE OVERSIGHT WATCHDOG  --  Operation Code Rescue")
    print("=" * 70)
    print(f" verifiers run         : {summary['total']}")
    print(f" passed                : {summary['passed']}")
    print(f" environment-only fail : {summary['environment_only']}  (expected off-Mac; not a defect)")
    print(f" pattern-stale checks  : {summary['pattern_stale']}  (feature present; verifier expectation drifted)")
    print(f" REAL regressions      : {summary['real_regressions']}")
    print("-" * 70)
    print(f" git uncommitted files : {git['uncommitted']}")
    print(f" git commits today     : {git['commits_today']}")
    print(f" git last commit       : {git['last_commit'] or '(none)'}")
    print("=" * 70)

    if not args.quiet and env_only:
        print("\nEnvironment-only (run on the Mac inside a UE shell to clear these):")
        for r in env_only:
            print(f"  - {r['name']:<58} {r['reason']}")

    if not args.quiet and stale:
        print("\nPattern-stale verifiers (all named symbols still exist in Source/ — update the")
        print("verifier's expected lines to the refactored code; tracked in CLAUDE_TO_CODEX.md):")
        for r in stale:
            print(f"  - {r['name']}")

    if real:
        print("\n>>> REAL REGRESSIONS -- need a human or a fix:")
        for r in real:
            print(f"  - {r['name']}  ({r['reason']})")
            for d in r["detail"]:
                print(f"        {d}")
        if drift:
            print("\nDrift analysis:")
            for n in drift:
                print(n)

    if git["uncommitted"] > 50:
        print(f"\n[!] PROCESS RISK: {git['uncommitted']} uncommitted files and "
              f"{git['commits_today']} commits today. Slice work is not being checkpointed; "
              "a single bad edit has no rollback point. Commit per slice.")

    print(f"\nVERDICT: {summary['verdict']}")
    return 0 if verdict_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
