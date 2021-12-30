#ifndef SCREENINFO_H 
#define SCREENINFO_H 

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
class CScreenInfo : public CScreen {


    public:

        /** Constructor */
        CScreenInfo(ILI9341_t3n &disp) : CScreen(disp) {};

        /** Destructor */
        virtual ~CScreenInfo() {};

        /** Copy constructor disabled */
        CScreenInfo(const CScreenInfo &) = delete;
        /** Assignment operator disabled */
        void operator=(const CScreenInfo &) = delete;

        virtual void Initialize() override;
        virtual void Update() override;

        void SetSignal(int pos, StateSignal *sig, String label, String formatting, int divisor = 1);

    private:

        /*
         * Constants that affect the spacing of objects on the displau
         */
        const int mMarginX = 1;   ///< horizontal padding pixels
        const int mMarginY = 12;  ///< veritcal padding pixels
        const ILI9341_t3_font_t &mFont = LiberationMono_40_Bold;   ///< The font to use
        const int mFontWidth = 35;  ///< the font width in pixels (I think that you need to guess this)

        /* 
         * Signals to display 
         */
        StateSignal *mSignal1 = nullptr;
        StateSignal *mSignal2 = nullptr;
        StateSignal *mSignal3 = nullptr;
        StateSignal *mSignal4 = nullptr;

        /* 
         * Labels placed to the left side of every number 
         */
        String mLabel1 = "";
        String mLabel2 = "";
        String mLabel3 = "";
        String mLabel4 = "";


        /* 
         * additional scaling that you may want to do. For example, instead of 
         * display 5 digits of RPM, we can set one of these scalers to 1000 to get 
         * rpm / 1000 
        */
        int mDivisorSig1 = 1;
        int mDivisorSig2 = 1;
        int mDivisorSig3 = 1;
        int mDivisorSig4 = 1;

        /*
         * sprintf formatting for each signal
         */
        String mFormatSig1 = "%-4.2f";
        String mFormatSig2 = "%-4.2f";
        String mFormatSig3 = "%-4.2f";
        String mFormatSig4 = "%-4.2f";
        
        /**
         * Character buffer used for string formatting
         */
        char mFormatBuf[10] = "\0\0\0\0\0\0\0\0\0";

        /*
         * These hold the signals' previous values.
         * They can be any value now, since they will be overwritten.
         * If you set to 0, it's likely that the screen will be blank upon startup because
         * the signals also have a value of 0.
         */
        float mPrevSig1 = 1337;
        float mPrevSig2 = 1337;
        float mPrevSig3 = 1337;
        float mPrevSig4 = 1337;



        bool UpdateSignal(int position, bool override=false);
        void DrawLines();
        void DrawLabels();

};



/**
 * Initialize the screen.
 * This draws the labels and updates the screen for the first time.
 */
void CScreenInfo::Initialize(){
    CScreen::Initialize(); 
    DrawLabels();
}



void CScreenInfo::DrawLabels(){
    int rowHeight = mHeight / 4;
    mDisplay.setTextColor(mColorPrimary, mColorBackground);
    mDisplay.setTextWrap(false);
    mDisplay.setFont(mFont);

    for (int i=0; i<4; i++){
        mDisplay.setCursor(mMarginX, mMarginY + rowHeight * i);
        switch (i){
            case 0:
                mDisplay.print(mLabel1);
                break;
            case 1:
                mDisplay.print(mLabel2);
                break;
            case 2:
                mDisplay.print(mLabel3);
                break;
            case 3:
                mDisplay.print(mLabel4); 
                break;
        }
    }
}



/**
 * Draws evenly spaced horizontal lines that divide the signals
 */
void CScreenInfo::DrawLines(){
    int rowHeight = mHeight / 4;
    mDisplay.drawFastHLine(0,             0,     mWidth, mColorSecondary);
    mDisplay.drawFastHLine(0, rowHeight    ,     mWidth, mColorSecondary);
    mDisplay.drawFastHLine(0, rowHeight * 2,     mWidth, mColorSecondary);
    mDisplay.drawFastHLine(0, rowHeight * 3,     mWidth, mColorSecondary);
    mDisplay.drawFastHLine(0, rowHeight * 4 - 1, mWidth, mColorSecondary);
}



