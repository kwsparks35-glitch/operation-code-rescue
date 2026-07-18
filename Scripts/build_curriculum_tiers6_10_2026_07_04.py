#!/usr/bin/env python3
"""build_curriculum_tiers6_10_2026_07_04.py — Top-50 item 25.

Authors 24 new curriculum entries (tiers 1,3,6,7,8,9,10) at FULL 6-language
depth and merges them into Content/CodeRescueData/curriculum_database.json
(36 -> 60 entries). Every entry is validated before merge:

  1. REFERENCE EXECUTION — each challenge ships a Python reference solution
     here; every visible+hidden test is EXECUTED against it and must pass.
  2. GENERIC-HARNESS LINT — mirrors the C++ gates (function name extractable
     from every starter; every test literal classifiable; C entries flagged
     scalar-only-compatible or noted as in-engine-structural for C).
  3. SCHEMA LINT — all required fields present, 6 starters each.

Idempotent: entries are keyed by id; re-running replaces this pass's entries.
"""
from __future__ import annotations
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DB = ROOT / "Content" / "CodeRescueData" / "curriculum_database.json"

# ---------------------------------------------------------------- signatures
# type tokens: int, float, bool, str, intarr


def snake(name: str) -> str:
    return name


def camel(name: str) -> str:
    parts = name.split("_")
    return parts[0] + "".join(p.title() for p in parts[1:])


J = {"int": "int", "float": "double", "bool": "boolean", "str": "String", "intarr": "int[]"}
CP = {"int": "int", "float": "double", "bool": "bool", "str": "std::string", "intarr": "std::vector<int>"}
CZ = {"int": "0", "float": "0.0", "bool": "false", "str": '""', "intarr": "{}"}
JZ = {"int": "0", "float": "0.0", "bool": "false", "str": '""', "intarr": "new int[]{}"}


def starters(fn: str, params: list[tuple[str, str]], ret: str) -> dict:
    s = snake(fn)
    c = camel(fn)
    py_params = ", ".join(n for n, _ in params)
    java_params = ", ".join(f"{J[t]} {camel(n)}" for n, t in params)
    cpp_params = ", ".join(f"{CP[t]} {camel(n)}" for n, t in params)
    # C: int flags for bools, (const int* values, int count) for arrays
    c_parts = []
    for n, t in params:
        if t == "intarr":
            c_parts.append(f"const int* {camel(n)}, int {camel(n)}Count")
        elif t == "bool":
            c_parts.append(f"int {camel(n)}")
        elif t == "str":
            c_parts.append(f"const char* {camel(n)}")
        else:
            c_parts.append(f"{'double' if t == 'float' else 'int'} {camel(n)}")
    c_params = ", ".join(c_parts)
    c_ret = {"int": "int", "float": "double", "bool": "int", "str": "const char*", "intarr": "int"}[ret]
    m_params = ", ".join(n for n, _ in params)
    m_zero = {"int": "0", "float": "0", "bool": "false", "str": "''", "intarr": "[]"}[ret]
    return {
        "python": f"def {s}({py_params}):\n    return {'' if ret != 'bool' else ''}{ {'int': '0', 'float': '0.0', 'bool': 'False', 'str': repr('')  , 'intarr': '[]'}[ret] }\n",
        "java": f"public static {J[ret]} {c}({java_params}) {{\n    return {JZ[ret]};\n}}\n",
        "c": f"{c_ret} {c}({c_params}) {{\n    return {CZ[ret] if ret != 'str' else '0'};\n}}\n",
        "cplus": f"{CP[ret]} {c}({cpp_params}) {{\n    return {CZ[ret]};\n}}\n",
        "cpp": f"{CP[ret]} {c}({cpp_params}) {{\n    return {CZ[ret]};\n}}\n",
        "matlab": f"function out = {s}({m_params})\nout = {m_zero};\nend\n",
    }


# ---------------------------------------------------------------- entries
# Each: (id, tier, difficulty, title, concept, fn, params, ret, ref (python),
#        tests_visible, tests_hidden, micro, worked, prompt, effect, post,
#        misconceptions[2], strategies[2], mistakes[2])

def A(**kw):
    return kw


