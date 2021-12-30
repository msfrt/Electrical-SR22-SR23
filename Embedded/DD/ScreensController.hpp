#ifndef CSCREENCONTROLLER_HPP
#define CSCREENCONTROLLER_HPP

#include "ScreenInfo.hpp"

class CScreensController {

    public:

        CScreensController(ILI9341_t3n &left, ILI9341_t3n &right);

        /** Destructor */
        ~CScreensController();

        /** Copy constructor disabled */
        CScreensController(const CScreensController &) = delete;
        /** Assignment operator disabled */
        void operator=(const CScreensController &) = delete;

        /// the different states that the screens can be in. Please note that states should include
        /// transitionary states, such as ScreenBegin before Screen or whatever
        enum ScreenStates { InfoScreen1, InfoScreen2, InfoScreen3, Titan };

        /**
         * Gets the current state of the screen
         * @returns the screen state
         */
        int GetState() {return mState;}

        void Update();
        void Initialize();
        void OnButtonPress();

    private:

        ILI9341_t3n &mDisplayLeft;
        ILI9341_t3n &mDisplayRight;

        void SetState(ScreenStates state);

        ScreenStates mState = InfoScreen1;  ///< screen states
        ScreenStates mStatePrev = InfoScreen1;  ///< Used to return to previous screen states after temporary states
        unsigned long mStateTime = 0;  ///< The time in milliseconds that the current state started


        /* Screen definitions */
        CScreenInfo *mInfoScreen1Left  = nullptr;
        CScreenInfo *mInfoScreen1Right = nullptr;

};


/**
 * Constructor
 *
 * Constructs screens for all of the states and then sets the initial state
 */
CScreensController::CScreensController(ILI9341_t3n &left, ILI9341_t3n &right) : 
                                       mDisplayLeft(left), mDisplayRight(right){

    mInfoScreen1Left  = new CScreenInfo(mDisplayLeft);
        mInfoScreen1Left->SetSignal(1, &M400_groundSpeed, "SPD:", "%04.1f");
        mInfoScreen1Left->SetSignal(2, &PDM_pdmVoltAvg,   "SPD:", "%04.1f");
        mInfoScreen1Left->SetSignal(3, &ATCCF_brakeBias,  "SPD:", "%02.0f%%");
        mInfoScreen1Left->SetSignal(4, &PDM_fanLeftPWM,   "SPD:", "%03.0f");

    mInfoScreen1Right = new CScreenInfo(mDisplayRight);
        mInfoScreen1Right->SetSignal(1, &M400_rpm,         "RPM:",  "%.1f", 1000);
        mInfoScreen1Right->SetSignal(2, &M400_oilPressure, "OILP:", "%.1f");
        mInfoScreen1Right->SetSignal(3, &M400_oilTemp,     "OILT:", "%.0f");
        mInfoScreen1Right->SetSignal(4, &M400_engineTemp,  "ENG:",  "%.1f");
}


/**
 * Destructor
 *
 * Destructs all dynamically allocated things
 */
CScreensController::~CScreensController(){

    delete mInfoScreen1Left;

}


/**
 * Initialize
 *
 * Sets the first state for the controller
 */
void CScreensController::Initialize(){

    // set state 1 here
    mInfoScreen1Left->Initialize();
    mInfoScreen1Right->Initialize();

}


/**
 * Update
 *
 * This is called as frequently as possible. If the time is allowable for an update
 * the updates are processed and the screens are written. 
 */
void CScreensController::Update(){

    switch (mState){
        case InfoScreen1:
            mInfoScreen1Left->Update();
            mInfoScreen1Right->Update();
            break;
    }

}

/**
 * Used internally to set the state of the screens. Handles exit and enter funcitonality for the screens.
 *
 * \param state The state to set 
 */
void CScreensController::SetState(ScreenStates state){

}


/**
 * Called when the control button is pressed. This can either change the screen state
 * or disable the current notification screen.
 */
void CScreensController::OnButtonPress(){

}


#endif





