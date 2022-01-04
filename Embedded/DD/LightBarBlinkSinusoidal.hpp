#ifndef LIGHT_BAR_BLINK_SINUSOIDAL_H 
#define LIGHT_BAR_BLINK_SINUSOIDAL_H 

#include "arduino.h"
#include <Adafruit_NeoPixel.h>
#include <EasyTimer.h>

#include "LightBar.hpp"
#include "LightBarBlink.hpp"
#include "CAN/CAN1.hpp"
#include "CAN/CAN2.hpp"

/*
 * light bar for a blinking display */
class CLightBarBlinkSinusoidal : public CLightBarBlink {

    private:

    protected:

        unsigned long mPeriod = 5000; ///< The period of a sinusoidal cycle in milliseconds
        unsigned long mCurrentTime = 0;   ///< Used to keep track of the current time in milliseconds since starting

    public:

        /** Constructor */
        CLightBarBlinkSinusoidal(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs);

        /** Destructor */
        virtual ~CLightBarBlinkSinusoidal() {};

        /** Copy constructor disabled */
        CLightBarBlinkSinusoidal(const CLightBarBlinkSinusoidal &) = delete;
        /** Assignment operator disabled */
        void operator=(const CLightBarBlinkSinusoidal &) = delete;

        virtual void Initialize() override;
        virtual void Update(unsigned long &elapased) override;

};

/*
 * Constructor
 * \param lights The neopixel light object
 * \param first index The first index of the neopixels to use
 * \param numLEDs The number of LEDs to use
 */
CLightBarBlinkSinusoidal::CLightBarBlinkSinusoidal(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs) : 
            CLightBarBlink(lights, firstIndex, numLEDs) {};



/**
 * Initialize the lights by clearing them.
 */
void CLightBarBlinkSinusoidal::Initialize(){
    CLightBarBlink::Initialize();
    mCurrentTime = 0;
}



/**
 * If the frame rate timer is all good, update the screen
 *
 * \param elapsed The time in milliseconds elapsed since last called
 */
void CLightBarBlinkSinusoidal::Update(unsigned long &elapsed){


    // if there is no signal set OR the signal's threshold has been reached, blink
    if (!mSignal || mSignal->value() >= mMin){

        // if we just entered into blinking mode, reset the time
        if (mState != Blinking){
            mCurrentTime = 0;
        }

        mState = Blinking;

        
    } else {
        mState = Off;
    }
   

    switch (mState){
        case Off:
            Clear();
            break;
        case Blinking:

            // update elapsed time
            mCurrentTime += elapsed;

            float fullCycle = 2 * PI;
            float percentOn = (1 - cos(((float)(mCurrentTime % mPeriod) / (float)mPeriod) * fullCycle)) / 2;  // range [0, 1]

            for (int led = GetFirstLEDIndex(); led <= GetLastLEDIndex(); led++){
                mLights.setPixelColor(led, percentOn*mR, percentOn*mG, percentOn*mB);
            }

            break;
    }


    // sends the update
    CLightBar::Update(elapsed);

}





#endif