/*
 * Sets the information specific to each signal, as well as this signal itself
 *
 * \param pos - The position of the signal on screen. Values can be one through four.
 * \param sig - A pointer to the StateSignal object that updates with the actual value.
 * \param label - The string label to display on the screen. Example "ENG:" for engine
 * \param formatting - the string for formatting the numerical signal. 
 *                     Refer to this for help: https://www.cplusplus.com/reference/cstdio/printf/
 * \param divisor - Optional integer. Divide the raw signal by the divisor before displaying on screen.
 *                  For example, divide RPM by 1000 to get 12,345 to display as 12.3 in screen.
 *                  Default value is 1 (no effect)
 */
void CScreenInfo::SetSignal(int pos, StateSignal *sig, String label, String formatting, int divisor){

    // create references to the objects that we want to change depending on the position
    switch (pos) {
        case 1:
            mSignal1 = sig;
            mLabel1 = label;
            mFormatSig1 = formatting;
            mDivisorSig1 = divisor;
            break;
        case 2:
            mSignal2 = sig;
            mLabel2 = label;
            mFormatSig2 = formatting;
            mDivisorSig2 = divisor;
            break;
        case 3:
            mSignal3 = sig;
            mLabel3 = label;
            mFormatSig3 = formatting;
            mDivisorSig3 = divisor;
            break;
        case 4:
            mSignal4 = sig;
            mLabel4 = label;
            mFormatSig4 = formatting;
            mDivisorSig4 = divisor;
            break;
    }

}




/**
 * Update an individual signal based upon its position
 \param position The signal position on screen (1-4)
 \param override Update the screen info even if the value has not changed (default is false)
 */
bool CScreenInfo::UpdateSignal(int position, bool override){

    // these pointers will point to whatever position's variables that we select
    StateSignal *signal = nullptr;
    int *divisor = nullptr;
    String *format = nullptr;
    float *previousVal = nullptr;

    switch (position){
        case 1:
            signal = mSignal1;
            divisor = &mDivisorSig1;
            format = &mFormatSig1;
            previousVal = &mPrevSig1;
            break;
        case 2:
            signal = mSignal2;
            divisor = &mDivisorSig2;
            format = &mFormatSig2;
            previousVal = &mPrevSig2;
            break;
        case 3:
            signal = mSignal3;
            divisor = &mDivisorSig3;
            format = &mFormatSig3;
            previousVal = &mPrevSig3;
            break;
        case 4:
            signal = mSignal4;
            divisor = &mDivisorSig4;
            format = &mFormatSig4;
            previousVal = &mPrevSig4;
            break;
    }

    // if the signal was never assigned, that means that it is invalid, so return
    if (!signal){
        return false;
    }


    // if the value hasn't been updated, don't write again to the screen!
    if (*previousVal == signal->value()){
        return false;
    } else {
        // the value is NOT the same as last time, so update the previous value and continue writing to the screen
        *previousVal = signal->value();
    }


    mDisplay.setFont(mFont);
    mDisplay.setTextColor(mColorPrimary);

    // calculate the formatted string
    sprintf(mFormatBuf, format->c_str(), signal->value() / *divisor);

    // calculate and set the cursor position based off of the signal and formatted string
    int rowHeight = mHeight / 4;
    int xPos = mWidth - (strlen(mFormatBuf) * mFontWidth);
    int yPos = rowHeight * (position - 1) + mMarginY;
    mDisplay.setCursor(xPos, yPos);

    // fill the background (can't use text background because it's really large and overlaps other text. 
    // We'll draw our own to avoid this.
    mDisplay.fillRect(xPos, yPos - mMarginY, mWidth - xPos, rowHeight, mColorBackground);

    // send it!
    mDisplay.print(mFormatBuf);

    // return true because we update it
    return true;

}


/**
 * If the frame rate timer is all good, update the screen
 */
void CScreenInfo::Update(){
    if (mFrameRateTimer.isup()){
        bool updated = false;
        for (int i=1; i<=4; i++){
            updated |= UpdateSignal(i);
        }       
        
        // finalize the screen and send over SPI
        if (updated){
            DrawLines();
            mDisplay.updateScreen();
        }
    }
}


#endif