#ifndef SCREEN_LAPTIME_H 
#define SCREEN_LAPTIME_H 

#include "arduino.h"
#include "ILI9341_t3n.h"
#include <EasyTimer.h>
#include <StateCAN.h>

#include "Screen.hpp"

// arduino compiler fails to find the c files unless you include them for whatever reason
#include "fonts/LiberationMonoBold.h"
#include "fonts/LiberationMonoBold.c"
#include "fonts/LiberationMonoBoldItalic.h"
#include "fonts/LiberationMonoBoldItalic.c"


/*
 * base class for all screen displays
 */
class CScreenLapTime : public CScreen {


    public:

        /** Constructor */
        CScreenLapTime(ILI9341_t3n &disp) : CScreen(disp) {};

        /** Destructor */
        virtual ~CScreenLapTime() {};

        /** Copy constructor disabled */
        CScreenLapTime(const CScreenLapTime &) = delete;
        /** Assignment operator disabled */
        void operator=(const CScreenLapTime &) = delete;

        virtual void Initialize() override;
        virtual void Update(unsigned long &elapsed) override;

        
        
    private:

        /*
         * Constants that affect the spacing of objects on the displau
         */
        const int mMarginX = 1;   ///< horizontal padding pixels
        const int mMarginY = 12;  ///< veritcal padding pixels
        const ILI9341_t3_font_t &mFontLabel = LiberationMono_32_Bold_Italic;   ///< The font for the screen label
        const int mFontLabelHeight = 32;   ///< The label's pixel height
        const int mFontLabelWidth = 26;   ///< The label's pixel width
        const ILI9341_t3_font_t &mFontPrimary = LiberationMono_48_Bold;   ///< The font for lap time
        const int mFontPrimaryHeight = 48;   ///< The font pixel height
        const int mFontPrimaryWidth = 40;   ///< The font pixel height
        String mLabel ="LAP TIME:";   ///< The screen's label
        
        float mBestLapTime = 9999999999;   ///< Will hold the best laptime in seconds
        float mLastLapTime = 9999999999;   ///< Will hold the last laptime in seconds
        unsigned long mLastLapMillis = 0;   ///< Will hold the milliseconds clock time that the last lap happened
};





/**
 * This is called ONLY when there is a new laptime recieved
 * This draws the labels, calculates values, and updates the screen for the first time.
 */
void CScreenLapTime::Initialize(){

    // before doing any screen mumbo jumbo, calculate the times
    float currentLapTime = static_cast<float>(millis() - mLastLapMillis) / 1000.0;
    mLastLapMillis = millis();

    CScreen::Initialize(); 
    
    // borders
    mDisplay.drawFastHLine(0,           0, mWidth, mColorSecondary);
    mDisplay.drawFastHLine(0, mHeight - 1, mWidth, mColorSecondary);
    // mDisplay.drawFastHLine(0, mHeight / 2, mWidth, mColorSecondary);
    // mDisplay.drawFastVLine(mWidth / 2, mHeight / 2, mHeight / 2, mColorSecondary);

    // print the screen label
    mDisplay.setCursor(2, 5);
    mDisplay.setFont(mFontLabel);
    mDisplay.setTextColor(mColorPrimary);
    mDisplay.print(mLabel);

    // display the lap time
    char formatBuf[10];
    sprintf(formatBuf, "%.2f", currentLapTime);
    mDisplay.setCursor((mWidth - strlen(formatBuf)*mFontPrimaryWidth)/2, mMarginY + mFontLabelHeight + 30);
    mDisplay.setFont(mFontPrimary);
    mDisplay.print(formatBuf);

    // display best/last rectangles
    int rectX = 0;
    int rectY = (mHeight * 2) / 3;
    int rectW = mWidth / 2;
    int rectH = (mHeight * 1) / 3;
    uint16_t color = currentLapTime < mBestLapTime ? mColorSuccess : mColorDanger;
    mDisplay.fillRect(rectX, rectY, rectW, rectH, color);
    mDisplay.setCursor(rectX + (rectW - 4*mFontLabelWidth)/2, rectY + (rectH - mFontLabelHeight) / 2);
    mDisplay.setFont(mFontLabel);
    mDisplay.setTextColor(mColorBackground);
    mDisplay.print("BEST");

    rectX = mWidth / 2;
    rectY = (mHeight * 2) / 3;
    rectW = mWidth / 2;
    rectH = (mHeight * 1) / 3;
    color = currentLapTime < mLastLapTime ? mColorSuccess : mColorDanger;
    mDisplay.fillRect(rectX, rectY, rectW, rectH, color);
    mDisplay.setCursor(rectX + (rectW - 4*mFontLabelWidth)/2, rectY + (rectH - mFontLabelHeight) / 2);
    mDisplay.setFont(mFontLabel);
    mDisplay.setTextColor(mColorBackground);
    mDisplay.print("LAST");

    // draw a line dividing the two
    mDisplay.drawFastVLine(rectX, rectY, rectH, mColorBackground);

    // update best time if necessary
    if (currentLapTime < mBestLapTime){
        mBestLapTime = currentLapTime;
    }
    // update the last time
    mLastLapTime = currentLapTime;

    // push updates to the screen
    unsigned long elapsed = 0;
    CScreen::Update(elapsed);
}





/**
 * If the frame rate timer is all good, update the screen
 * 
 * \param elapsed The time elapsed in milliseconds since the update was last called
 */
void CScreenLapTime::Update(unsigned long &elapsed){
    // do nothing here, since everything is controlled in the initialization function
}


#endif