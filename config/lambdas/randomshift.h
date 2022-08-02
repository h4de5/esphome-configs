#include "esphome.h"

// #define TAG "randomshift"

void randomshift(AddressableLight &it) {

  // it.size() - Number of LEDs
  // it[num] - Access the LED at index num.
  // Set the LED at num to the given r, g, b values
  // it[num] = esphome::Color(r, g, b);
  // Get the color at index num (esphome::Color instance)
  // it[num].get();

  // Example: Simple color wipe
  // every 3rd led ...
  if(random_float() > 0.7)
    it[0] = esphome::Color::random_color();
  else
      it[0] = Color::BLACK;
  for (int i = it.size() -1 ; i > 0; i--) {
    it[i] = it[i - 1].get();
  }

  // Bonus: use .range() and .all() to set many LEDs without having to write a loop.
  //it.range(0, 50) = Color::BLACK;
  //it.all().fade_to_black(10);
}