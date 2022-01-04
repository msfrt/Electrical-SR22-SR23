#ifndef SCREEN_MESSAGE_H 
#define SCREEN_MESSAGE_H 

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
class CScreenMessage : public CScreen {


    public:

        /** Constructor */
        CScreenMessage(ILI9341_t3n &disp) : CScreen(disp) {};

        /** Destructor */
        virtual ~CScreenMessage() {};

        /** Copy constructor disabled */
        CScreenMessage(const CScreenMessage &) = delete;
        /** Assignment operator disabled */
        void operator=(const CScreenMessage &) = delete;

        virtual void Initialize() override;
        virtual void Update(unsigned long &elapsed) override;

        /**
         * Sets the message that is to be displayed upon a call to Update
         * \param message The message (max 8 characters) to display on the screen
         */
        void SetMessage(String message) {mMessage = message;}
        
    private:

        /*
         * Constants that affect the spacing of objects on the displau
         */
        const int mMarginX = 1;   ///< horizontal padding pixels
        const int mMarginY = 12;  ///< veritcal padding pixels
        const ILI9341_t3_font_t &mFontLabel = LiberationMono_32_Bold_Italic;   ///< The font for the screen label
        const ILI9341_t3_font_t &mFont = LiberationMono_48_Bold;   ///< The font to use
        const int mFontHeight = 48;  ///< The height of the font in pixels
        const int mFontWidth = 35;  ///< the font width in pixels (I think that you need to guess this)
        const int mMaxChars = 8;   ///< The maximum number of characters that the screen can display

        String mMessage = "yessir!!";   ///< The message to be displayed on the screen upon a call to Initialize()
        String mLabel = "Message:"; ///< The label to display on the top of the screen
};





/**
 * Initialize the screen.
 * This draws the labels and updates the screen for the first time.
 */
void CScreenMessage::Initialize(){
    CScreen::Initialize(); 
    
    // top and bottom borders
    mDisplay.drawFastHLine(0,           0, mWidth, mColorSecondary);
    mDisplay.drawFastHLine(0, mHeight - 1, mWidth, mColorSecondary);

    // print the screen label
    mDisplay.setCursor(2, 5);
    mDisplay.setFont(mFontLabel);
    mDisplay.setTextColor(mColorPrimary);
    mDisplay.print(mLabel);

    // String.length() seems to be counting \0 as a character... how dum
    int realLength = 0;
    for (unsigned int i=0; i<mMessage.length(); i++){
        if (mMessage.c_str()[i] != '\0'){
            realLength += 1;
        }
    }

    mDisplay.setCursor((mMaxChars - realLength) * ((mWidth / 2) / mMaxChars), mHeight/2 - mFontHeight/2);
    mDisplay.setFont(LiberationMono_48_Bold);
    mDisplay.print(mMessage);

    // push updates to the screen
    unsigned long elapsed = 0;
    CScreen::Update(elapsed);
}





/**
 * If the frame rate timer is all good, update the screen
 * 
 * \param elapsed The time elapsed in milliseconds since the update was last called
 */
void CScreenMessage::Update(unsigned long &elapsed){
    // do nothing here, since everything is controlled in the initialization function


}


#endif