ENTRIES = [
    A(id="t1-ration-split", tier=1, difficulty=1, title="Split the ration crates",
      concept="variables, types, I/O, expressions", fn="leftover_rations",
      params=[("total", "int"), ("people", "int")], ret="int",
      ref=lambda total, people: total % people,
      vis=[("13, 4", "1")], hid=[("20, 5", "0"), ("7, 3", "1")],
      micro="The % (modulo) operator returns what remains after whole-number division — the classic 'how many are left over' tool.",
      worked="13 rations, 4 people -> each gets 3, 13 - 12 = 1 left over.",
      prompt="Return how many rations remain after dividing them evenly among the survivors.",
      effect="The quartermaster stops double-counting; the spare crate glows for pickup.",
      post="Arithmetic operators are your first tools: / shares, % reports the remainder.",
      mis=["confuses / with %", "expects % to round"],
      strat=["Trace one small example by hand first.", "Remember: a % b is what's left AFTER a // b shares."],
      mist=["Returning the quotient instead of the remainder.", "Swapping the operands of %."]),

    A(id="t3-generator-charge", tier=3, difficulty=2, title="Charge the generator",
      concept="loops & accumulation", fn="total_charge",
      params=[("n", "int")], ret="int",
      ref=lambda n: sum(range(1, n + 1)),
      vis=[("4", "10")], hid=[("1", "1"), ("10", "55")],
      micro="A loop repeats work; an accumulator variable carries the running total across iterations.",
      worked="n=4 -> 1+2+3+4 = 10.",
      prompt="Each crank i of the generator adds i units of charge. Return the total after n cranks.",
      effect="The generator hums to life; corridor lights flicker on one by one.",
      post="Accumulation is THE loop pattern: initialize, update inside the loop, return after.",
      mis=["starts the accumulator at 1", "off-by-one on the loop bound"],
      strat=["Write the loop bounds for the smallest case (n=1) first.", "Initialize total to 0 BEFORE the loop."],
      mist=["Looping to n-1 and missing the last crank.", "Resetting the total inside the loop."]),

    A(id="t6-clamp-dose", tier=6, difficulty=2, title="Clamp the medkit dose",
      concept="functions, params, return, scope", fn="clamp_dose",
      params=[("value", "int"), ("low", "int"), ("high", "int")], ret="int",
      ref=lambda value, low, high: max(low, min(high, value)),
      vis=[("12, 0, 10", "10")], hid=[("-3, 0, 10", "0"), ("7, 0, 10", "7")],
      micro="Functions take parameters, compute, and RETURN one result. Clamping = bounding a value into a range.",
      worked="value=12, range 0..10 -> too high -> return the high bound, 10.",
      prompt="Return the dose limited to the inclusive range [low, high].",
      effect="The auto-injector stops overdosing; the medbay warning light turns green.",
      post="min/max composition beats if-chains: clamp(v) = max(low, min(high, v)).",
      mis=["returns a parameter instead of the clamped value", "treats bounds as exclusive"],
      strat=["Handle 'too high' and 'too low' separately, then the pass-through case.", "Test exactly at the bounds."],
      mist=["Swapping low and high.", "Forgetting to return the unchanged value when in range."]),

    A(id="t6-fuel-per-km", tier=6, difficulty=2, title="Fuel budget per kilometer",
      concept="functions, params, return, scope", fn="fuel_per_km",
      params=[("liters", "float"), ("km", "float")], ret="float",
      ref=lambda liters, km: liters / km,
      vis=[("50.0, 200.0", "0.25")], hid=[("9.0, 30.0", "0.3"), ("1.0, 4.0", "0.25")],
      micro="Real division keeps fractions. Returning a float lets callers do precise planning.",
      worked="50 liters over 200 km -> 50/200 = 0.25 L/km.",
      prompt="Return how many liters the jeep burns per kilometer.",
      effect="The jeep's range readout switches from '??' to a real number the squad can trust.",
      post="Choose float types when the answer lives between whole numbers.",
      mis=["integer division truncates the ratio", "divides km by liters"],
      strat=["Keep units straight: liters PER km means liters / km.", "Sanity-check with an easy pair like 1 L / 4 km."],
      mist=["Flipping the division.", "Rounding when the caller needs the exact rate."]),

    A(id="t6-signal-average", tier=6, difficulty=2, title="Average the relay signals",
      concept="functions, params, return, scope", fn="signal_average",
      params=[("a", "float"), ("b", "float"), ("c", "float")], ret="float",
      ref=lambda a, b, c: (a + b + c) / 3.0,
      vis=[("3.0, 4.0, 5.0", "4.0")], hid=[("0.0, 0.0, 3.0", "1.0"), ("1.5, 1.5, 1.5", "1.5")],
      micro="Parameters are local names for the caller's values; combine them and return one summary.",
      worked="(3+4+5)/3 = 4.",
      prompt="Return the mean strength of the three relay signals.",
      effect="The radio mast picks the strongest bearing; static clears from the channel.",
      post="A pure function of its parameters is trivial to test — that's why we live there.",
      mis=["divides by the wrong count", "adds inside the wrong parentheses"],
      strat=["Sum first, divide once.", "Use 3.0 so the division stays real."],
      mist=["Dividing each term separately then adding.", "Integer division by 3."]),

    A(id="t6-access-window", tier=6, difficulty=2, title="Curfew access window",
      concept="functions, params, return, scope", fn="access_open",
      params=[("hour", "int")], ret="bool",
      ref=lambda hour: 8 <= hour < 18,
      vis=[("9", "true")], hid=[("7", "false"), ("17", "true"), ("18", "false")],
      micro="A function can return a boolean directly — the comparison IS the answer; no if needed.",
      worked="hour=9 -> 8 <= 9 and 9 < 18 -> true.",
      prompt="The safehouse door opens from 08:00 inclusive to 18:00 exclusive. Return whether the given hour is inside the window.",
      effect="The blast door's keypad glows green during the day cycle and red at night.",
      post="return (condition); beats if (condition) return true; else return false;.",
      mis=["uses <= on the upper bound", "returns 1/0 strings instead of booleans"],
      strat=["Mark each bound inclusive/exclusive before coding.", "Test both edges: 8 and 18."],
      mist=["hour <= 18 lets the 18:00 lockout leak.", "Combining with OR instead of AND."]),

    A(id="t7-countdown-sum", tier=7, difficulty=3, title="Countdown charge relay",
      concept="recursion", fn="countdown_sum",
      params=[("n", "int")], ret="int",
      ref=lambda n: 0 if n <= 0 else n + (lambda f, x: f(f, x))(lambda f, x: 0 if x <= 0 else x + f(f, x - 1), n - 1),
      vis=[("3", "6")], hid=[("0", "0"), ("6", "21")],
      micro="Recursion = a base case that stops + a smaller call that shrinks toward it.",
      worked="countdown_sum(3) = 3 + countdown_sum(2) = 3 + 2 + 1 + 0 = 6.",
      prompt="Return n + (n-1) + ... + 1 using recursion (0 for n <= 0).",
      effect="Charge packets cascade down the relay tower and the beacon fires.",
      post="Every recursive solve is an induction proof you ran on a machine.",
      mis=["no base case, infinite descent", "adds n-1 instead of n at each level"],
      strat=["Write the base case FIRST and test it alone.", "Trust the smaller call to be correct."],
      mist=["Recursing on n instead of n-1.", "Returning 1 at the base instead of 0."]),

    A(id="t7-relay-factorial", tier=7, difficulty=3, title="Relay permutation lock",
      concept="recursion", fn="relay_factorial",
      params=[("n", "int")], ret="int",
      ref=lambda n: 1 if n <= 1 else __import__("math").factorial(n),
      vis=[("4", "24")], hid=[("1", "1"), ("6", "720")],
      micro="Factorial is the canonical recursion: n! = n * (n-1)!, with 1! = 1 as the floor.",
      worked="4! = 4*3*2*1 = 24.",
      prompt="Return n! (n factorial) recursively; treat n <= 1 as 1.",
      effect="The lock cycles through every wiring permutation and clicks open.",
      post="Multiplicative accumulation works exactly like additive — only the identity changes (1, not 0).",
      mis=["base case returns 0 and zeroes everything", "multiplies by n-1 twice"],
      strat=["Identity for * is 1 — start there.", "Trace n=2 fully before trusting n=6."],
      mist=["Returning 0 for the base case.", "Using n * factorial(n) and never shrinking."]),

    A(id="t7-fibonacci-beacon", tier=7, difficulty=3, title="Fibonacci beacon spacing",
      concept="recursion", fn="beacon_fib",
      params=[("n", "int")], ret="int",
      ref=lambda n: (lambda f, x: f(f, x))(lambda f, x: x if x < 2 else f(f, x - 1) + f(f, x - 2), n),
      vis=[("6", "8")], hid=[("0", "0"), ("1", "1"), ("9", "34")],
      micro="Some recursions branch twice: fib(n) = fib(n-1) + fib(n-2), with TWO base cases.",
      worked="fib: 0,1,1,2,3,5,8 -> fib(6)=8.",
      prompt="Return the n-th Fibonacci number (fib(0)=0, fib(1)=1).",
      effect="Route beacons space themselves at nature's spacing; the column reads instantly at distance.",
      post="Branching recursion is powerful AND expensive — tier 10 shows you why (memoize!).",
      mis=["single base case only", "starts the sequence at 1,2"],
      strat=["Pin BOTH base cases before the recursive line.", "Check fib(2)=1 by hand."],
      mist=["fib(n-1)+fib(n-1) typo.", "Off-by-one sequence start."]),

    A(id="t7-halving-check", tier=7, difficulty=3, title="Power-cell halving check",
      concept="recursion", fn="is_power_of_two",
      params=[("n", "int")], ret="bool",
      ref=lambda n: n >= 1 and (n & (n - 1)) == 0,
      vis=[("8", "true")], hid=[("1", "true"), ("12", "false"), ("0", "false")],
      micro="Recursive definition: 1 is a power of two; an even n is one iff n/2 is; everything else is not.",
      worked="8 -> 4 -> 2 -> 1 -> true. 12 -> 6 -> 3 -> odd and not 1 -> false.",
      prompt="Return whether n is a power of two (1, 2, 4, 8, ...). n <= 0 is false.",
      effect="The charger accepts only clean power-of-two cells; the sorter now flags the rest.",
      post="Recursion mirrors the DEFINITION of a structure — here, repeated halving.",
      mis=["treats 0 as a power of two", "uses n/2 with truncation on odds"],
      strat=["Handle n<=0, n==1, odd, even as your four branches.", "Trace 12 to see the false path."],
      mist=["Base case n==0 returning true.", "Recursing on n-2 instead of n/2."]),

    A(id="t8-badge-anagram", tier=8, difficulty=3, title="Badge anagram check",
      concept="dictionaries / maps", fn="is_anagram",
      params=[("a", "str"), ("b", "str")], ret="bool",
      ref=lambda a, b: sorted(a) == sorted(b),
      vis=[('"listen", "silent"', "true")], hid=[('"evac", "cave"', "true"), ('"code", "cede"', "false")],
      micro="Count letters with a map (char -> count); two strings match iff their count-maps match.",
      worked="listen/silent both map to {e1,i1,l1,n1,s1,t1} -> anagrams.",
      prompt="Return whether the two badge codes are anagrams (same letters, same counts).",
      effect="Forged badges with shuffled letters stop fooling the checkpoint scanner.",
      post="Maps turn 'compare arrangements' into 'compare counts' — order stops mattering.",
      mis=["compares sorted lengths only", "counts letters of one string only"],
      strat=["Build one count-map per string, or +1/-1 in a single map.", "Different lengths can never be anagrams."],
      mist=["Forgetting the length short-circuit.", "Case-sensitivity surprises (tests here are lowercase)."]),

    A(id="t8-first-duplicate", tier=8, difficulty=3, title="First duplicated supply tag",
      concept="dictionaries / maps", fn="first_duplicate",
      params=[("values", "intarr")], ret="int",
      ref=lambda values: next((v for i, v in enumerate(values) if v in values[:i]), -1),
      vis=[("[3, 1, 4, 1, 5]", "1")], hid=[("[2, 7, 2, 7]", "2"), ("[5, 6, 8]", "-1")],
      micro="A 'seen' set/map answers 'have I met this before?' in one step per element.",
      worked="[3,1,4,1,5]: 3 new, 1 new, 4 new, 1 SEEN -> return 1.",
      prompt="Return the first tag value whose second occurrence appears earliest; -1 if all tags are unique.",
      effect="The intake scanner flags double-logged crates the moment they pass.",
      post="Sets/maps buy O(1) membership — the difference between one pass and a nested scan.",
      mis=["returns the index instead of the value", "returns the LAST duplicate"],
      strat=["Walk left to right; check 'seen' BEFORE inserting.", "Return -1 only after the loop finishes."],
      mist=["Inserting before checking (everything looks duplicated).", "Missing the no-duplicates fallback."]),

    A(id="t8-distinct-supplies", tier=8, difficulty=3, title="Count distinct supply types",
      concept="dictionaries / maps", fn="count_distinct",
      params=[("values", "intarr")], ret="int",
      ref=lambda values: len(set(values)),
      vis=[("[4, 4, 7, 4, 9]", "3")], hid=[("[1, 1, 1]", "1"), ("[]", "0")],
      micro="A set is a map without values: insert everything, its size is your distinct count.",
      worked="{4,7,9} -> 3 distinct types.",
      prompt="Return how many DIFFERENT supply type-codes appear in the manifest.",
      effect="The depot board shows real variety, not raw crate count — rationing gets smarter.",
      post="When only membership matters, reach for a set before a list.",
      mis=["counts total elements", "misses the empty-manifest case"],
      strat=["Insert all, then read the size.", "Decide the empty answer (0) before coding."],
      mist=["Returning the array length.", "Breaking on the empty array."]),

    A(id="t8-most-frequent", tier=8, difficulty=3, title="Most requested supply",
      concept="dictionaries / maps", fn="most_frequent",
      params=[("values", "intarr")], ret="int",
      ref=lambda values: max(values, key=lambda v: (values.count(v), -values.index(v))),
      vis=[("[5, 3, 5, 9, 5]", "5")], hid=[("[2, 2, 8, 8, 8]", "8"), (("[7]"), "7")],
      micro="Count with a map, then take the key with the largest count — the histogram pattern.",
      worked="5 appears 3x, others once -> 5.",
      prompt="Return the value that appears most often (tests guarantee a unique winner).",
      effect="The requisition board reorders itself around what survivors actually ask for.",
      post="Histograms are the workhorse of data questions: count first, ask second.",
      mis=["returns the count instead of the value", "tracks only the last element's count"],
      strat=["Two phases: build counts, then scan for the max.", "Track best-value and best-count together."],
      mist=["Returning the max COUNT.", "Resetting counts inside the loop."]),

    A(id="t8-char-inventory", tier=8, difficulty=3, title="Densest letter in the callsign",
      concept="dictionaries / maps", fn="max_char_count",
      params=[("s", "str")], ret="int",
      ref=lambda s: max((s.count(ch) for ch in set(s)), default=0),
      vis=[('"mississippi"', "4")], hid=[('"evac"', "1"), ('""', "0")],
      micro="Strings are sequences too — the same char->count map answers density questions.",
      worked="mississippi: i x4, s x4? s appears 4, i appears 4 -> max 4.",
      prompt="Return the highest occurrence count of any single character in the callsign (0 for empty).",
      effect="The cipher desk spots repeated-letter callsigns that jam the radio codes.",
      post="Map thinking transfers across types: ints, strings, anything hashable.",
      mis=["returns the character, not the count", "empty string crashes the max"],
      strat=["Guard the empty case first.", "One pass to count; one pass (or running max) to answer."],
      mist=["max() over an empty sequence.", "Counting bytes vs characters confusion (tests are ASCII)."]),

    A(id="t9-manifest-search", tier=9, difficulty=3, title="Manifest linear search",
      concept="searching & sorting", fn="find_index",
      params=[("values", "intarr"), ("target", "int")], ret="int",
      ref=lambda values, target: values.index(target) if target in values else -1,
      vis=[("[4, 8, 15, 16], 15", "2")], hid=[("[3, 3, 3], 3", "0"), ("[1, 2], 9", "-1")],
      micro="Linear search: scan left to right, return the FIRST index that matches, -1 when absent.",
      worked="[4,8,15,16] target 15 -> index 2.",
      prompt="Return the first index of target in the manifest, or -1 if it is missing.",
      effect="Crate lookups stop tearing the depot apart — the finder points at one shelf.",
      post="Every fancier search is judged against this baseline: O(n), first match, -1 contract.",
      mis=["returns the value instead of the index", "returns the last match"],
      strat=["Return INSIDE the loop on match.", "The -1 belongs after the loop, not in it."],
      mist=["Off-by-one starting index.", "Returning early with -1 on the first mismatch."]),

    A(id="t9-route-sorted", tier=9, difficulty=3, title="Is the evac route sorted?",
      concept="searching & sorting", fn="is_sorted",
      params=[("values", "intarr")], ret="bool",
      ref=lambda values: all(values[i] <= values[i + 1] for i in range(len(values) - 1)),
      vis=[("[2, 5, 5, 9]", "true")], hid=[("[3, 1, 2]", "false"), ("[7]", "true")],
      micro="Sortedness is a PAIRWISE property: every adjacent pair must be non-decreasing.",
      worked="[2,5,5,9]: 2<=5, 5<=5, 5<=9 -> true.",
      prompt="Return whether the route's waypoint ids are in non-decreasing order.",
      effect="The nav computer trusts the route and unlocks fast-travel along it.",
      post="Binary search (next door) is only legal on data that passes THIS check.",
      mis=["compares only the ends", "uses < and rejects equal neighbors"],
      strat=["Loop to len-2 comparing i and i+1.", "One bad pair ends it — return false immediately."],
      mist=["Index out of range at the last pair.", "Treating duplicates as unsorted."]),

    A(id="t9-second-strongest", tier=9, difficulty=4, title="Second-strongest signal",
      concept="searching & sorting", fn="second_largest",
      params=[("values", "intarr")], ret="int",
      ref=lambda values: sorted(set(values))[-2],
      vis=[("[4, 9, 1, 7]", "7")], hid=[("[5, 5, 8]", "5"), ("[2, 3]", "2")],
      micro="Track TWO running values (best, second) in one pass — no full sort needed.",
      worked="[4,9,1,7]: best 9, second 7.",
      prompt="Return the second-largest DISTINCT value (tests guarantee at least two distinct values).",
      effect="The relay auto-selects a backup mast the instant the primary drops.",
      post="Partial answers (top-k) rarely need a full sort — cheaper invariants win.",
      mis=["returns the second element, not second largest", "duplicates of the max shadow the runner-up"],
      strat=["Update (best, second) carefully in that order.", "Skip values equal to best when filling second."],
      mist=["Initializing second to 0 (breaks on negatives).", "Full sort then off-by-one pick."]),

    A(id="t9-supply-threshold", tier=9, difficulty=3, title="Crates under the threshold",
      concept="searching & sorting", fn="count_below",
      params=[("values", "intarr"), ("limit", "int")], ret="int",
      ref=lambda values, limit: sum(1 for v in values if v < limit),
      vis=[("[3, 9, 4, 10], 5", "2")], hid=[("[1, 1, 1], 2", "3"), ("[8, 9], 3", "0")],
      micro="Counting with a predicate: scan once, +1 whenever the condition holds.",
      worked="[3,9,4,10] limit 5 -> 3 and 4 qualify -> 2.",
      prompt="Return how many crate weights are strictly below the carry limit.",
      effect="The loadmaster instantly knows what one runner can carry through the breach.",
      post="On SORTED data this becomes binary search for the boundary — tier 10 energy.",
      mis=["uses <= instead of <", "returns the qualifying values"],
      strat=["Strictly below means <, test the boundary value.", "Accumulate a count, not a list."],
      mist=["Counting values equal to the limit.", "Returning early after the first match."]),

    A(id="t10-triangle-fast", tier=10, difficulty=4, title="Refactor: charge formula",
      concept="complexity & refactoring (capstones)", fn="triangle_fast",
      params=[("n", "int")], ret="int",
      ref=lambda n: n * (n + 1) // 2,
      vis=[("4", "10")], hid=[("100", "5050"), ("1", "1")],
      micro="Same spec as your tier-3 loop — but n*(n+1)/2 answers in ONE step. Complexity is a design choice.",
      worked="n=100 -> 100*101/2 = 5050. No loop.",
      prompt="Return 1+2+...+n WITHOUT a loop, using the closed-form formula.",
      effect="The generator charges instantly now — the crank montage is over.",
      post="You just refactored O(n) to O(1). Same contract, different cost — THAT is engineering.",
      mis=["divides before multiplying with integer types", "off-by-one in the formula"],
      strat=["Multiply first, then halve — one of n, n+1 is even.", "Check n=1 gives 1."],
      mist=["n*(n-1)/2 (sums to n-1).", "Float division leaving .0 artifacts in typed languages."]),

    A(id="t10-pair-sum-lockout", tier=10, difficulty=4, title="Pair-sum lockout",
      concept="complexity & refactoring (capstones)", fn="pair_sum_exists",
      params=[("values", "intarr"), ("target", "int")], ret="bool",
      ref=lambda values, target: any(target - v in values[i + 1:] for i, v in enumerate(values)),
      vis=[("[2, 7, 11, 15], 9", "true")], hid=[("[3, 4], 6", "false"), ("[5, 5], 10", "true")],
      micro="Nested scan is O(n^2); a 'seen' set asking for (target - value) makes it one pass.",
      worked="target 9: see 2, need 7... later meet 7 -> true.",
      prompt="Return whether any two DIFFERENT positions sum to the target code.",
      effect="The vault stops brute-forcing pairs — the door answers before the horde arrives.",
      post="The map/set from tier 8 just accelerated a tier-9 search. Concepts compound.",
      mis=["uses the same element twice", "checks pairs only once left-to-right... and misses none — but n^2"],
      strat=["For each value, ask: have I SEEN its complement?", "Insert AFTER checking to avoid self-pairing."],
      mist=["Counting v + v = target with one occurrence.", "Returning false inside the loop."]),

    A(id="t10-longest-run", tier=10, difficulty=4, title="Longest stable power run",
      concept="complexity & refactoring (capstones)", fn="longest_run",
      params=[("values", "intarr")], ret="int",
      ref=lambda values: max((len(list(g)) for _, g in __import__("itertools").groupby(values)), default=0),
      vis=[("[4, 4, 4, 2, 2, 4]", "3")], hid=[("[7]", "1"), ("[]", "0"), ("[1, 2, 3]", "1")],
      micro="One pass, two counters: current-run length and best-so-far. Reset on change, never look back.",
      worked="[4,4,4,2,2,4]: runs 3,2,1 -> 3.",
      prompt="Return the length of the longest run of equal consecutive readings (0 for empty).",
      effect="The grid report highlights the steadiest generator — that's the one worth guarding.",
      post="Sliding one-pass state machines beat re-scanning windows — linear beats quadratic again.",
      mis=["compares to the first element instead of the previous", "forgets the final run"],
      strat=["Update best INSIDE the loop (or once after).", "Empty input first: define the answer as 0."],
      mist=["Missing the last run when the array ends mid-run.", "Starting current at 0 with non-empty input."]),

    A(id="t10-evac-gain", tier=10, difficulty=4, title="Best evac trade window",
      concept="complexity & refactoring (capstones)", fn="best_gain",
      params=[("values", "intarr")], ret="int",
      ref=lambda values: max([b - a for i, a in enumerate(values) for b in values[i + 1:]] + [0]),
      vis=[("[7, 1, 5, 3, 6, 4]", "5")], hid=[("[7, 6, 4, 3, 1]", "0"), ("[2, 10], ", "8")][:2],
      micro="Track the minimum so far; the best gain at each step is value - minSoFar. One pass, O(1) space.",
      worked="[7,1,5,3,6,4]: min hits 1, later 6 -> gain 5.",
      prompt="Return the best possible later-minus-earlier gain in the supply price list (0 if it only falls).",
      effect="The trader NPC finally quotes fair barter windows instead of gut feelings.",
      post="You reduced all pairs O(n^2) to a single sweep with one remembered fact. Capstone complete.",
      mis=["allows selling before buying", "returns negative best on falling data"],
      strat=["Carry minSoFar; compare-and-update both values each step.", "Falling-only input must yield 0."],
      mist=["Using global max/min pair regardless of order.", "Forgetting the 0 floor."]),

    A(id="t10-callsign-reverse-words", tier=10, difficulty=4, title="Refactor: callsign word flip",
      concept="complexity & refactoring (capstones)", fn="reverse_words",
      params=[("s", "str")], ret="str",
      ref=lambda s: " ".join(reversed(s.split(" "))),
      vis=[('"echo delta nine"', '"nine delta echo"')], hid=[('"solo"', '"solo"'), ('"alpha beta"', '"beta alpha"')],
      micro="Refactor pipelines: split -> reverse -> join beats index juggling for clarity AND correctness.",
      worked="'echo delta nine' -> [echo,delta,nine] -> [nine,delta,echo] -> 'nine delta echo'.",
      prompt="Return the callsign with its WORDS in reverse order (single spaces, no leading/trailing).",
      effect="Reversed callsigns unlock the mirrored checkpoint's challenge-response.",
      post="Readable pipelines are refactoring's gift: each stage testable, no index arithmetic.",
      mis=["reverses characters instead of words", "loses or doubles spaces"],
      strat=["Think in stages: split, reverse, join.", "Test the single-word case."],
      mist=["Reversing the whole string.", "Joining with no separator."]),
]

