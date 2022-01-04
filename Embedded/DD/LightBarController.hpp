#ifndef CLIGHTBARCONTROLLER_HPP
#define CLIGHTBARCONTROLLER_HPP

#include "LightBarBlink.hpp"
#include "LightBarRPM.hpp"
#include "LightBarStartupSides1.hpp"
#include "CAN/CAN1.hpp"
#include "CAN/CAN2.hpp"

class CLightBarController {

    public:

        CLightBarController(Adafruit_NeoPixel &left, Adafruit_NeoPixel &top, Adafruit_NeoPixel &right);

        /** Destructor */
        ~CLightBarController();

        /** Copy constructor disabled */
        CLightBarController(const CLightBarController &) = delete;
        /** Assignment operator disabled */
        void operator=(const CLightBarController &) = delete;

        /// the different states that the LEDs can be in. Please note that states should include
        /// transitionary states
        enum LEDStates { Normal, Notification };

        /**
         * Gets the current state of the leds
         * @returns the led state
         */
        int GetState() {return mState;}

        void Update(unsigned long &elapsed);
        void Initialize();
        void OnButtonPress();

    private:

        Adafruit_NeoPixel &mLeftLEDs;
        Adafruit_NeoPixel &mTopLEDs;
        Adafruit_NeoPixel &mRightLEDs;

        void SetState(LEDStates state);

        LEDStates mState;  ///< screen states
        unsigned long mStateStartTime = 0;  ///< The time in milliseconds that the current state started



        CLightBarRPM *mRPMBarGearN  = nullptr;
        CLightBarRPM *mRPMBarGear1  = nullptr;
        CLightBarRPM *mRPMBarGear2  = nullptr;
        CLightBarRPM *mRPMBarGear3  = nullptr;
        CLightBarRPM *mRPMBarGear4  = nullptr;
        CLightBarRPM *mRPMBarGear5  = nullptr;
        CLightBarBlink *mTCBarLeft  = nullptr;
        CLightBarBlink *mTCBarRight = nullptr;
        
        CLightBarBlink *mNoficationBarLeft  = nullptr;
        CLightBarBlink *mNoficationBarRight = nullptr;


};


/**
 * Constructor
 *
 * Constructs screens for all of the states and then sets the initial state
 */
CLightBarController::CLightBarController(Adafruit_NeoPixel &left, Adafruit_NeoPixel &top, Adafruit_NeoPixel &right) : 
                                         mLeftLEDs(left), mTopLEDs(top), mRightLEDs(right){


    /* Normal Operation */
    int maxRPM = 13000;
    int minRPM = 3000;
    int downshiftRPM = -1;
    mRPMBarGearN = new CLightBarRPM(mTopLEDs, 0, mTopLEDs.numPixels(), M400_rpm, minRPM, maxRPM, downshiftRPM);

    maxRPM = 13224;
    minRPM = maxRPM - 4000;
    downshiftRPM = -1;
    mRPMBarGear1 = new CLightBarRPM(mTopLEDs, 0, mTopLEDs.numPixels(), M400_rpm, minRPM, maxRPM, downshiftRPM);

    maxRPM = 11900;
    minRPM = maxRPM - 4000;
    downshiftRPM = 9362;
    mRPMBarGear2 = new CLightBarRPM(mTopLEDs, 0, mTopLEDs.numPixels(), M400_rpm, minRPM, maxRPM, downshiftRPM);

    maxRPM = 12000;
    minRPM = 3000;
    downshiftRPM = 9703;
    mRPMBarGear3 = new CLightBarRPM(mTopLEDs, 0, mTopLEDs.numPixels(), M400_rpm, minRPM, maxRPM, downshiftRPM);

    maxRPM = 12000;
    minRPM = maxRPM - 4000;
    downshiftRPM = 9783;
    mRPMBarGear4 = new CLightBarRPM(mTopLEDs, 0, mTopLEDs.numPixels(), M400_rpm, minRPM, maxRPM, downshiftRPM);

    maxRPM = 14000;
    minRPM = maxRPM - 4000;
    downshiftRPM = 9855;
    mRPMBarGear5 = new CLightBarRPM(mTopLEDs, 0, mTopLEDs.numPixels(), M400_rpm, minRPM, maxRPM, downshiftRPM);
    
    mTCBarLeft  = new CLightBarBlink(mLeftLEDs, 1, 3, M400_tcPowerReduction, 1);
    mTCBarLeft->SetColor(255, 0, 255);

    mTCBarRight = new CLightBarBlink(mRightLEDs, 1, 3, M400_tcPowerReduction, 1);
    mTCBarRight->SetColor(255, 0, 255);
    

}


/**
 * Destructor
 *
 * Destructs all dynamically allocated things
 */
CLightBarController::~CLightBarController(){

    delete mRPMBarGearN;
    delete mRPMBarGear1;
    delete mRPMBarGear2;
    delete mRPMBarGear3;
    delete mRPMBarGear4;
    delete mRPMBarGear5;
    delete mTCBarLeft;
    delete mTCBarRight;

}


/**
 * Initialize
 *
 * Sets the first state for the controller
 */
void CLightBarController::Initialize(){

    SetState(Normal);

}


/**
 * Update
 *
 * This is called as frequently as possible. If the time is allowable for an update
 * the updates are processed and the screens are written. 
 * \param elapsed The time in milliseconds since the update method was last called
 */
void CLightBarController::Update(unsigned long &elapsed){

    switch (mState){
        case Normal:
            mTCBarLeft->Update(elapsed);
            mTCBarRight->Update(elapsed);
            mRPMBarGearN->Update(elapsed);
            break;
        case Notification:
            break;
    }

}

/**
 * Used internally to set the state of the screens. Handles exit and enter funcitonality for the screens.
 *
 * \param state The state to set 
 */
void CLightBarController::SetState(LEDStates state){

    /*
     * State exiting actions
     */
    switch (mState){
        case Normal:
            break;
        case Notification:
            break;
    }

    /*
     * Set the state
     */
    mState = state;       // move new input state to current state
    mStateStartTime = millis();  // update state start time


    /*
     * State entering actions
     */
    switch (mState){
        case Normal:
            mTCBarLeft->Initialize();
            mTCBarRight->Initialize();
            mRPMBarGearN->Initialize();
            break;
        case Notification:
            break;
    }
    
}


/**
 * Called when the control button is pressed. This can either change the screen state
 * or disable the current notification screen.
 */
void CLightBarController::OnButtonPress(){

    switch (mState){
        case Normal:
            break;
        case Notification:
            SetState(Normal);
            break;
    }
}


#endif





