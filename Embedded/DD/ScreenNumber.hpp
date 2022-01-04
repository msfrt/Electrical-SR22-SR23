#ifndef SCREEN_NUMBER_H 
#define SCREEN_NUMBER_H 

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
#include "fonts/Liberation_BIG.hpp"


/*
 * base class for all screen displays
 */
class CScreenNumber : public CScreen {


    public:

        /** Constructor */
        CScreenNumber(ILI9341_t3n &disp, StateSignal &signal, String label) : CScreen(disp), mSignal(signal), mLabel(label) {};

        /** Destructor */
        virtual ~CScreenNumber() {};

        /** Copy constructor disabled */
        CScreenNumber(const CScreenNumber &) = delete;
        /** Assignment operator disabled */
        void operator=(const CScreenNumber &) = delete;

        virtual void Initialize() override;
        virtual void Update(unsigned long &elapsed) override;

        
        
    private:

        /*
         * Constants that affect the spacing of objects on the displau
         */
        const int mMarginX = 1;   ///< horizontal padding pixels
        const int mMarginY = 12;  ///< veritcal padding pixels
        const ILI9341_t3_font_t &mFontLabel = LiberationMono_32_Bold_Italic;   ///< The font for the screen label
        const int mFontLabelHeight = 32;
        const ILI9341_t3_font_t &mFont = Liberation_BIG;   ///< The font to use
        const int mFontHeight = 144;  ///< Estimate of the height of the font in pixels
        const int mFontWidth = 100; ///< estimation of the font width in pixels
        //const int mFontWidth = 35;  ///< the font width in pixels (I think that you need to guess this)
        //const int mMaxChars = 8;   ///< The maximum number of characters that the screen can display
        int mPrevVal = 0; ///< write to the screen if previos val is different than current val
        
        StateSignal &mSignal;  ///< The number to display on the screen
        String mLabel = ""; ///< The label to display on the top of the screen

        String mFormat = "%d";  ///< THe format for the number printed

};





/**
 * Initialize the screen.
 * This draws the labels and updates the screen for the first time.
 */
void CScreenNumber::Initialize(){
    CScreen::Initialize(); 
    
    // top and bottom borders
    mDisplay.drawFastHLine(0,           0, mWidth, mColorSecondary);
    mDisplay.drawFastHLine(0, mHeight - 1, mWidth, mColorSecondary);

    // print the screen label
    mDisplay.setCursor(2, 5);
    mDisplay.setFont(mFontLabel);
    mDisplay.setTextColor(mColorPrimary);
    mDisplay.print(mLabel);

    // set the previous value to something outragous
    mPrevVal = -1337;

    // push updates to the screen
    unsigned long elapsed = 0;
    CScreen::Update(elapsed);
}





/**
 * If the frame rate timer is all good, update the screen
 * 
 * \param elapsed The time elapsed in milliseconds since the update was last called
 */
void CScreenNumber::Update(unsigned long &elapsed){
    // do nothing here, since everything is controlled in the initialization function

    if (static_cast<int>(mSignal.value()) != mPrevVal){
        // update the value
        mPrevVal = static_cast<int>(mSignal.value());

        // overwrite old stuff
        mDisplay.fillRect(0, mMarginY + mFontLabelHeight, mWidth, mHeight - (mMarginY + mFontLabelHeight) - 1, mColorBackground);

        
        mDisplay.setFont(mFont);
        mDisplay.setTextColor(mColorPrimary, mColorBackground);
        char formatBuf[10];
        sprintf(formatBuf, "%.0f", mSignal.value());
        int numLen = strlen(formatBuf);
        mDisplay.setCursor((mWidth - numLen*mFontWidth)/2, mHeight/2 - mFontHeight/2);
        mDisplay.print(formatBuf);
        Serial.println(formatBuf);
        //mDisplay.print(round(mSignal.value()));

        CScreen::Update(elapsed);
    }

}


#endif