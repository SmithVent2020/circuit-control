#include "BreathData.h"
#include "State.h"

// Initialize singleton instances
OffState* OffState::instance = 0;
InsState* InsState::instance = 0;
InsHoldState* InsHoldState::instance = 0;
SustState* SustState::instance = 0;
ExpState* ExpState::instance = 0;
PeepState* PeepState::instance = 0;
ExpHoldState* ExpHoldState::instance = 0;

/*********************************************************/
/* OffState                                              */
/*********************************************************/

// Return the (singleton) instance of this class
State OffState::get() {
    if (instance == 0) {
        instance = new OffState();
    }
}

// performs actions necessary upon entering a state
void OffState::enter() {
    Serial.println("entering off state"); //@debugging
}

// performs actions necessary upon exiting a state
void OffState::exit() {
    Serial.println("exiting off state"); //@debugging
}

// perform maintenance, and perhaps transition to new state
State OffState::update() {
    Serial.println("updating off state"); //@debugging

    // for now, transition immediately to inspiration state
    return InsState.enter();
}

/*********************************************************/
/* InsState                                              */
/*********************************************************/

// Return the (singleton) instance of this class
State InsState::get() {
    if (instance == 0) {
        instance = new InsState();
    }
}

// performs actions necessary upon entering a state
void InsState::enter() {
    Serial.println("entering insp state"); //@debugging
    beginInspiration();  // close valves, etc.    
}

// performs actions necessary upon exiting a state
void InsState::exit() {
    Serial.println("exiting insp state"); //@debugging    
}

// perform maintenance, and perhaps transition to new state
State InsState::update() {
    Serial.println("updating insp state"); //@debugging
    
    display.updateFlowWave(inspFlowReader.get());                           //update flow waveform on the display
    inspFlowReader.updateVolume();                                          //update the inpsiratory volume counter
    
    bool timeout = (millis() >= targetInspEndTime + INSP_TIME_SENSITIVITY); //calculate when the INSP_STATE should time out

    //transition out of INSP_STATE if either the inspired volume >= tidal volume set by user on the display
    //or if the INSP_STATE  has timed out (according to set BPM, IE ratio and INSP_TIME_SENSITIVITY
    State nextState;
    if (inspFlowReader.getVolume() >= display.volume() || timeout) {  
        // we're leaving this state   
        breath.inspDuration = millis() - cycleTimer; // Record length of inspiration

        // check for alarm conditions
        if (timeout) {
            //If the transition was triggered by a timeout, not the inspiredv volume:
            alarmMgr.activateAlarm(ALARM_TIDAL_LOW);         // trigger the low tidal volume alarm
        } else if(alarmMgr.alarmStatus(ALARM_TIDAL_LOW)) {
            alarmMgr.deactivateAlarm(ALARM_TIDAL_LOW);      //otherwise deactivate a low tidal volume alarm if one is currently on
        }

        // decide which state to enter next
        if (display.inspHold()) { 
            //if user has turned on the inspiratory hold, 
            nextState = InsHoldState.enter();
        } else {
            //if inspiratory hold is not on, transition to EXP_STATE
            nextState = ExpState.enter();
        }
    } else {
        // Stay in INSP_STATE
        nextState = this;
        
        // keep adjusting inspiratory valve until targetInspEndTime is reached
        Serial.println("maintaining breath"); //@cleanup
        inspValve.maintainBreath(cycleTimer);
    }
    return nextState;    
}

/*********************************************************/
/* InsHoldState                                              */
/*********************************************************/

// Return the (singleton) instance of this class
State InsHoldState::get() {
    if (instance == 0) {
        instance = new InsHoldState();
    }
}

// performs actions necessary upon entering a state
void InsHoldState::enter() {
    Serial.println("entering ins hold state"); //@debugging    
    display.resetInspHold();   //reset inspiratory hold value on display
    beginHoldInspiration();    //begin the INSP_HOLD_STATE
}

// performs actions necessary upon exiting a state
void InsHoldState::exit() {
    Serial.println("exiting ins hold state"); //@debugging    
}

// perform maintenance, and perhaps transition to new state
State InsHoldState::update() {
    Serial.println("updating ins hold state"); //@debugging    
    display.updateFlowWave(inspFlowReader.get()); //update flow waveform on display, based on current flow reading
    
    State nextState;
    if (HOLD_INSP_DURATION <= millis() - inspHoldTimer) {
        //switch to EXP_STATE
        nextState = ExpState.enter();

        //if we have not reached the end time for HOLD_INSP_STATE
        inspPressureReader.setPlateau();              //record current inspiratory pressure as the plateau pressure
        if (inspPressureReader.plateau() > PPLAT_MAX) { 
            //check if the plateau pressure is too high and alarm if so
            alarmMgr.activateAlarm(ALARM_PPLAT_HIGH);
            Serial.println("Activating PPlat High Alarm"); //@cleanup
            Serial.print("PPlat ="); Serial.print("\t"); Serial.println(inspPressureReader.plateau()); //@cleanup
        } else if(alarmMgr.alarmStatus(ALARM_PPLAT_HIGH)) {
            //otherwise, deactivate the PPlat alarm if it is currently activated
            alarmMgr.deactivateAlarm(ALARM_PPLAT_HIGH);
        }
    } else {
        nextState = this;
    }
    return nextState;
}

