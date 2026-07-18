"""build_weapons_v5.py — RocketLauncherV5 (2026-07-16 pass 5).

The rocket launcher finally gets real launcher art: shoulder tube with muzzle
flare ring, rear venturi, top optic (emissive reticle at sight-line height
0.14 m for the ADS alignment in C++), carry handle, fore/trigger grips and a
shoulder pad. Same contract as V3/V4: meters, +X down the bore, origin at the
trigger grip. GLB -> RawArt/WeaponsV5/.

Run inside Blender:  exec(open(r"<this file>").read())
"""
import bpy
import os

_HERE = os.path.expanduser(
    "~/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/Scripts/BlenderArt")
exec(open(os.path.join(_HERE, "build_weapons_v4.py")).read().split("def pistol()")[0])

OUT_DIR = os.path.join(PROJECT_ROOT, "RawArt", "WeaponsV5")
PREV_DIR = os.path.join(OUT_DIR, "previews_v5")
os.makedirs(OUT_DIR, exist_ok=True)
os.makedirs(PREV_DIR, exist_ok=True)


def rocket_launcher():
    b = B("RocketLauncherV5")
    b.cylx(0.28, 0, 0.10, 0.062, 0.98, STEEL, seg=18)               # main tube
    b.cylx(0.74, 0, 0.10, 0.075, 0.10, DARKST, seg=18)              # muzzle flare ring
    b.cylx(-0.24, 0, 0.10, 0.070, 0.09, DARKST, seg=18)             # rear venturi
    b.cylx(-0.30, 0, 0.10, 0.052, 0.06, STEEL, seg=14)              # venturi throat
    b.box(0.28, 0, 0.030, 0.30, 0.030, 0.020, RUBBER)               # shoulder pad (under tube)
    b.box(0.10, 0, 0.169, 0.16, 0.018, 0.014, POLYMER)              # carry handle bar
    b.box(0.025, 0, 0.140, 0.012, 0.018, 0.048, POLYMER)            # handle post rear
    b.box(0.175, 0, 0.140, 0.012, 0.018, 0.048, POLYMER)            # handle post front
    b.cylx(0.34, 0, 0.140, 0.016, 0.09, DARKST, seg=14)             # optic tube
    b.box(0.293, 0, 0.140, 0.003, 0.009, 0.009, REDDOT)             # emissive reticle (rear)
    b.box(0.34, 0, 0.118, 0.05, 0.014, 0.012, STEEL)                # optic mount
    b.box(0.0, 0, -0.012, 0.030, 0.026, 0.085, POLYMER, ry=0.16)    # trigger grip
    b.box(0.02, 0, 0.028, 0.045, 0.024, 0.012, POLYMER)             # guard
    b.box(0.032, 0, 0.042, 0.008, 0.016, 0.018, STEEL)              # trigger
    b.box(0.20, 0, -0.010, 0.024, 0.024, 0.055, POLYMER, ry=0.06)   # fore grip
    b.box(0.52, 0.055, 0.10, 0.10, 0.012, 0.030, TAN)               # side latch box
    b.box(-0.05, -0.058, 0.10, 0.16, 0.008, 0.026, TAN)             # sling saddle
    b.done(1.15)


wipe(); rocket_launcher()
print("[weapons_v5] ROCKET LAUNCHER COMPLETE ->", OUT_DIR)
