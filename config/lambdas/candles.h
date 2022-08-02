#include "esphome.h"

// #define TAG "candles"

void candles(AddressableLight &it, bool initial_run) {

  const int STAR_K_NORM_MIN=1900;
  const int STAR_K_NORM_MAX=3000;
  const int STAR_K_DISTURBED_MIN=1700;
  const int STAR_K_DISTURBED_MAX=7500;
  static int *target=NULL;
  static int *curr=NULL;
  static int *inc=NULL;
  static int *state=NULL;
  static int *startState=NULL;
  static int *stateLen=NULL;
  static esphome::Color *targetColor=NULL;
  static esphome::Color *currColor=NULL;

  // ESP_LOGD(TAG, "candle");
  if(state==NULL) {
    // ESP_LOGD(TAG, "candle mallocs");
    target=new int[it.size()];
    curr=new int[it.size()];
    inc=new int[it.size()];
    state=new int[it.size()];
    startState=new int[it.size()];
    stateLen=new int[it.size()];
    targetColor=new esphome::Color[it.size()];
    currColor=new esphome::Color[it.size()];
    // ESP_LOGD(TAG, "candle mallocs done");
  }
  if(initial_run) {
    // ESP_LOGD(TAG, "candle initial_run");
    for(int i=0; i < it.size(); i++) {
      // ESP_LOGD(TAG, "candle initial_run[%d]", i); delay(10);
      curr[i]=0; target[i]=0; // curr==target => init
      inc[i]=0; state[i]=0; startState[i]=0; stateLen[i]=0;
      currColor[i] = it[i].get();
      targetColor[i] = it[i].get();
    }
    // ESP_LOGD(TAG, "candle initial_run done"); delay(10);
  }
  for (int i = 0; i < it.size() ; i++) {
    // ESP_LOGD(TAG, "candle [%d]", i); delay(10);
    if(state[i] == 0 || state[i] > 500) {
      state[i] = random(500); // random delay till first flicker
      target[i] = curr[i]; // init new target
    }
    // set new target if target reached
    if(state[i] == 450 || (curr[i] == target[i] && state[i] > 450)) { // 10% is disturbed candle
      target[i]=random(50, 255);
      inc[i]=random(50)+1;
      currColor[i]=targetColor[i];
      // targetColor[i]=current_color+esphome::Color::random_color()*random(100);
      int k=(random_float() * (STAR_K_DISTURBED_MAX-STAR_K_DISTURBED_MIN) )+STAR_K_DISTURBED_MIN; // 1800K - 7500K
      targetColor[i] = colorTemperatureToRGB(k);
      stateLen[i]=abs(target[i]-curr[i]) / inc[i];
      startState[i]=state[i];
    } else if(curr[i] == target[i]) { // slow changing candle
      target[i]=random(80, 160);
      inc[i]=random(10)+1;
      currColor[i]=targetColor[i];
      // targetColor[i]=current_color+esphome::Color::random_color()*random(40);
      int k=(random_float() * (STAR_K_NORM_MAX-STAR_K_NORM_MIN) )+STAR_K_NORM_MIN; // 2200K - 6000K
      targetColor[i] = colorTemperatureToRGB(k);
      stateLen[i]=abs(target[i]-curr[i]) / inc[i];
      startState[i]=state[i];
    }
    // ESP_LOGD(TAG, "candle [%d]2", i); delay(10);
    if(curr[i] > target[i]) {
      curr[i]-=min(inc[i], curr[i]-target[i]);
    } else {
      curr[i]+=min(inc[i], target[i]-curr[i]);
    }
    // ESP_LOGD(TAG, "candle [%d]3", i); delay(10);
    int factor=255;
    if(stateLen[i] > 0) factor = 255 * (state[i] - startState[i]) / (stateLen[i]);
    esphome::Color c=currColor[i]*(factor*(curr[i]+1)/256) + targetColor[i]*((255-factor)*(curr[i]+1)/256);
    it[i]=c;
    // it[i]=it[i]*curr[i];
    state[i]++;
    // if(i==0) ESP_LOGD(TAG, "candle [0]=> t:%d c:%d inc:%d state:%d startState:%d stateLen:%d factor:%d cc:%02x tc:%02x out=%02x",
    //  target[i], curr[i], inc[i], state[i], startState[i], stateLen[i],
    //  factor, currColor[i].red, targetColor[i].red, c.red);
  }
}