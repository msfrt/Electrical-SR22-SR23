#ifndef LIGHT_BAR_BLINK_H 
#define LIGHT_BAR_BLINK_H 

#include "arduino.h"
#include <Adafruit_NeoPixel.h>
#include <EasyTimer.h>

#include "LightBar.hpp"
#include "CAN/CAN1.hpp"
#include "CAN/CAN2.hpp"

/*
 * light bar for a blinking display */
class CLightBarBlink : public CLightBar {

    public:

        enum BarStates {Off, Blinking};  // The possible states of the bar

    private:
         
        EasyTimer mBlinkTimer = EasyTimer(40);   ///< The rate at which the lights during upshift should blink
        bool mLightsOn = false;  ///< used during the blinking


    protected:

        BarStates mState = Off;   ///< The current state of LED light output

        StateSignal *mSignal = nullptr;   ///< The signal (if applicable)
        int mMin = 0;   ///< The minimum bound for the bar to start flashing

        ///< colors!
        int mR = 255;
        int mG = 255;
        int mB = 255;


    public:

        /** Constructor */
        CLightBarBlink(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs);

        /** Destructor */
        virtual ~CLightBarBlink() {};

        /** Copy constructor disabled */
        CLightBarBlink(const CLightBarBlink &) = delete;
        /** Assignment operator disabled */
        void operator=(const CLightBarBlink &) = delete;

        virtual void Initialize() override;
        virtual void Update(unsigned long &elapased) override;

        /* SetColor
         * Sets the color of the blinking lights
         */
        void SetColor(int r, int g, int b) {mR=r; mG=g; mB=b;}

        /**
         * Updates the frequency of the blinking
         */
        void SetFrequency(int frequency) {mBlinkTimer.set_frequency(frequency);}

        /**
         * Attach (set) a signal so that the lights blink whenever the signal's value exceeds the minimum value
         * \param sig a pointer to the state signal to look at
         * \param mMinValue The minimum value to look at
         */
        void AttachSignal(StateSignal *sig, int min) {mSignal=sig; mMin=min;};

};

/*
 * Constructor
 * \param lights The neopixel light object
 * \param first index The first index of the neopixels to use
 * \param numLEDs The number of LEDs to use
 */
CLightBarBlink::CLightBarBlink(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs) : 
            CLightBar(lights, firstIndex, numLEDs) {};



/**
 * Initialize the lights by clearing them.
 */
void CLightBarBlink::Initialize(){
    CLightBar::Initialize();
}



/**
 * If the frame rate timer is all good, update the screen
 *
 * \param elapsed The time in milliseconds elapsed since last called
 */
void CLightBarBlink::Update(unsigned long &elapsed){


    // if there is no signal set OR the signal's threshold has been reached, blink
    if (!mSignal || mSignal->value() >= mMin){
        mState = Blinking;
    } else {
        mState = Off;
    }
   

    switch (mState){
        case Off:
            Clear();
            break;
        case Blinking:
            if (mBlinkTimer.isup()){
                if (mLightsOn){
                    mLightsOn = false;
                    Clear();
                } else {
                    mLightsOn = true;
                    for (int led = GetFirstLEDIndex(); led <= GetLastLEDIndex(); led++){
                        mLights.setPixelColor(led, mR, mG, mB);
                    }
                }
            }
            break;
    }

    // sends the update
    CLightBar::Update(elapsed);

}





#endif