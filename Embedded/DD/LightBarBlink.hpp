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
         
        StateSignal &mSignal;   ///< The RPM signal
        int mMin;   ///< The minimum bound for the bar to start flashing

        BarStates mState = Off;   ///< The current state of LED light output

        EasyTimer mBlinkTimer = EasyTimer(40);   ///< The rate at which the lights during upshift should blink
        bool mLightsOn = false;  ///< used during the blinking

        int CalculatePWM(int &totalBarPWMs);

        ///< colors!
        int mR = 255;
        int mG = 255;
        int mB = 255;


    protected:


    public:

        /** Constructor */
        CLightBarBlink(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs, StateSignal &signal, int min);

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

};

/*
 * Constructor
 * \param lights The neopixel light object
 * \param first index The first index of the neopixels to use
 * \param numLEDs The number of LEDs to use
 * \param signal The signal to determine the blinking state
 */
CLightBarBlink::CLightBarBlink(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs, StateSignal &signal, int min) : 
            CLightBar(lights, firstIndex, numLEDs), mSignal(signal), mMin(min) {};



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


    // first, determine the state
    if (mSignal.value() >= mMin){
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