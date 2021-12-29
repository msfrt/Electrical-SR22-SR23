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

/// The size of each char array used in this class. Keep it as one value for simplicity
#define SCREEN_INFO_BUF_SIZE 10


/*
 * base class for all screen displays
 */
class CScreenInfo : public CScreen {


    public:

        /** Constructor */
        CScreenInfo(ILI9341_t3n &disp, int width, int height) : CScreen(disp, width, height) {};

        /** Destructor */
        // ~CScreen() {};

        /** Copy constructor disabled */
        CScreenInfo(const CScreenInfo &) = delete;
        /** Assignment operator disabled */
        void operator=(const CScreenInfo &) = delete;

        //virtual void Initialize() override;
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
        String mLabel1;
        String mLabel2;
        String mLabel3;
        String mLabel4;

        /* 
         * the number of digits (including '.') that there is space to display.
         * The digits per signal are automatically calculated, but default to 4
         */
        const int mIndeciesPerLine = 9;  /// the number of digits that can fit horizontally accross the screen
        int mDigitsSig1 = 4;
        int mDigitsSig2 = 4;
        int mDigitsSig3 = 4;
        int mDigitsSig4 = 4;

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
        // char mFormatSig1[10] = "%-4.2f";
        // char mFormatSig2[10] = "%-4.2f";
        // char mFormatSig3[10] = "%-4.2f";
        // char mFormatSig4[10] = "%-4.2f";

        /*
         * These hold the signals' previous values.
         */
        float mPrevSig1 = 0;
        float mPrevSig2 = 0;
        float mPrevSig3 = 0;
        float mPrevSig4 = 0;



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
}



void CScreenInfo::DrawLabels(){
    int rowHeight = mHeight / 4;
    mDisplay.setTextColor(mColorPrimary, mColorBackground);
    mDisplay.setTextWrap(false);
    mDisplay.setFont(mFont);
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
            mDigitsSig1 = mIndeciesPerLine - label.length();
            break;
        case 2:
            mSignal2 = sig;
            mLabel2 = label;
            mFormatSig2 = formatting;
            mDivisorSig2 = divisor;
            mDigitsSig2 = mIndeciesPerLine - label.length();
            break;
        case 3:
            mSignal3 = sig;
            mLabel3 = label;
            mFormatSig3 = formatting;
            mDivisorSig3 = divisor;
            mDigitsSig3 = mIndeciesPerLine - label.length();
            break;
        case 4:
            mSignal4 = sig;
            mLabel4 = label;
            mFormatSig4 = formatting;
            mDivisorSig4 = divisor;
            mDigitsSig4 = mIndeciesPerLine - label.length();
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
    String *label = nullptr;
    int *digits = nullptr;
    int *divisor = nullptr;
    String *format = nullptr;
    float *previousVal = nullptr;

    switch (position){
        case 1:
            signal = mSignal1;
            label = &mLabel1;
            digits = &mDigitsSig1;
            divisor = &mDigitsSig1;
            format = &mFormatSig1;
            previousVal = &mPrevSig1;
            break;
        case 2:
            signal = mSignal2;
            label = &mLabel2;
            digits = &mDigitsSig2;
            divisor = &mDigitsSig2;
            format = &mFormatSig2;
            previousVal = &mPrevSig2;
            break;
        case 3:
            signal = mSignal3;
            label = &mLabel3;
            digits = &mDigitsSig3;
            divisor = &mDigitsSig3;
            format = &mFormatSig3;
            previousVal = &mPrevSig3;
            break;
        case 4:
            signal = mSignal4;
            label = &mLabel4;
            digits = &mDigitsSig4;
            divisor = &mDigitsSig4;
            format = &mFormatSig4;
            previousVal = &mPrevSig4;
            break;
    }

    // if the signal was never assigned, that means that it is invalid, so return
    if (!signal){
        return false;
    }

    mDisplay.setFont(mFont);
    mDisplay.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    
    mDisplay.println(*label);

    return true;

}


/**
 * If the frame rate timer is all good, update the screen
 */
void CScreenInfo::Update(){
    UpdateSignal(1);
    UpdateSignal(2);
    UpdateSignal(3);
    UpdateSignal(4);
    DrawLines();

    // push updates to the screen
    mDisplay.updateScreen();
}






#endif