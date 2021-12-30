#ifndef SCREEN_H 
#define SCREEN_H 

#include "arduino.h"
#include "ILI9341_t3n.h"
#include <EasyTimer.h>


/*
 * base class for all screen displays
 */
class CScreen {

    private:
         

    protected:
        ILI9341_t3n &mDisplay;  ///< The low-level object that actually interfaces with the hardware
        EasyTimer mFrameRateTimer = EasyTimer(10);  ///< Timer for screen updates, default at 10 Hz
        int mWidth = 0;  ///< Width of the display
        int mHeight = 0;  ///< Height of the display

        /*
         * Primary and secondary colors to use for all screens
         */
        uint16_t mColorPrimary    = ILI9341_WHITE;
        uint16_t mColorSecondary  = ILI9341_GREEN;
        uint16_t mColorBackground = ILI9341_BLACK;
        uint16_t mColorWarning    = ILI9341_YELLOW;
        uint16_t mColorDanger     = ILI9341_RED;



    public:

        /** Constructor */
        CScreen(ILI9341_t3n &disp, int width, int height) : 
                mDisplay(disp), mWidth(width), mHeight(height) {};

        /** Destructor */
        // ~CScreen() {};

        /** Copy constructor disabled */
        CScreen(const CScreen &) = delete;
        /** Assignment operator disabled */
        void operator=(const CScreen &) = delete;

        virtual void Initialize();
        virtual void Update();
        void SetMaxFrameRate(uint8_t freq);

};



/**
 * Initialize the screen.
 * This ensures that the frame buffer is turned on. This means that no updates are sent to the screen until 
 * UpdateScreen() is explicitly called.
 */
void CScreen::Initialize(){
    mDisplay.useFrameBuffer(true);
    mDisplay.fillScreen(mColorBackground);
}

/**
 * If the frame rate timer is all good, update the screen
 */
void CScreen::Update(){
    if (mFrameRateTimer.isup()){
        mDisplay.updateScreen();
    }
}

/**
 * Change the maximum frequency that the screen can update
 * \param freq - the maximum frequency for screen updates
 */
void CScreen::SetMaxFrameRate(uint8_t freq){
    if (freq == 0){
        mFrameRateTimer.disable();
    } else {
        mFrameRateTimer.enable();
        mFrameRateTimer.set_frequency(freq);
    }
}



#endif