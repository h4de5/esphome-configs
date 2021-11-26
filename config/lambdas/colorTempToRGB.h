/**
 *
 * https://gist.github.com/paulkaplan/5184275
 *
 */
#include "esphome.h"

int clamp(int x, int min, int max) {

  if (x < min) {
    return min;
  }
  if (x > max) {
    return max;
  }

  return x;
}

Color colorTemperatureToRGB(int kelvin) {

  int temp = kelvin / 100;

  int red, green, blue;

  if (temp <= 66) {

    red = 255;

    green = temp;
    green = 99.4708025861 * log(green) - 161.1195681661;

    if (temp <= 19) {

      blue = 0;

    } else {

      blue = temp - 10;
      blue = 138.5177312231 * log(blue) - 305.0447927307;

    }

  } else {

    red = temp - 60;
    red = 329.698727446 * pow(red, -0.1332047592);

    green = temp - 60;
    green = 288.1221695283 * pow(green, -0.0755148492);

    blue = 255;

  }

  return Color(
    clamp(red, 0, 255),
    clamp(green, 0, 255),
    clamp(blue, 0, 255)
  );

}