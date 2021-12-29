#ifndef CSCREENCONTROLLER_HPP
#define CSCREENCONTROLLER_HPP


class CScreensController {

    public:

        /// the different states that the screens can be in. Please note that states should include
        /// transitionary states, such as ScreenBegin before Screen or whatever
        enum ScreenStates { InfoScreen1, InfoScreen2, InfoScreen3, Titan };

        /**
         * Gets the current state of the screen
         * @returns the screen state
         */
        int GetState() {return mState;}

        void Update();
        void OnButtonPress();

    private:

        void SetState(ScreenStates state);

        ScreenStates mState = InfoScreen1;  ///< screen states
        ScreenStates mStatePrev = InfoScreen1;  ///< Used to return to previous screen states after temporary states
        unsigned long mStateTime = 0;  ///< The time in milliseconds that the current state started
        int mMaxFrameFrame = 12;  ///< the maximum framerate allowable to update the displays in frames per second

};


/**
 * Update
 *
 * This is called as frequently as possible. If the time is allowable for an update
 * the updates are processed and the screens are written. 
 */
void CScreensController::Update(){

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





