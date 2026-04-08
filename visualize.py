import json
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def load_result(json_path):
    with open(json_path, "r", encoding="utf-8") as f:
        return json.load(f)


def draw_bbox(draw, bbox, color, img_size, text="", width=2):
    x1 = min(bbox[0], bbox[2])
    baseline = min(bbox[1], bbox[3])
    x2 = max(bbox[0], bbox[2])

    # The OCR engine bbox is (x1, y1, x2, y2) where y1≈y2 (top of text only,
    # no height info). Estimate height from per-character width, ratio 1.5
    # empirically matches test data vs. wechatocr ground truth.
    char_count = max(len(text), 1)
    char_width = (x2 - x1) / char_count
    estimated_h = char_width * 1.5
    y1 = baseline
    y2 = baseline + estimated_h

    # clamp to image bounds
    img_w, img_h = img_size
    x1 = max(0, min(x1, img_w - 1))
    y1 = max(0, min(y1, img_h - 1))
    x2 = max(0, min(x2, img_w - 1))
    y2 = max(0, min(y2, img_h - 1))
    draw.rectangle([x1, y1, x2, y2], outline=color, width=width)
    return x1, y1, x2, y2


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


def visualize(image_path, json_path, output_path=None):
    data = load_result(json_path)
    img = Image.open(image_path).convert("RGB")
    draw = ImageDraw.Draw(img)
    font = get_font(16)

    line_color = (255, 0, 0)       # red for lines
    word_color = (0, 128, 255)     # blue for words
    text_color = (0, 180, 0)       # green for text labels

    for line in data.get("lines", []):
        # Draw line-level bbox
        x1, y1, x2, y2 = draw_bbox(draw, line["bbox"], line_color, img.size, text=line["text"], width=2)
        # Draw text label above the line bbox
        label = line["text"]
        ty = max(y1 - 20, 0)
        draw.text((x1, ty), label, fill=text_color, font=font)

        # Draw word-level bboxes
        for word in line.get("words", []):
            draw_bbox(draw, word["bbox"], word_color, img.size, text=word["text"], width=1)

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

    visualize(str(image_path), str(json_path))