/*********************************************************/
/* SustState                                              */
/*********************************************************/

// Return the (singleton) instance of this class
State SustState::get() {
    if (instance == 0) {
        instance = new SustState();
    }
}

// performs actions necessary upon entering a state
void SustState::enter() {
    Serial.println("entering sust state"); //@debugging    
}

// performs actions necessary upon exiting a state
void SustState::exit() {
    Serial.println("exiting sust state"); //@debugging    
}

// perform maintenance, and perhaps transition to new state
State SustState::update() {
    Serial.println("updating sust state"); //@debugging    
}

/*********************************************************/
/* ExpState                                              */
/*********************************************************/

// Return the (singleton) instance of this class
State ExpState::get() {
    if (instance == 0) {
        instance = new ExpState();
    }
}

// performs actions necessary upon entering a state
void ExpState::enter() {
    Serial.println("entering exp state"); //@debugging    
    beginExpiration();
}

// performs actions necessary upon exiting a state
void ExpState::exit() {
    Serial.println("exiting exp state"); //@debugging    
}

// perform maintenance, and perhaps transition to new state
State ExpState::update() {
    Serial.println("updating exp state"); //@debugging    
    display.updateFlowWave(expFlowReader.get() * -1); //update flow waveform on display using the expiratory flow sensor and flipping reading to negative (out of the patient)
    expFlowReader.updateVolume();                     //update expiratory volume counter
    
    State nextState;
    if (expFlowReader.getVolume() >= targetExpVolume || millis() > targetExpEndTime + EXP_TIME_SENSITIVITY){ 
        //if 80% of inspired volume has been expired, transition to PEEP_PAUSE_STATE 
        nextState = PeepState.enter();
    } else {
        nextState = this;
    }
    return nextState;
}

/*********************************************************/
/* PeepState                                              */
/*********************************************************/

// Return the (singleton) instance of this class
State PeepState::get() {
    if (instance == 0) {
        instance = new PeepState();
    }
}

// performs actions necessary upon entering a state
void PeepState::enter() {
    Serial.println("entering peep state"); //@debugging    
    beginPeepPause();
}

// performs actions necessary upon exiting a state
void PeepState::exit() {
    Serial.println("exiting peep state"); //@debugging    
}

// perform maintenance, and perhaps transition to new state
State PeepState::update() {
    Serial.println("updating peep state"); //@debugging    
    display.updateFlowWave(expFlowReader.get() * -1); //update expiratory flow waveform on display
    expFlowReader.updateVolume();                     //update expiratory volume counter
    
    State nextState;
    if (millis() - peepPauseTimer >= MIN_PEEP_PAUSE) {
        //if the PEEP pause time has run out
        expPressureReader.setPeep(); // record the peep as the current pressure
        
        //transition to HOLD_EXP_STATE
        nextState = ExpHoldState.enter();
    } else {
        nextState = this;
    }
    return nextState;
}

/*********************************************************/
/* ExpHoldState                                              */
/*********************************************************/

// Return the (singleton) instance of this class
State ExpHoldState::get() {
    if (instance == 0) {
        instance = new ExpHoldState();
    }
}

// performs actions necessary upon entering a state
void ExpHoldState::enter() {
    Serial.println("entering exp hold state"); //@debugging    
    beginHoldExpiration();
}

// performs actions necessary upon exiting a state
void ExpHoldState::exit() {
    Serial.println("exiting exp hold state"); //@debugging    
}

// perform maintenance, and perhaps transition to new state
State ExpHoldState::update() {
    Serial.println("updating exp hold state"); //@debugging    
    display.updateFlowWave(expFlowReader.get() * -1); //update flow waveform on display
    expFlowReader.updateVolume();                     //update expiratory flow counter

    // Check if patient triggers inhale by checkin if expiratory pressure has dropped below the pressure sensitivity set by user
    bool patientTriggered = expPressureReader.get() < expPressureReader.peep() - display.sensitivity();
    bool timeout = (millis()  > targetCycleEndTime); //check if the expiratory hold timer has run out

    State nextState;
    if (timeout) { //@debugging add back with real patient: patientTriggered || @cleanup: add this back
        //if the patient has triggered a breath, or the timer has run out transition to INSP_STATE
        nextState = InspState.enter();
    } else {
        nextState = this;
    }
    return nextState;
}
