#ifndef CSCREENCONTROLLER_HPP
#define CSCREENCONTROLLER_HPP

#include "ScreenInfo.hpp"
#include "ScreenStartupAnim.hpp"
#include "ScreenMessage.hpp"
#include "ScreenNumber.hpp"

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
        enum ScreenStates { /*StartupLeft, StartupRight,*/ GearInfo, InfoScreen1, InfoScreen2, Notification, Titan };

        /**
         * Gets the current state of the screen
         * @returns the screen state
         */
        int GetState() {return mState;}

        void Update(unsigned long &elapsed);
        void Initialize();
        void OnButtonPress();
        void OnMessage(String message);


    private:

        ILI9341_t3n &mDisplayLeft;
        ILI9341_t3n &mDisplayRight;

        void SetState(ScreenStates state);

        ScreenStates mState;  ///< screen states
        ScreenStates mStatePrev = GearInfo;  ///< Used to return to previous screen states after temporary states
        unsigned long mStateStartTime = 0;  ///< The time in milliseconds that the current state started

        unsigned long mMessageDisplayDuration = 5000;  ///< The time in milliseconds to display a message when recieved

        /* Screen definitions */
        CScreenStartupAnim *mStartupScreenLeft  = nullptr;
        CScreenStartupAnim *mStartupScreenRight = nullptr;

        CScreenInfo *mInfoScreen1Left  = nullptr;
        CScreenInfo *mInfoScreen1Right = nullptr;

        CScreenInfo *mInfoScreen2Left  = nullptr;
        CScreenInfo *mInfoScreen2Right = nullptr;

        CScreenMessage *mMessageScreen = nullptr;

        CScreenNumber *mGearScreen = nullptr;

};


/**
 * Constructor
 *
 * Constructs screens for all of the states and then sets the initial state
 */
CScreensController::CScreensController(ILI9341_t3n &left, ILI9341_t3n &right) : 
                                       mDisplayLeft(left), mDisplayRight(right){

    /* gear screen */
    mGearScreen = new CScreenNumber(mDisplayLeft, M400_gear, "GEAR:");
    
    /* Info screen 1 */
    mInfoScreen1Left  = new CScreenInfo(mDisplayLeft);
        mInfoScreen1Left->SetSignal(1, &M400_groundSpeed,     "SPD:", "%4.1f");
        mInfoScreen1Left->SetSignal(2, &PDM_pdmVoltAvg,       "BAT:", "%4.1f");
        mInfoScreen1Left->SetSignal(3, &ATCCF_brakeBias,      "BIAS:", "%2.0f%%");
        mInfoScreen1Left->SetSignal(4, &PDM_fanLeftDutyCycle, "FANS:", "%3.0f");

    mInfoScreen1Right = new CScreenInfo(mDisplayRight);
        mInfoScreen1Right->SetSignal(1, &M400_rpm,         "RPM:",   "%5.1f", 1000);
        mInfoScreen1Right->SetSignal(2, &M400_oilPressure, "OILP:",  "%4.1f");
        mInfoScreen1Right->SetSignal(3, &M400_oilTemp,     "OILT:",  "%4.0f");
        mInfoScreen1Right->SetSignal(4, &C50_tcSet,        "TCSET:", "%3.0f");

    /* Info screen 2 */
    mInfoScreen2Left  = new CScreenInfo(mDisplayLeft);
        mInfoScreen2Left->SetSignal(1, &ATCCF_rotorTempFL, "FL:", "%5.1f");
        mInfoScreen2Left->SetSignal(2, &ATCCF_rotorTempFR, "FR:", "%5.1f");
        mInfoScreen2Left->SetSignal(3, &ATCCR_rotorTempRL, "RL:", "%5.1f");
        mInfoScreen2Left->SetSignal(4, &ATCCR_rotorTempRR, "RR:", "%5.1f");
    // keep the same screen on the right side
    mInfoScreen2Right = mInfoScreen1Right;


    /* message screen */
    mMessageScreen = new CScreenMessage(mDisplayLeft);




}


/**
 * Destructor
 *
 * Destructs all dynamically allocated things
 */
CScreensController::~CScreensController(){

    /* gear */
    delete mGearScreen;

    /* Screen 1 */
    delete mInfoScreen1Left;
    delete mInfoScreen1Right;

    /* Screen 2 */
    delete mInfoScreen2Left;

    /* message display */
    delete mMessageScreen;
    

}


