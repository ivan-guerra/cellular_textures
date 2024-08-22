
#include <Magick++.h>
#include <Magick++/STL.h>

#include <format>
#include <vector>

int main() {
  std::vector<Magick::Image> images;

  // Read images from files
  for (size_t i = 1; i <= 100; ++i) {
    images.emplace_back(std::format("image{}.png", i));
    // Set delay between frames (in 1/100th of a second)
    images.back().animationDelay(5);
  }

  Magick::writeImages(images.begin(), images.end(), "output.gif");

  return 0;
}
