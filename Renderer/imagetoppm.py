import argparse
from PIL import Image

def jpg_to_ppm_p3(input_path, output_path):
    img = Image.open(input_path).convert("RGB")
    width, height = img.size
    pixels = list(img.getdata())

    with open(output_path, "w") as f:
        f.write(f"P3\n{width} {height}\n255\n")
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[y * width + x]
                f.write(f"{r} {g} {b} ")
            f.write("\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert JPG to PPM (P3 format)")
    parser.add_argument("input", help="Input JPG file path")
    parser.add_argument("output", help="Output PPM file path")
    args = parser.parse_args()

    jpg_to_ppm_p3(args.input, args.output)
