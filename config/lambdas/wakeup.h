#include "esphome.h"

void wakeup(AddressableLight &it, int duration) {

  static int state = 0;
  auto call = id(it).make_call();

  call.set_transition_length(duration / 10);

  if (state == 0) {
    call.set_rgb(0,0,1.0);
    call.set_brightness(0.15);
  } else if (state == 1) {
    call.set_rgb(0.1,0,1.0);
    call.set_brightness(0.40);
  } else if (state == 2) {
    call.set_rgb(0.2,0.1,1.0);
    call.set_brightness(0.60);
  } else if (state == 3) {
    call.set_rgb(0.3,0.1,1.0);
    call.set_brightness(0.80);
  } else if (state == 4) {
    call.set_rgb(0.4,0.2,1.0);
    call.set_brightness(1.0);
  } else if (state == 5) {
    call.set_rgb(0.5,0.4,1.0);
  } else if (state == 6) {
    call.set_rgb(0.6,0.5,1.0);
  } else if (state == 7) {
    call.set_rgb(0.7,0.5,1.0);
  } else if (state == 8) {
    call.set_rgb(0.8,0.6,1.0);
  } else if (state == 9) {
    call.set_rgb(0.9,0.7,1.0);
  } else if (state == 10) {
    call.set_rgb(1.0,1.0,1.0);
  }
  if (state == 11){
    id(luz_100).turn_on();
    call.set_effect("None");
    call.set_state(false);
    call.perform();
    state = -1;
  }

  call.perform();
  state += 1;
}