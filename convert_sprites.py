#!/usr/bin/env python3
"""
Convert PNG sprites to C header files for ESP32 firmware.
Resizes to target size and converts to 1-bit black/white bitmaps.
"""

from PIL import Image
import os

# Configuration
SPRITE_DIR = os.path.expanduser("~/clawdbot-tamagotchi/sprites")
OUTPUT_DIR = os.path.expanduser("~/clawdbot-tamagotchi/xteink-firmware/include")
TARGET_HEIGHT = 200  # pixels - smaller to ensure it fits on screen

SPRITES = [
    "alert",
    "error",
    "excited",
    "idle",
    "sleeping",
    "talking",
    "thinking",
    "working"
]

def convert_to_bitmap_array(img, name):
    """Convert PIL image to C array string."""
    width, height = img.size

    # Handle RGBA images - composite onto white background
    if img.mode == 'RGBA':
        background = Image.new('RGB', img.size, (255, 255, 255))
        background.paste(img, mask=img.split()[3])  # Use alpha as mask
        img = background

    # Convert to grayscale
    img_gray = img.convert('L')
    pixels = list(img_gray.getdata())

    # GxEPD2 drawBitmap: bit=1 draws the color, bit=0 is transparent
    # We want black/dark pixels to show, so dark=1, light=0
    # Pixels are packed MSB first, row by row
    # Threshold: pixels darker than 128 are considered "black"
    bytes_per_row = (width + 7) // 8
    total_bytes = bytes_per_row * height

    bitmap_bytes = []

    for y in range(height):
        for byte_idx in range(bytes_per_row):
            byte_val = 0
            for bit in range(8):
                x = byte_idx * 8 + bit
                if x < width:
                    pixel_idx = y * width + x
                    # bit=1 where pixel is dark (< 128), bit=0 where pixel is light
                    if pixels[pixel_idx] < 128:
                        byte_val |= (0x80 >> bit)
            bitmap_bytes.append(byte_val)

    # Format as C array
    lines = []
    lines.append(f"// Auto-generated bitmap for {name}")
    lines.append(f"// Size: {width}x{height} pixels, {total_bytes} bytes")
    lines.append(f"#ifndef SPRITE_{name.upper()}_H")
    lines.append(f"#define SPRITE_{name.upper()}_H")
    lines.append("")
    lines.append(f"#define SPRITE_{name.upper()}_WIDTH {width}")
    lines.append(f"#define SPRITE_{name.upper()}_HEIGHT {height}")
    lines.append("")
    lines.append(f"const unsigned char sprite_{name}[] PROGMEM = {{")

    # Output bytes in rows of 16
    for i in range(0, len(bitmap_bytes), 16):
        chunk = bitmap_bytes[i:i+16]
        hex_str = ", ".join(f"0x{b:02X}" for b in chunk)
        if i + 16 < len(bitmap_bytes):
            hex_str += ","
        lines.append(f"    {hex_str}")

    lines.append("};")
    lines.append("")
    lines.append(f"#endif // SPRITE_{name.upper()}_H")

    return "\n".join(lines), width, height


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    print("Converting sprites to C bitmaps...")
    print(f"Target height: {TARGET_HEIGHT}px")
    print("-" * 50)

    all_sprites_info = []

    for name in SPRITES:
        input_path = os.path.join(SPRITE_DIR, f"{name}.png")
        output_path = os.path.join(OUTPUT_DIR, f"sprite_{name}.h")

        if not os.path.exists(input_path):
            print(f"WARNING: {input_path} not found, skipping")
            continue

        # Load image
        img = Image.open(input_path)
        orig_width, orig_height = img.size

        # Calculate new size maintaining aspect ratio
        scale = TARGET_HEIGHT / orig_height
        new_width = int(orig_width * scale)
        new_height = TARGET_HEIGHT

        # Resize with high quality
        img_resized = img.resize((new_width, new_height), Image.Resampling.LANCZOS)

        # Convert to bitmap array
        c_code, final_width, final_height = convert_to_bitmap_array(img_resized, name)

        # Write header file
        with open(output_path, 'w') as f:
            f.write(c_code)

        bytes_size = ((final_width + 7) // 8) * final_height
        print(f"  {name}: {orig_width}x{orig_height} -> {final_width}x{final_height} ({bytes_size} bytes)")
        all_sprites_info.append((name, final_width, final_height))

    # Create master include file
    master_path = os.path.join(OUTPUT_DIR, "sprites.h")
    with open(master_path, 'w') as f:
        f.write("// Auto-generated sprite includes\n")
        f.write("#ifndef SPRITES_H\n")
        f.write("#define SPRITES_H\n\n")
        for name in SPRITES:
            f.write(f'#include "sprite_{name}.h"\n')
        f.write("\n#endif // SPRITES_H\n")

    print("-" * 50)
    print(f"Generated {len(all_sprites_info)} sprite headers in {OUTPUT_DIR}")
    print(f"Master include: {master_path}")


if __name__ == "__main__":
    main()
