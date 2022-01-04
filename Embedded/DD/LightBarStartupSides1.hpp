#ifndef LIGHT_BAR_STARTUP_SIDES_1_H 
#define LIGHT_BAR_STARTUP_SIDES_1_H 

#include "arduino.h"
#include <Adafruit_NeoPixel.h>
#include <EasyTimer.h>

#include "LightBar.hpp"
#include "CAN/CAN1.hpp"
#include "CAN/CAN2.hpp"

/*
 * light bar for a blinking display */
class CLightBarStartupSides1 : public CLightBar {

    public:


    private:

        bool mCompleted = false;
        int mCurrentStep = 0;
         
        EasyTimer mBlinkTimer = EasyTimer(40);   ///< The rate at which the lights during upshift should blink

    protected:


    public:

        /** Constructor */
        CLightBarStartupSides1(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs);

        /** Destructor */
        virtual ~CLightBarStartupSides1() {};

        /** Copy constructor disabled */
        CLightBarStartupSides1(const CLightBarStartupSides1 &) = delete;
        /** Assignment operator disabled */
        void operator=(const CLightBarStartupSides1 &) = delete;

        virtual void Initialize() override;
        virtual void Update(unsigned long &elapased) override;

        bool IsCompleted() const {return mCompleted;}

};

/*
 * Constructor
 * \param lights The neopixel light object
 * \param first index The first index of the neopixels to use
 * \param numLEDs The number of LEDs to use
 */
CLightBarStartupSides1::CLightBarStartupSides1(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs) : 
            CLightBar(lights, firstIndex, numLEDs) {};



/**
 * Initialize the lights by clearing them.
 */
void CLightBarStartupSides1::Initialize(){
    CLightBar::Initialize();
}



/**
 * If the frame rate timer is all good, update the screen
 *
 * \param elapsed The time in milliseconds elapsed since last called
 */
void CLightBarStartupSides1::Update(unsigned long &elapsed){

    int led = GetLastLEDIndex() - (mCurrentStep / 255);
    int pwm = mCurrentStep % 255;

    // determine if we're done with all of the LEDs
    if (led < GetFirstLEDIndex()){
        mCompleted = true;
        return;
    }

    // write the led
    mLights.setPixelColor(led, pwm, pwm, pwm);

    mCurrentStep += 5;
    
    // sends the update
    CLightBar::Update(elapsed);

}





#endif