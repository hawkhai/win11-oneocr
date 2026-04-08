"""
visualize.py - Visualize OneOCR results on images.

Supports both 4-point legacy bbox and 8-point (4-corner) bbox formats.
Shows confidence scores, image angle, and handwritten/printed style labels.

Usage:
  python visualize.py <image_path> <json_path> [output_path]
  python visualize.py                          # defaults to test.png + oneocr_test_result.json
"""

import json
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def load_result(json_path):
    with open(json_path, "r", encoding="utf-8") as f:
        return json.load(f)


def get_font(size=14):
    font_candidates = [
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simsun.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/arial.ttf",
    ]
    for fp in font_candidates:
        if Path(fp).exists():
            try:
                return ImageFont.truetype(fp, size)
            except Exception:
                continue
    return ImageFont.load_default()


def parse_bbox(bbox_data):
    """Parse bbox from array or legacy dict format.

    Current format: [x1, y1, x2, y2, x3, y3, x4, y4]  (8 floats)
    Legacy 4-float: [x1, y1, x2, y2]                    (baseline only)
    Legacy dict:    {x1, y1, x2, y2, x3, y3, x4, y4}

    Returns 4 corner points: [(x1,y1), (x2,y2), (x3,y3), (x4,y4)]
    representing top-left, top-right, bottom-right, bottom-left.
    """
    if isinstance(bbox_data, list):
        if len(bbox_data) == 8:
            # Current format: [x1, y1, x2, y2, x3, y3, x4, y4]
            return [
                (bbox_data[0], bbox_data[1]),  # top-left
                (bbox_data[2], bbox_data[3]),  # top-right
                (bbox_data[4], bbox_data[5]),  # bottom-right
                (bbox_data[6], bbox_data[7]),  # bottom-left
            ]
        elif len(bbox_data) >= 4:
            # Legacy 4-float list: [x1, y1, x2, y2] (baseline only)
            x1, y1, x2, y2_baseline = bbox_data[0], bbox_data[1], bbox_data[2], bbox_data[3]
            estimated_h = abs(x2 - x1) * 0.5 if abs(x2 - x1) > 0 else 20
            top = min(y1, y2_baseline)
            bottom = top + estimated_h
            return [
                (x1, top),
                (x2, top),
                (x2, bottom),
                (x1, bottom),
            ]
    elif isinstance(bbox_data, dict):
        # Legacy dict format: {x1, y1, x2, y2, x3, y3, x4, y4}
        return [
            (bbox_data["x1"], bbox_data["y1"]),
            (bbox_data["x2"], bbox_data["y2"]),
            (bbox_data["x3"], bbox_data["y3"]),
            (bbox_data["x4"], bbox_data["y4"]),
        ]
    return None


def draw_polygon(draw, points, color, width=2):
    """Draw a polygon from 4 corner points."""
    for i in range(len(points)):
        p1 = points[i]
        p2 = points[(i + 1) % len(points)]
        draw.line([p1, p2], fill=color, width=width)


def confidence_color(confidence):
    """Map confidence (0.0-1.0) to color: red(low) -> yellow(mid) -> green(high)."""
    if confidence is None:
        return (128, 128, 128)
    c = max(0.0, min(1.0, confidence))
    if c < 0.5:
        r = 255
        g = int(255 * c * 2)
    else:
        r = int(255 * (1.0 - c) * 2)
        g = 255
    return (r, g, 0)


def visualize(image_path, json_path, output_path=None):
    data = load_result(json_path)
    img = Image.open(image_path).convert("RGB")
    draw = ImageDraw.Draw(img)
    font = get_font(16)
    font_small = get_font(12)

    line_color = (255, 0, 0)        # red for lines
    word_color = (0, 128, 255)      # blue for words
    text_color = (0, 180, 0)        # green for text labels
    info_color = (200, 200, 0)      # yellow for metadata
    handwritten_color = (255, 128, 0)  # orange for handwritten

    # Draw image-level info
    y_offset = 5
    if "image_angle" in data and data["image_angle"] is not None:
        angle_text = f"Image angle: {data['image_angle']:.4f}"
        draw.text((5, y_offset), angle_text, fill=info_color, font=font_small)
        y_offset += 18

    line_count = data.get("line_count", len(data.get("lines", [])))
    count_text = f"Lines: {line_count}"
    draw.text((5, y_offset), count_text, fill=info_color, font=font_small)

    for line in data.get("lines", []):
        # Parse line bbox
        bbox_data = line.get("bounding_box", line.get("bbox"))
        if not bbox_data:
            continue

        points = parse_bbox(bbox_data)
        if not points:
            continue

        # Determine line color based on style
        current_line_color = line_color
        style = line.get("style")
        if style:
            if style.get("type") == "handwritten":
                current_line_color = handwritten_color

        # Draw line polygon
        draw_polygon(draw, points, current_line_color, width=2)

        # Text label above bbox
        min_x = min(p[0] for p in points)
        min_y = min(p[1] for p in points)

        label_parts = [line.get("text", "")]
        if style:
            style_tag = "H" if style.get("type") == "handwritten" else "P"
            # confidence: handwritten confidence (0.0 = printed, 1.0 = handwritten)
            style_conf = style.get("confidence")
            if style_conf is not None:
                label_parts.append(f"[{style_tag}:{style_conf:.2f}]")
            else:
                label_parts.append(f"[{style_tag}]")

        label = " ".join(label_parts)
        ty = max(min_y - 20, 0)
        draw.text((min_x, ty), label, fill=text_color, font=font)

        # Draw word-level bboxes
        for word in line.get("words", []):
            word_bbox = word.get("bounding_box", word.get("bbox"))
            if not word_bbox:
                continue

            word_points = parse_bbox(word_bbox)
            if not word_points:
                continue

            # Color based on confidence
            conf = word.get("confidence")
            if conf is not None:
                w_color = confidence_color(conf)
            else:
                w_color = word_color

            draw_polygon(draw, word_points, w_color, width=1)

            # Show confidence below word
            if conf is not None:
                wx = min(p[0] for p in word_points)
                wy = max(p[1] for p in word_points) + 2
                draw.text((wx, wy), f"{conf:.2f}", fill=w_color, font=font_small)

    if output_path is None:
        stem = Path(image_path).stem
        output_path = Path(image_path).parent / f"{stem}_visualized.png"

    img.save(output_path)
    print(f"Saved visualization to: {output_path}")
    img.show()


if __name__ == "__main__":
    base_dir = Path(__file__).parent
    image_path = base_dir / "test.png"
    json_path = base_dir / "oneocr_test_result.json"

    if len(sys.argv) >= 3:
        image_path = sys.argv[1]
        json_path = sys.argv[2]
    if len(sys.argv) >= 4:
        output_path = sys.argv[3]
    else:
        output_path = None

    visualize(str(image_path), str(json_path), output_path)