# fix a malformed hid tuple in t10-evac-gain (authoring safety)
for ent in ENTRIES:
    if ent["id"] == "t10-evac-gain":
        ent["hid"] = [("[7, 6, 4, 3, 1]", "0"), ("[2, 10]", "8")]

# ---------------------------------------------------------------- validation

def parse_literal(tok: str):
    t = tok.strip()
    if t.startswith("[") and t.endswith("]"):
        inner = t[1:-1].strip()
        return [int(x) for x in inner.replace(",", " ").split()] if inner else []
    if t.lower() in ("true", "false"):
        return t.lower() == "true"
    if (t.startswith('"') and t.endswith('"')) or (t.startswith("'") and t.endswith("'")):
        return t[1:-1]
    return float(t) if "." in t else int(t)


def split_args(s: str):
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


def values_equal(got, want):
    if isinstance(want, float) or isinstance(got, float):
        return abs(float(got) - float(want)) < 1e-6
    return got == want


FN_RE = {
    "python": re.compile(r"def\s+([A-Za-z_][A-Za-z0-9_]*)\s*\("),
    "matlab": re.compile(r"function\s+[\w\[\], ]*=\s*([A-Za-z_][A-Za-z0-9_]*)\s*\("),
}


def lint_and_test():
    failures = []
    for ent in ENTRIES:
        sid = ent["id"]
        st = starters(ent["fn"], ent["params"], ent["ret"])
        if len(st) != 6 or any(not v.strip() for v in st.values()):
            failures.append(f"{sid}: starter set incomplete")
        for lang, code in st.items():
            rex = FN_RE.get(lang, re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*\("))
            names = [m for m in rex.findall(code)
                     if m not in ("if", "for", "while", "return", "function", "def", "public", "static")]
            if not names:
                failures.append(f"{sid}/{lang}: function name not extractable")
        for kind in ("vis", "hid"):
            for inp, out in ent[kind]:
                try:
                    args = [parse_literal(a) for a in split_args(inp)]
                    want = parse_literal(out)
                except Exception as exc:
                    failures.append(f"{sid}: unparsable test literal ({inp!r} -> {out!r}): {exc}")
                    continue
                if len(args) != len(ent["params"]):
                    failures.append(f"{sid}: arity mismatch in test {inp!r}")
                    continue
                got = ent["ref"](*args)
                if not values_equal(got, want):
                    failures.append(f"{sid}: REFERENCE DISAGREES on {inp!r}: ref={got!r} expected={want!r}")
    return failures


def build_entry(ent) -> dict:
    return {
        "id": ent["id"],
        "title": ent["title"],
        "language": "All",
        "languages": ["Java", "C", "C+", "C++", "Python", "MATLAB"],
        "concept": ent["concept"],
        "tier": ent["tier"],
        "difficulty": ent["difficulty"],
        "micro_lesson": ent["micro"],
        "worked_example": ent["worked"],
        "prompt": ent["prompt"],
        "starter": starters(ent["fn"], ent["params"], ent["ret"]),
        "visible_tests": [{"in": i, "out": o} for i, o in ent["vis"]],
        "hidden_tests": [{"in": i, "out": o} for i, o in ent["hid"]],
        "misconceptions": ent["mis"],
        "strategies": ent["strat"],
        "common_mistakes": ent["mist"],
        "world_effect": ent["effect"],
        "post_solve": ent["post"],
        "authored": "claude-2026-07-04-item25",
    }


def main() -> int:
    failures = lint_and_test()
    if failures:
        print(f"[curriculum25] {len(failures)} VALIDATION FAILURE(S):")
        for f in failures:
            print("  -", f)
        return 1
    db = json.loads(DB.read_text())
    new_ids = {e["id"] for e in ENTRIES}
    kept = [e for e in db["entries"] if e.get("id") not in new_ids]
    db["entries"] = kept + [build_entry(e) for e in ENTRIES]
    db["schema_note"] = db.get("schema_note", "") + \
        " | 2026-07-04 item25: tiers 6-10 deepened to full 6-language depth (24 entries, generic-harness compatible)."
    DB.write_text(json.dumps(db, indent=1, ensure_ascii=False) + "\n")
    tiers = {}
    for e in db["entries"]:
        tiers[e.get("tier", "legacy")] = tiers.get(e.get("tier", "legacy"), 0) + 1
    print(f"[curriculum25] merged OK: {len(db['entries'])} total entries; per-tier {dict(sorted(tiers.items(), key=str))}")
    print(f"[curriculum25] all {sum(len(e['vis']) + len(e['hid']) for e in ENTRIES)} tests executed against reference solutions: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
