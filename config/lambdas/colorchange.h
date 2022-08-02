#include "esphome.h"

// #define TAG "colorchange"

void colorchange(AddressableLight &it) {

  static esphome::Color *xLedTargetColor=NULL;
  static esphome::Color *xLedCurrColor=NULL;

  if(xLedTargetColor==NULL) {
    xLedTargetColor=new esphome::Color[it.size()];
    xLedCurrColor=new esphome::Color[it.size()];
    for(int i=0; i < it.size(); i++) {
      xLedCurrColor[i]=it[i].get();
      xLedTargetColor[i]=esphome::Color::random_color();
      // esphome::Color target= xLedTargetColor[i];
      // ESP_LOGD(TAG, "init [%d]=> %02x %02x %02x", i, target.red, target.green, target.blue);
    }
  }
  for (int i = 0; i <  it.size(); i++) {
    esphome::Color &c=xLedCurrColor[i];
    esphome::Color org=c;
    esphome::Color target= xLedTargetColor[i];
    if(c.red == target.red && c.green == target.green && c.blue == target.blue) {
      xLedTargetColor[i]=esphome::Color::random_color();
      // target= xLedTargetColor[i];
      // ESP_LOGD(TAG, "init [%d]=> %02x %02x %02x", i, target.red, target.green, target.blue);
    }
    if(c.red < target.red) {
      c.red++;
    } else if(c.red > target.red) {
      c.red--;
    }
    if(c.green < target.green) {
      c.green++;
    } else if(c.green > target.green) {
      c.green--;
    }
    if(c.blue < target.blue) {
      c.blue++;
    } else if(c.blue > target.blue) {
      c.blue--;
    }
    // ESP_LOGD(TAG, "change [%d]=> %02x %02x %02x  => %02x %02x %02x", i, c.red, c.green, c.blue, target.red, target.green, target.blue);
  //            it[i].set_red(c.red+2);
  //            it[i].set_green(c.green+2);
  //            it[i].set_blue(c.blue);
    // esphome::Color x(c.red, c.green, c.blue);
    // esphome::Color x(10, 30, 60);
    it[i] = c;
    // if(i==0) ESP_LOGD(TAG, "change [%d]=> %02x %02x %02x  => %02x %02x %02x", i, org.red, org.green, org.blue,  it[i].get_red(), it[i].get_green(), it[i].get_blue());
  }
}