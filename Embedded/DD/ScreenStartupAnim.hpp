#ifndef SCREENSTARTUPANIM_H 
#define SCREENSTARTUPANIM_H 

#include "arduino.h"
#include "ILI9341_t3n.h"
#include <EasyTimer.h>
#include <StateCAN.h>

#include "Screen.hpp"


#include "images/SpartanBitmap.hpp"
#include "images/StateRacingBitmap.hpp"


/*
 * base class for all screen displays
 */
class CScreenStartupAnim : public CScreen {


    public:

        CScreenStartupAnim(ILI9341_t3n &disp);

        /** Destructor */
        virtual ~CScreenStartupAnim() {};

        /** Copy constructor disabled */
        CScreenStartupAnim(const CScreenStartupAnim &) = delete;
        /** Assignment operator disabled */
        void operator=(const CScreenStartupAnim &) = delete;

        //virtual void Initialize() override;
        virtual void Update(unsigned long &elapsed) override;

        /* Animation completion
         * \returns true when the animation has been completed 
         */
        bool IsCompleted(){return mCompleted;}

    private:

        bool mCompleted = false;   ///< If the animation is complete, this is set to true

        float mSpartyPosX = 0;   ///< current X position of sparty in pixels
        float mSpartyPosY = 0;   ///< current Y position of sparty in pixels
        float mSpartySpeed = 400;  ///< Speed in pixels per second

        int mStateRacingPosX = 0; ///< current X position of state racing in pixels
        int mStateRacingPosY = 0; ///< current Y position of state racing in pixels

        
};



/**
 * Constructor
 * \param disp The low-level dispay class
 */
CScreenStartupAnim::CScreenStartupAnim(ILI9341_t3n &disp) : CScreen(disp) {

    // set max framerate to 30 fps (this will never be achieved lol)
    mFrameRateTimer.set_frequency(30);

    mSpartyPosY = (mHeight - spartanBitmapHeight) / 2;  // center the sparty vertically
    mSpartyPosX = -spartanBitmapWidth;

    mStateRacingPosY = (mHeight - stateRacingBitmapHeight) / 2;
    mStateRacingPosX = (mWidth  - stateRacingBitmapWidth ) / 2;
};



/**
 * If the frame rate timer is all good, update the screen
 */
void CScreenStartupAnim::Update(unsigned long &elapsed){

    float secondsElapsed = float(elapsed) / 1000.0;

    mSpartyPosX += secondsElapsed * mSpartySpeed;

    if (mFrameRateTimer.isup()){

        mDisplay.fillScreen(mColorBackground);

        // draw state racing bitmap
        mDisplay.drawBitmap(mStateRacingPosX, mStateRacingPosY, stateRacingBitmap, stateRacingBitmapWidth, stateRacingBitmapHeight, mColorPrimary);

        // cover part of the SR bitmap with black so sparty reveals it
        mDisplay.fillRect(mSpartyPosX + 20, mSpartyPosY, mWidth - mSpartyPosX, spartanBitmapHeight, mColorBackground);
        
        // draw sparty at the current position
        mDisplay.drawBitmap(mSpartyPosX, mSpartyPosY, spartanBitmap, spartanBitmapWidth, spartanBitmapHeight, mColorSecondary);

        // send the updates to the screen
        mDisplay.updateScreen();

    }

    // determine if sparty if off of the screen yet ("we are complete")
    if (mSpartyPosX >= mWidth){
        mCompleted = true;
    }


}


#endif