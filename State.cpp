#include "BreathData.h"
#include "State.h"
#include "Constants.h"
#include "Display.h"
#include "Flow.h"
#include "Pressure.h"
#include "Valve.h"
#include "ProportionalValve.h"
#include "AlarmManager.h"
#include "State.h"

// Initialize singleton instances
OffState* OffState::instance = 0;
InsStateVC* InsStateVC::instance = 0;
InsHoldStateVC* InsHoldStateVC::instance = 0;
SustStateVC* SustStateVC::instance = 0;
ExpStateVC* ExpStateVC::instance = 0;
PeepStateVC* PeepStateVC::instance = 0;
ExpHoldStateVC* ExpHoldStateVC::instance = 0;

BreathData* State::breath;

/*********************************************************/
/* OffState                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* OffState::begin(BreathData b) {
    if (instance==0) { 
        instance = new OffState(); 
    }
    Serial.println("entering off state"); //@debugging
    breath = &b;
    return instance;
}

static State* OffState::enter() {
    // if we enter this state other than at the beginning, sound an alarm        
    alarmMgr.activateAlarm(ALARM_SHUTDOWN);
}

// perform maintenance, and perhaps transition to new state
State* OffState::update() {
    Serial.println("updating off state"); //@debugging

    // for now, transition immediately to inspiration state
    return InsStateVC::enter();
}

/*********************************************************/
/* InsStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* InsStateVC::enter() {
    if (instance==0) { 
        instance = new InsStateVC(); 
    }

    State* nextState;
    //check if the user has turned of ventilation
    if (display.isTurnedOff()) {
        nextState = OffState::enter();
    } else {
        Serial.println("entering insp state"); //@debugging
        breath->beginInspiration();  // close valves, etc.
        nextState = instance;
    }
    return nextState;
}

// perform maintenance, and perhaps transition to new state
State* InsStateVC::update() {
    Serial.println("updating insp state"); //@debugging
    
    display.updateFlowWave(inspFlowReader.get());                           //update flow waveform on the display
    inspFlowReader.updateVolume();                                          //update the inpsiratory volume counter
    
    bool timeout = (millis() >= breath->targetInspEndTime + INSP_TIME_SENSITIVITY); //calculate when the INSP_STATE should time out

    //transition out of INSP_STATE if either the inspired volume >= tidal volume set by user on the display
    //or if the INSP_STATE  has timed out (according to set BPM, IE ratio and INSP_TIME_SENSITIVITY
    State* nextState;
    if (inspFlowReader.getVolume() >= display.volume() || timeout) {  
        // we're leaving this state   
        breath->inspDuration = millis() - breath->cycleTimer; // Record length of inspiration

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
            nextState = InsHoldStateVC::enter();
        } else {
            //if inspiratory hold is not on, transition to EXP_STATE
            nextState = ExpStateVC::enter();
        }
    } else {
        // Stay in INSP_STATE
        nextState = this;
        
        // keep adjusting inspiratory valve until targetInspEndTime is reached
        Serial.println("maintaining breath"); //@cleanup
        inspValve.maintainBreath(breath->cycleTimer);
    }
    return nextState;    
}

/*********************************************************/
/* InsHoldStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* InsHoldStateVC::enter() {
    if (instance==0) { 
        instance = new InsHoldStateVC(); 
    }
    Serial.println("entering ins hold state"); //@debugging    
    display.resetInspHold();   //reset inspiratory hold value on display
    breath->beginHoldInspiration();    //begin the INSP_HOLD_STATE
    return instance;
}

// perform maintenance, and perhaps transition to new state
State* InsHoldStateVC::update() {
    Serial.println("updating ins hold state"); //@debugging    
    display.updateFlowWave(inspFlowReader.get()); //update flow waveform on display, based on current flow reading
    
    State* nextState;
    if (HOLD_INSP_DURATION <= millis() - breath->inspHoldTimer) {
        //switch to EXP_STATE
        nextState = ExpStateVC::enter();

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
/* SustStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* SustStateVC::enter() {
    if (instance==0) { 
        instance = new SustStateVC(); 
    }
    Serial.println("entering sust state"); //@debugging    
    return instance;
}

// perform maintenance, and perhaps transition to new state
State* SustStateVC::update() {
    Serial.println("updating sust state"); //@debugging    
}

/*********************************************************/
/* ExpStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* ExpStateVC::enter() {
    if (instance==0) { 
        instance = new ExpStateVC(); 
    }
    Serial.println("entering exp state"); //@debugging    
    breath->beginExpiration();
    return instance;
}

// perform maintenance, and perhaps transition to new state
State* ExpStateVC::update() {
    Serial.println("updating exp state"); //@debugging    
    display.updateFlowWave(expFlowReader.get() * -1); //update flow waveform on display using the expiratory flow sensor and flipping reading to negative (out of the patient)
    expFlowReader.updateVolume();                     //update expiratory volume counter
    
    State* nextState;
    if (expFlowReader.getVolume() >= breath->targetExpVolume || millis() > breath->targetExpEndTime + EXP_TIME_SENSITIVITY){ 
        //if 80% of inspired volume has been expired, transition to PEEP_PAUSE_STATE 
        nextState = PeepStateVC::enter();
    } else {
        nextState = this;
    }
    return nextState;
}

/*********************************************************/
/* PeepStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* PeepStateVC::enter() {
    if (instance==0) { 
        instance = new PeepStateVC(); 
    }
    Serial.println("entering peep state"); //@debugging    
    breath->beginPeepPause();
    return instance;
}

// perform maintenance, and perhaps transition to new state
State* PeepStateVC::update() {
    Serial.println("updating peep state"); //@debugging    
    display.updateFlowWave(expFlowReader.get() * -1); //update expiratory flow waveform on display
    expFlowReader.updateVolume();                     //update expiratory volume counter
    
    State* nextState;
    if (millis() - breath->peepPauseTimer >= MIN_PEEP_PAUSE) {
        //if the PEEP pause time has run out
        expPressureReader.setPeep(); // record the peep as the current pressure
        
        //transition to HOLD_EXP_STATE
        nextState = ExpHoldStateVC::enter();
    } else {
        nextState = this;
    }
    return nextState;
}

/*********************************************************/
/* ExpHoldStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* ExpHoldStateVC::enter() {
    if (instance==0) { 
        instance = new ExpHoldStateVC(); 
    }
    Serial.println("entering exp hold state"); //@debugging    
    breath->beginHoldExpiration();
    return instance;
}

// perform maintenance, and perhaps transition to new state
State* ExpHoldStateVC::update() {
    Serial.println("updating exp hold state"); //@debugging    
    display.updateFlowWave(expFlowReader.get() * -1); //update flow waveform on display
    expFlowReader.updateVolume();                     //update expiratory flow counter

    // Check if patient triggers inhale by checkin if expiratory pressure has dropped below the pressure sensitivity set by user
    bool patientTriggered = expPressureReader.get() < expPressureReader.peep() - display.sensitivity();
    bool timeout = (millis()  > breath->targetCycleEndTime); //check if the expiratory hold timer has run out

    State* nextState;
    if (timeout) { //@debugging add back with real patient: patientTriggered || @cleanup: add this back
        //if the patient has triggered a breath, or the timer has run out transition to INSP_STATE
        nextState = InsStateVC::enter();
    } else {
        nextState = this;
    }
    return nextState;
}