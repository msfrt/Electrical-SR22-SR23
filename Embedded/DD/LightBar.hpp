#ifndef LIGHT_BAR_H 
#define LIGHT_BAR_H 

#include "arduino.h"
#include <Adafruit_NeoPixel.h>
#include <EasyTimer.h>


/*
 * base class for all screen displays
 */
class CLightBar {

    private:
         

    protected:
        Adafruit_NeoPixel &mLights;  ///< The low-level object that actually interfaces with the hardware

        void Clear();

    public:

        /** Constructor */
        CLightBar(Adafruit_NeoPixel &lights) : mLights(lights) {};

        /** Destructor */
        virtual ~CLightBar() {};

        /** Copy constructor disabled */
        CLightBar(const CLightBar &) = delete;
        /** Assignment operator disabled */
        void operator=(const CLightBar &) = delete;

        virtual void Initialize();
        virtual void Update(unsigned long &elapased);

};



/**
 * Initialize the lights by clearing them.
 */
void CLightBar::Initialize(){
    Clear();
}

/**
 * Initialize the lights by clearing them.
 */
void CLightBar::Clear(){
    for (int i = 0; i <= mLights.numPixels(); i++){
        mLights.setPixelColor(i, 255, 0, 0);
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