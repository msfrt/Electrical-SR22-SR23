#ifndef LIGHT_BAR_RPM_H 
#define LIGHT_BAR_RPM_H 

#include "arduino.h"
#include <Adafruit_NeoPixel.h>
#include <EasyTimer.h>

#include "LightBar.hpp"
#include "CAN/CAN1.hpp"
#include "CAN/CAN2.hpp"

/*
 * light bar for an RPM display */
class CLightBarRPM : public CLightBar {

    public:

        enum BarStates {Downshift, Off, Normal, Upshift};  // The possible states of the RPM bar

    private:
         
        StateSignal &mRPM;   ///< The RPM signal
        int mMin;   ///< The minimum bound for the RPM bar to start lighting up
        int mMax;   ///< The maximum bound for the rpm bar light up before signalling an upshift
        int mDownshift;   ///< Signal a downshift if below this RPM. Set to a negative number to disable.

        BarStates mState = Normal;   ///< The current state of LED light output

        EasyTimer mUpshiftBlinkTimer = EasyTimer(20);   ///< The rate at which the lights during upshift should blink
        bool mUpshiftOn = false;  ///< Whether the lights are currently on or off in the upshift flash

        int mBarPWMPositions;   ///< The number total pwms that the bar has (calculated upon construction. 255 posns per LED)
        int mNumYellow;   ///< number of yellow pixels
        int mNumRed;      ///< number of red pixels

        int CalculatePWM(int &totalBarPWMs);

    protected:


    public:

        /** Constructor */
        CLightBarRPM(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs, StateSignal &rpmSignal, int minRPM, int maxRPM, int downshiftRPM = -1);

        /** Destructor */
        virtual ~CLightBarRPM() {};

        /** Copy constructor disabled */
        CLightBarRPM(const CLightBarRPM &) = delete;
        /** Assignment operator disabled */
        void operator=(const CLightBarRPM &) = delete;

        virtual void Initialize() override;
        virtual void Update(unsigned long &elapased) override;

};

/*
 * Constructor
 * \param lights The neopixel light object
 * \param first index The first index of the neopixels to use
 * \param numLEDs The number of LEDs to use
 * \param rpm The RPM signal
 * \param minRPM The minimum bound for the RPM bar to start lighting up
 * \param maxRPM The maximum bound for the rpm bar light up before signalling an upshift
 * \param downshiftRPM The RPM to signal a downshift below
 */
CLightBarRPM::CLightBarRPM(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs, StateSignal &rpmSignal, int minRPM, int maxRPM, int downshiftRPM) : 
            CLightBar(lights, firstIndex, numLEDs), mRPM(rpmSignal), mMin(minRPM), mMax(maxRPM), mDownshift(downshiftRPM) {

    mBarPWMPositions = numLEDs * 255;
    mNumYellow = numLEDs * 0.25;
    mNumRed = numLEDs * 0.25;
}



/**
 * Initialize the lights by clearing them.
 */
void CLightBarRPM::Initialize(){
    CLightBar::Initialize();
}



/**
 *  Take the total PWM remaining for the bar, then returns a value 0-255 for an incremental
 *  LED in the bar of LEDs.
 * \returns an integer 0-255 representing a PWM value;
 */
int CLightBarRPM::CalculatePWM(int &totalBarPWMs){

    // led should be completely on
    if (totalBarPWMs >= 255){
        totalBarPWMs -= 255;
        return 255;
    
    // led should be completely off
    } else if (totalBarPWMs <= 0){
        return 0;

    // led is somewhere in the middle
    } else {
        int temp = totalBarPWMs;
        totalBarPWMs = 0;
        return temp;
    }
}




/**
 * If the frame rate timer is all good, update the screen
 *
 * \param elapsed The time in milliseconds elapsed since last called
 */
void CLightBarRPM::Update(unsigned long &elapsed){


    // first, determine the state
    if ((int)mRPM.value() > mMax){
        mState = Upshift;

    } else if ((int)mRPM.value() > mMin){
        mState = Normal;

    } else if ((int)mRPM.value() > mDownshift ){
        mState = Off;

    } else {
        mState = Downshift;

    }

    
    //Serial.println(mState);


    switch (mState){
        case Downshift:
            {
                int red = 0;
                for (int i = GetFirstLEDIndex(); i <= GetLastLEDIndex(); i++){
                    mLights.setPixelColor(i, red, 0, 255); // burple
                    if (i < GetNumLEDs()/2 - 1){
                        red += 30;
                    } else if (i == GetNumLEDs()/2) { 
                        // do nothing
                    } else {
                        red -= 30;
                    }
                }
            }
            break;

        case Off:
            Clear();
            CLightBar::Update(elapsed);
            break;

        case Normal:
            {  // brackets needed since we initilize a value here
                int barPWMs = map(mRPM.value(), mMin, mMax, 0, mBarPWMPositions);

                for (int led = GetFirstLEDIndex(); led <= GetLastLEDIndex(); led++){

                    // green
                    if (led < GetNumLEDs() - mNumYellow - mNumRed){
                        mLights.setPixelColor(led, 0, CalculatePWM(barPWMs), 0);
                    
                    // yellow
                    } else if (led < GetNumLEDs() - mNumRed){
                        int pwms = CalculatePWM(barPWMs);
                        mLights.setPixelColor(led, pwms, pwms, 0);

                    // red
                    } else {
                        mLights.setPixelColor(led, CalculatePWM(barPWMs), 0, 0);
                    }

                    
                }
            }
            break;


        case Upshift:
            
            if (mUpshiftBlinkTimer.isup()){
                if (mUpshiftOn){
                    mUpshiftOn = false;
                    Clear();
                } else {
                    mUpshiftOn = true;
                    for (int i = GetFirstLEDIndex(); i <= GetLastLEDIndex(); i++){
                        mLights.setPixelColor(i, 255, 0, 0); // red
                    }
                }
            }

            break;
    }

    // sends the update
    CLightBar::Update(elapsed);

}





#endif