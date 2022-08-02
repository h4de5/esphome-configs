#include "esphome.h"
#undef delay

class StarInfo {
public:
	StarInfo() :
		ledNumber(-1), brightness(0), brightnessInc(0), delay(0), delayWait(0), color(0)
		{ } ;
	StarInfo(int ledNumber, int brightness, int brightnessInc, byte delay, byte delayWait, esphome::Color color) :
		ledNumber(ledNumber), brightness(brightness), brightnessInc(brightnessInc), delay(delay), delayWait(delayWait), color(color)
		{ } ;

	int ledNumber;
	int brightness; // -255 ... 255
	int brightnessInc;
	byte delay;
	byte delayWait;
	esphome::Color color;
};

const int STAR_K_MIN=1800;
const int STAR_K_MAX=7500;

static StarInfo *stars=NULL; // 1st is fastest, last is slowest star
static int numStars=-1;
// led strip
//const float starPercentage=0.10;
// outdoor leds
const float starPercentage=0.15; // determines the number of stars
const float fastestStar=0.4;   // inc every 1 tick
const float slowestStar=10;  // inc every 10 ticks
const int cometLength=10;
const int cometDirection=-1; // from highest number to lowest
const int cometSleepTicks=1500; // how many ticks to wait to start new comet
const int cometBrightnessInc=15; // comet runs from -255 ... 255 => higher value means comet is shorter

static int cometLedNumber=-1;
static int cometBrightness=-255;

// #define TAG "starsLambda"

void starsLambda(AddressableLight &it) {
	// it.size() - Number of LEDs
	// it[num] - Access the LED at index num.
	// Set the LED at num to the given r, g, b values
	// it[num] = esphome::Color(r, g, b);
	// Get the color at index num (esphome::Color instance)
	// it[num].get();
	// n stars
	if(numStars == -1)  {
		numStars=it.size() * starPercentage;
		stars = new StarInfo[numStars];
	}

	it.range(0, it.size()) = esphome::Color::BLACK;

	for(int i=0; i < numStars; i++) {
		// init star:
		if(stars[i].ledNumber == -1) {
			// ESP_LOGD(TAG, "init star %d", i);
			stars[i].ledNumber = random_float() * it.size(); // FIXME: check if this led already used..
			int k=(random_float() * (STAR_K_MAX-STAR_K_MIN) )+STAR_K_MIN; // 2200K - 7200K
			stars[i].color = colorTemperatureToRGB(k);
			float r=(fastestStar*(numStars - i) / numStars) + (slowestStar*(pow(i,2)) / pow(numStars,2));
			stars[i].delay = floor(r);
			if(r < 0.5) {
				stars[i].brightnessInc=2;
			} else {
				stars[i].brightnessInc=1;
			}
			// stars[i].delayWait=stars[i].delay + random_float() * 255; // add some randomness to initial Delay
			stars[i].delayWait=0;
			stars[i].brightness=-255;
			// ESP_LOGD(TAG, "init star [%d] =>%d - r:%f - k:%d=%02x %02x %02x initDelay=%d", i, stars[i].ledNumber, r, k,
				// stars[i].color.red, stars[i].color.green, stars[i].color.blue, stars[i].delayWait);
		} else {
			if(stars[i].delayWait == 0) {
				stars[i].delayWait = stars[i].delay;
				stars[i].brightness+=stars[i].brightnessInc;
				if(stars[i].brightness > 255) {
				// if(stars[i].brightness > 350) {
					stars[i].ledNumber = -1;
				}
			} else {
				stars[i].delayWait--;
			}
		}

		if(stars[i].ledNumber >= 0) {
			int brightness=255 - abs(stars[i].brightness);
			esphome::Color color=stars[i].color * brightness;
			// gamma correction => check rgb_order: GRB if stars are not red / orange / yellow / white / blue-ish
			//color.red=color.red*pow(color.red/255.0,1);
			//color.green=color.green*pow(color.green/255.0,1.2);
			//color.blue=color.blue*pow(color.blue/255.0,1.1);
			//if(i==0)
     		//	 ESP_LOGD(TAG, "[1]=>%d b:%d %02x %02x %02x",stars[i].ledNumber,stars[i].brightness, color.red, color.green, color.blue);
			it[stars[i].ledNumber]=color;
			// it[i]=color;
		} else {
			// ESP_LOGD(TAG, "star %d not inited", i);
		}

	}
	// comet
	if(cometLedNumber >= 0 && cometBrightness < 255) {
		for(int i=0; i < cometLength; i++) {
			int brightness=255 - abs(cometBrightness);
			esphome::Color color=esphome::Color::WHITE * (brightness * i/cometLength);
			int ledNumber=cometLedNumber+(i*cometDirection);
			if(i == cometLength-1) {
				color+=80;
			}
			if(ledNumber < it.size() && ledNumber >= 0) {
				esphome::Color curr=it[ledNumber].get();
				it[ledNumber]=color+curr;
			}
		}
		cometLedNumber+=cometDirection;
        cometBrightness+=cometBrightnessInc;
	} else if(cometLedNumber < 0 || cometBrightness > (255 + cometSleepTicks) ) {
		if(cometDirection > 0) {
			cometLedNumber = random_float() * (it.size() - (512/cometBrightnessInc)); // comet length = 512/cometBrightnessInc
		} else {
			cometLedNumber = random_float() * (it.size() - (512/cometBrightnessInc)) + (512/cometBrightnessInc);
		}
        cometBrightness=-255;
	} else {
		cometBrightness++;
	}
}