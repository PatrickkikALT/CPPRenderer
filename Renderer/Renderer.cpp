#include <iostream>
#include <fstream>
#include <string>


void gradient() {
  int width = 100;
  int height = 100;

  std::ofstream image("gradient.ppm");
  image << "P3\n" << width << " " << height << "\n255\n";

  for (int j = 0; j < height; ++j) {
	for (int i = 0; i < width; ++i) {
	  //creates a gradient
	  int r = (i * 255) / (width - 1);
	  int g = (j * 255) / (height - 1);
	  int b = ((i + j) * 255) / (width + height - 2);
	  image << r << " " << g << " " << b << " ";
	}
	image << "\n";
  }
  image.close();
}

void hsv_to_rgb(float h, float s, float v, int& r, int& g, int& b) {
  // calculate chroma, the difference between the max and min rgb values
  float c = v * s;

  // find the second largest component of the color based on hue
  float x = c * (1 - std::fabs(fmod(h / 60.0, 2) - 1));

  float m = v - c;

  float r_, g_, b_;

  // determine which section of the color wheel h is in
  if (h < 60) { r_ = c; g_ = x; b_ = 0; }
  else if (h < 120) { r_ = x; g_ = c; b_ = 0; }
  else if (h < 180) { r_ = 0; g_ = c; b_ = x; }
  else if (h < 240) { r_ = 0; g_ = x; b_ = c; }
  else if (h < 300) { r_ = x; g_ = 0; b_ = c; }
  else { r_ = c; g_ = 0; b_ = x; }

  // add m to match brightness and scale to 0–255 range
  r = static_cast<int>((r_ + m) * 255);
  g = static_cast<int>((g_ + m) * 255);
  b = static_cast<int>((b_ + m) * 255);
}

void rainbow() {
  int width = 100;
  int height = 100;

  std::ofstream image("rainbow.ppm");
  image << "P3\n" << width << " " << height << "\n255\n";

  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i) {
      float normalized = static_cast<float>(i + j) / (width + height - 2);
      float h = normalized * 360.0f;

      int r, g, b;
      hsv_to_rgb(h, 1.0f, 1.0f, r, g, b);
      image << r << " " << g << " " << b << " ";
    }
    image << "\n";
  }

  image.close();
}



int main() {
  gradient();
  rainbow();
  return 0;
}