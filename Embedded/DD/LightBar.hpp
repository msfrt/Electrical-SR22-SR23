#ifndef LIGHT_BAR_H 
#define LIGHT_BAR_H 

#include "arduino.h"
#include <Adafruit_NeoPixel.h>
#include <EasyTimer.h>


/*
 * base class for all screen displays
 */
class CLightBar {
         

    protected:
        Adafruit_NeoPixel &mLights;  ///< The low-level object that actually interfaces with the hardware

        /*
         * Gets the index of the first LED to update
         */
        int GetFirstLEDIndex() const {return mFirstLED;}

        /*
         * Gets the index of the last LED to update
         */
        int GetLastLEDIndex() const {return mFirstLED + mNumLEDs - 1;}

        /*
         * Get the number of LEDs in this light bar
         */
        int GetNumLEDs() const {return mNumLEDs;}

    private:

        int mFirstLED;
        int mNumLEDs;

    public:

        /**
         * Constructor
         * \param lights The neopixel lights object
         * \param firstIndex The index of the first LED to use (LED indecies start at 0 in neopixels)
         * \param numLEDS The number of LEDs to update
         */
        CLightBar(Adafruit_NeoPixel &lights, int firstIndex, int numLEDs) : mLights(lights), mFirstLED(firstIndex), mNumLEDs(numLEDs) {};

        /** Destructor */
        virtual ~CLightBar() {};

        /** Copy constructor disabled */
        CLightBar(const CLightBar &) = delete;
        /** Assignment operator disabled */
        void operator=(const CLightBar &) = delete;

        virtual void Initialize();
        virtual void Update(unsigned long &elapased);
        void Clear();

};



/**
 * Initialize the lights by clearing them.
 */
void CLightBar::Initialize(){
    mLights.begin();
    Clear();
    mLights.show();
}

/**
 * Initialize the lights by clearing them.
 */
void CLightBar::Clear(){
    for (int i = GetFirstLEDIndex(); i <= GetLastLEDIndex(); i++){
        mLights.setPixelColor(i, 0, 0, 0);
    }
}



/**
 * If the frame rate timer is all good, update the screen
 *
 * \param elapsed The time in milliseconds elapsed since last called
 */
void CLightBar::Update(unsigned long &elapsed){
    mLights.show();
}





#endif