# Sprites

This folder contains user-generated sprites for the XTeInk display.

## Creating Your Own Sprites

Follow the workflow in the main README under [Custom Sprites](README.md#custom-sprites):

1. **Generate** your character with Nano Banana Pro (or any image generator)
2. **Remove background** with Canva Pro or similar
3. **Convert to pixel art** with Piskel (piskelapp.com)
4. **Export** as 200×200 PNG with transparent background

## Required Sprite Names

Save your sprites as:
- `sleeping.png`
- `idle.png`
- `alert.png`
- `thinking.png`
- `talking.png`
- `working.png`
- `excited.png`
- `error.png`

## Running the Converter

After adding your PNGs:
```bash
python3 convert_sprites.py
```

This generates the C header files in `firmware/include/` needed by the firmware.
