#!/usr/bin/env python3
"""Render a dependency-light 3D preview for the curriculum-first gallery."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


PROJECT_ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = PROJECT_ROOT / "Saved" / "VisualReview"
OUTPUT_PATH = OUTPUT_DIR / "curriculum_first_review_gallery_render.png"

WIDTH = 1920
HEIGHT = 1080


STATIONS = [
    ("SUM RETURN", "visible 20+15+10", "hidden zero and mixed totals", (65, 189, 244)),
    ("LOCK BOOLEAN", "visible true && true", "hidden unsafe pairs stay shut", (88, 240, 140)),
    ("REVERSE STRING", "visible rescue -> eucser", "hidden city packet reverses", (183, 122, 255)),
    ("PALINDROME", "visible racecar passes", "hidden mirror and impostor", (166, 107, 234)),
    ("FIZZBUZZ", "visible 1..15 sequence", "hidden longer beacon sweep", (255, 184, 60)),
    ("EVEN FILTER", "visible keep 2,4,6", "hidden odd-only and order", (73, 217, 121)),
    ("LINKED LIST", "visible count to sentinel", "hidden changed starts", (90, 174, 234)),
    ("BINARY SEARCH", "visible middle target", "hidden first/last/not-found", (242, 85, 69)),
]


@dataclass
class Box:
    center: tuple[float, float, float]
    size: tuple[float, float, float]
    color: tuple[int, int, int]
    label: str = ""


def shade(color: tuple[int, int, int], amount: float) -> tuple[int, int, int]:
    return tuple(max(0, min(255, int(channel * amount))) for channel in color)


def project(point: tuple[float, float, float]) -> tuple[float, float]:
    x, y, z = point
    return (
        WIDTH / 2 + (x - y) * 115.0,
        765.0 + (x + y) * 51.0 - z * 120.0,
    )


def box_vertices(box: Box) -> dict[str, tuple[float, float, float]]:
    cx, cy, cz = box.center
    sx, sy, sz = box.size
    x0, x1 = cx - sx / 2, cx + sx / 2
    y0, y1 = cy - sy / 2, cy + sy / 2
    z0, z1 = cz - sz / 2, cz + sz / 2
    return {
        "lbf": (x0, y0, z0),
        "rbf": (x1, y0, z0),
        "rtf": (x1, y1, z0),
        "ltf": (x0, y1, z0),
        "lbb": (x0, y0, z1),
        "rbb": (x1, y0, z1),
        "rtb": (x1, y1, z1),
        "ltb": (x0, y1, z1),
    }


def draw_box(draw: ImageDraw.ImageDraw, box: Box) -> None:
    v = box_vertices(box)
    faces = [
        (["lbb", "rbb", "rtb", "ltb"], shade(box.color, 1.20)),
        (["rbf", "rtf", "rtb", "rbb"], shade(box.color, 0.84)),
        (["ltf", "rtf", "rtb", "ltb"], shade(box.color, 0.72)),
        (["lbf", "rbf", "rbb", "lbb"], shade(box.color, 0.96)),
        (["lbf", "ltf", "ltb", "lbb"], shade(box.color, 0.62)),
    ]
    for keys, color in faces:
        points = [project(v[key]) for key in keys]
        draw.polygon(points, fill=color, outline=(18, 26, 34))


def draw_text_center(
    draw: ImageDraw.ImageDraw,
    xy: tuple[float, float],
    text: str,
    font: ImageFont.ImageFont,
    fill: tuple[int, int, int],
    anchor: str = "mm",
) -> None:
    draw.multiline_text(xy, text, font=font, fill=fill, anchor=anchor, align="center", spacing=5)


def add_character(boxes: list[Box], x: float, y: float, color: tuple[int, int, int]) -> None:
    boxes.append(Box((x, y, 0.55), (0.18, 0.15, 0.88), color))
    boxes.append(Box((x, y, 1.12), (0.22, 0.20, 0.22), (239, 212, 170)))
    boxes.append(Box((x, y - 0.18, 0.76), (0.46, 0.06, 0.10), shade(color, 1.18)))


def render() -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    image = Image.new("RGB", (WIDTH, HEIGHT), (9, 15, 22))
    draw = ImageDraw.Draw(image)

    title_font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial Bold.ttf", 42)
    label_font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial Bold.ttf", 21)
    small_font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 16)
    tiny_font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 13)

    for i in range(0, HEIGHT, 8):
        tone = 15 + int(i / HEIGHT * 26)
        draw.line([(0, i), (WIDTH, i)], fill=(8, tone, tone + 8))

    boxes: list[Box] = [
        Box((0.0, 0.46, -0.06), (7.9, 4.25, 0.10), (22, 34, 43)),
        Box((0.0, -1.92, 0.98), (8.0, 0.16, 1.85), (17, 26, 34)),
        Box((0.0, -1.98, 2.02), (8.0, 0.18, 0.24), (47, 70, 88)),
    ]

    label_positions: list[tuple[tuple[float, float], str, ImageFont.ImageFont, tuple[int, int, int]]] = []
    label_positions.append(((WIDTH / 2, 94), "CURRICULUM-FIRST REVIEW GALLERY", title_font, (246, 201, 104)))
    label_positions.append(((WIDTH / 2, 145), "Visible test + hidden test + mistake marker + mentor + survivor payoff for every validator archetype", small_font, (214, 236, 247)))

    for index, (label, visible, hidden, color) in enumerate(STATIONS):
        row = index // 4
        col = index % 4
        x = -2.85 + col * 1.9
        y = -0.58 + row * 1.55
        boxes.append(Box((x, y, 0.08), (1.44, 0.92, 0.16), (28, 40, 51)))
        boxes.append(Box((x - 0.33, y - 0.12, 0.63), (0.18, 0.18, 1.12), color))
        boxes.append(Box((x + 0.33, y - 0.12, 0.79), (0.18, 0.18, 1.45), (242, 184, 75)))
        boxes.append(Box((x, y + 0.34, 0.29), (0.96, 0.09, 0.22), (225, 77, 68)))
        add_character(boxes, x - 0.58, y + 0.56, color)
        add_character(boxes, x + 0.58, y + 0.56, (95, 226, 143))

        sx, sy = project((x, y - 0.84, 1.55))
        label_positions.append(((sx, sy), label, label_font, (250, 255, 255)))
        vx, vy = project((x, y - 0.84, 1.31))
        label_positions.append(((vx, vy), visible, tiny_font, (189, 234, 255)))
        hx, hy = project((x, y - 0.84, 1.13))
        label_positions.append(((hx, hy), hidden, tiny_font, (255, 220, 168)))
        mx, my = project((x - 0.58, y + 0.83, 1.30))
        label_positions.append(((mx, my), "mentor", tiny_font, (230, 244, 255)))
        sx2, sy2 = project((x + 0.58, y + 0.83, 1.30))
        label_positions.append(((sx2, sy2), "survivor", tiny_font, (230, 255, 235)))

    boxes.sort(key=lambda b: b.center[0] + b.center[1] + b.center[2])
    for box in boxes:
        draw_box(draw, box)

    legend_x, legend_y = 98, HEIGHT - 112
    draw.rounded_rectangle((legend_x, legend_y, legend_x + 610, legend_y + 72), radius=10, fill=(13, 24, 32), outline=(71, 98, 115))
    draw.rectangle((legend_x + 20, legend_y + 22, legend_x + 44, legend_y + 46), fill=(65, 189, 244))
    draw.rectangle((legend_x + 54, legend_y + 22, legend_x + 78, legend_y + 46), fill=(242, 184, 75))
    draw.rectangle((legend_x + 88, legend_y + 22, legend_x + 112, legend_y + 46), fill=(225, 77, 68))
    draw_text_center(draw, (legend_x + 350, legend_y + 36), "Legend: visible pylon, hidden pylon, mistake marker", small_font, (217, 237, 247))

    for xy, text, font, fill in label_positions:
        draw_text_center(draw, xy, text, font, fill)

    image.save(OUTPUT_PATH)
    return OUTPUT_PATH


if __name__ == "__main__":
    print(render())