/**
 * Initialize
 *
 * Sets the first state for the controller
 */
void CScreensController::Initialize(){

    SetState(GearInfo);

}


/**
 * Update
 *
 * This is called as frequently as possible. If the time is allowable for an update
 * the updates are processed and the screens are written. 
 * \param elapsed The time in milliseconds since the update method was last called
 */
void CScreensController::Update(unsigned long &elapsed){

    switch (mState){
        // case StartupLeft:
        //     mStartupScreenLeft->Update(elapsed);
        //     if (mStartupScreenLeft->IsCompleted()){
        //         SetState(StartupRight);
        //     }
        //     break;


        // case StartupRight:
        //     mStartupScreenRight->Update(elapsed); 
        //     if (mStartupScreenRight->IsCompleted()){
        //         SetState(InfoScreen1);
        //     } 
        //     break;
        case GearInfo:
            mGearScreen->Update(elapsed);
            mInfoScreen1Right->Update(elapsed);
            break;

        case InfoScreen1:
            mInfoScreen1Left->Update(elapsed);
            mInfoScreen1Right->Update(elapsed);
            break;

        case InfoScreen2:
            mInfoScreen2Left->Update(elapsed);
            mInfoScreen2Right->Update(elapsed);
            break;

        case Notification:
            mMessageScreen->Update(elapsed);

            // update the last state's right screen, too!
            switch (mStatePrev){
                case GearInfo: // fall through
                case InfoScreen1:
                    mInfoScreen1Right->Update(elapsed);
                    break;
                case InfoScreen2:
                    mInfoScreen2Right->Update(elapsed);
                    break;
                case Notification:
                    break;
                case Titan:
                    break;
            }

            // if the state is complete, set a new state
            if (millis() - mStateStartTime > mMessageDisplayDuration){
                SetState(mStatePrev);
            }

            break;

        case Titan:
            break;
    }

}

/**
 * Used internally to set the state of the screens. Handles exit and enter funcitonality for the screens.
 *
 * \param state The state to set 
 */
void CScreensController::SetState(ScreenStates state){

    /*
     * State exiting actions
     */
    switch (mState){
        // case StartupLeft:
        //     break;
        // case StartupRight:
        //     break;
        case GearInfo:
            break;
        case InfoScreen1:
            break;
        case InfoScreen2:
            break;
        case Notification:
            // in order to prevent getting stuck in the Notification state,
            // set the state to the previous state. That way, it's like
            // the notifaction state never existed, since after the next 10ish lines
            // of code, the previous state will be two states ago, and the current state
            // will be the last previous state... confusing but yeah
            mState = mStatePrev;
            break;
        case Titan:
            break;
    }

    /*
     * Set the state
     */
    mStatePrev = mState;  // move current to previous state
    mState = state;       // move new input state to current state
    mStateStartTime = millis();  // update state start time


    /*
     * State entering actions
     */
    switch (mState){
        // case StartupLeft:
        //     mStartupScreenLeft->Initialize();
        //     break;
        // case StartupRight:
        //     mStartupScreenRight->Initialize();
        //     break;
        case GearInfo:
            mGearScreen->Initialize();
            mInfoScreen1Right->Initialize();
            break;
        case InfoScreen1:
            mInfoScreen1Left->Initialize();
            mInfoScreen1Right->Initialize();
            break;
        case InfoScreen2:
            mInfoScreen2Left->Initialize();
            mInfoScreen2Right->Initialize();
            break;
        case Notification:
            mMessageScreen->Initialize();
            break;
        case Titan:
            break;
    }
    
}


/**
 * Called when the control button is pressed. This can either change the screen state
 * or disable the current notification screen.
 */
void CScreensController::OnButtonPress(){

    switch (mState){
        // case StartupLeft:
        //     SetState(InfoScreen1);
        //     break;
        // case StartupRight:
        //     SetState(InfoScreen1);
        //     break;
        case GearInfo:
            SetState(InfoScreen1);
            break;
        case InfoScreen1:
            SetState(InfoScreen2);
            break;
        case InfoScreen2:
            SetState(GearInfo);
            break;
        case Notification:
            SetState(mStatePrev);
        case Titan:
            break;
    }
}



/**
 * What to do when a message is recieved
 * \param message The message to display
 */
void CScreensController::OnMessage(String message){

    if (mMessageScreen){
        mMessageScreen->SetMessage(message);
    }
    SetState(Notification);

}


#endif





