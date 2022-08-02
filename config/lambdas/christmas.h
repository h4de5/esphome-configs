#include "esphome.h"

// #define TAG "christmas"

void christmas(AddressableLight &it) {

  static uint8_t *xLedInc=NULL;
  static int *xLedDim=NULL; // -255 ... 255
  static esphome::Color *xLedColor=NULL;
  // ..... creates compiler warning .....
  #define getColor() \
      esphome::Color color; \
      switch( (int) floor( random_uint32() / (double(UINT32_MAX)+1) * 3)) { \
        case 0: color=esphome::Color(255, 0, 18); break; \
        case 1: color=esphome::Color(0, 179, 44); break; \
        default: color=esphome::Color(255, 255, 255); break; \
      } \
      int dim=127+random_float()*128; \
      xLedColor[i]=color*dim;

  if(xLedInc==NULL) {
    xLedInc=new uint8_t[it.size()];
    xLedDim=new int[it.size()];
    xLedColor=new esphome::Color[it.size()];
    for(int i=0; i < it.size(); i++) {
      xLedInc[i]=random_float()*8;
      xLedDim[i]=random_float()*511-255;
      getColor();
      // ESP_LOGD(TAG, "init [%d]=> b:%d %02x %02x %02x   => %02x %02x %02x", i, dim, color.red, color.green, color.blue,  xLedColor[i].red,  xLedColor[i].green,  xLedColor[i].blue);
    }
  }
  for (int i = 0; i <  it.size(); i++) {
    int dim=255 - abs(xLedDim[i]);
    esphome::Color color=xLedColor[i] * dim;
    // if(i==0) ESP_LOGD(TAG, "[0]=> b:%d %02x %02x %02x",dim, color.red, color.green, color.blue);
    it[i] = color;
    xLedDim[i]+=xLedInc[i];
    if(xLedDim[i] > 255) {
      xLedDim[i]=-255;
      getColor();
    }
  }
}