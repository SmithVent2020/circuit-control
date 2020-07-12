#include "BreathData.h"
#include "State.h"
#include "Constants.h"
#include "Display.h"
#include "Flow.h"
#include "Pressure.h"
#include "Valve.h"
#include "ProportionalValve.h"
#include "AlarmManager.h"
#include "BreathData.h"
#include "State.h"

// Initialize breath data
BreathData State::breath;

// Initialize singleton instances
OffState* OffState::instance = 0;
InsStateVC* InsStateVC::instance = 0;
InsHoldStateVC* InsHoldStateVC::instance = 0;
ExpStateVC* ExpStateVC::instance = 0;
PeepStateVC* PeepStateVC::instance = 0;
ExpHoldStateVC* ExpHoldStateVC::instance = 0;

/*********************************************************/
/* OffState                                              */
/*********************************************************/

static State* OffState::enter() {
    if (instance==0) {
        instance = new OffState();  // one-time setup
    } else {
        // if we enter this state other than at the beginning, sound an alarm        
        alarmMgr.activateAlarm(ALARM_SHUTDOWN);
    }

    // valve settings
    inspValve.endBreath(); // close the inspiratory valve
    expValve.open();       // keep expiratory valve open for safety (also does not use as much power)

    return instance;
}

// perform maintenance, and perhaps transition to new state
State* OffState::update() {
    // for now, transition immediately to inspiration state
    return InsStateVC::enter();
}

/*********************************************************/
/* InsStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* InsStateVC::enter() {
    if (instance==0) instance = new InsStateVC();  // one-time setup

    breath.beginInspiration();  // close valves, etc.
    return instance;
}

// perform maintenance, and perhaps transition to new state
State* InsStateVC::update() {   
    display.updateFlowWave(inspFlowReader.get());  //update flow waveform on the display
    inspFlowReader.updateVolume();                 //update the inpsiratory volume counter
    
    bool timeout = (millis() >= breath.targetInspEndTime + INSP_TIME_SENSITIVITY); //calculate when the INSP_STATE should time out

    // check for state transition
    // transition out of INSP_STATE if either the inspired volume >= tidal volume set by user on the display
    // or if the INSP_STATE  has timed out (according to set BPM, IE ratio and INSP_TIME_SENSITIVITY
    State* nextState;
    if (inspFlowReader.getVolume() >= display.volume() || timeout) {  
        // transition to next state
        breath.inspDuration = millis() - breath.cycleTimer; // Record length of inspiration

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
        // continue in current state
        nextState = this;
        
        // adjust inspiratory valve as necessary to maintain targets
        inspValve.maintainBreath(breath.cycleTimer);
    }
    return nextState;    
}

/*********************************************************/
/* InsHoldStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* InsHoldStateVC::enter() {
    if (instance==0) instance = new InsHoldStateVC();   // one-time setup

    breath.beginHoldInspiration();    //begin the INSP_HOLD_STATE
    return instance;
}

// perform maintenance, and perhaps transition to new state
State* InsHoldStateVC::update() {
    display.updateFlowWave(inspFlowReader.get()); //update flow waveform on display, based on current flow reading
    
    // check for state transition
    State* nextState;
    if (HOLD_INSP_DURATION <= millis() - breath.inspHoldTimer) {
        // transition to next state
        nextState = ExpStateVC::enter();

        //if we have not reached the end time for HOLD_INSP_STATE
        inspPressureReader.setPlateau();              //record current inspiratory pressure as the plateau pressure
        // check plateau pressure range and alarm if abnormal  
        if (inspPressureReader.plateau() > PPLAT_MAX) { 
         alarmMgr.activateAlarm(ALARM_PPLAT_HIGH);
        } else if (alarmMgr.alarmStatus(ALARM_PPLAT_HIGH)) {
          alarmMgr.deactivateAlarm(ALARM_PPLAT_HIGH);
        }
    } else {
        // continue in current state
        nextState = this;
    }
    return nextState;
}

/*********************************************************/
/* ExpStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* ExpStateVC::enter() {
    if (instance==0) instance = new ExpStateVC();  // one-time setup

    breath.beginExpiration();
    return instance;
}

// perform maintenance, and perhaps transition to new state
State* ExpStateVC::update() {
    // To update flow graph, flip sign of expiratory flow sensor to show flow out of lungs 
    display.updateFlowWave(expFlowReader.get() * -1);
    expFlowReader.updateVolume(); 
    
    // check for state transition
    State* nextState;
    if (expFlowReader.getVolume() >= breath.targetExpVolume || millis() > breath.targetExpEndTime + EXP_TIME_SENSITIVITY){ 
        // transition to next state
        nextState = PeepStateVC::enter();
    } else {
        // continue in current state
        nextState = this;
    }
    return nextState;
}

/*********************************************************/
/* PeepStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* PeepStateVC::enter() {
    if (instance==0) instance = new PeepStateVC();  // one-time setup

    Serial.println("entering peep state"); //@debugging    
    breath.beginPeepPause();
    return instance;
}

// perform maintenance, and perhaps transition to new state
State* PeepStateVC::update() {
    // To update flow graph, flip sign of expiratory flow sensor to show flow out of lungs 
    display.updateFlowWave(expFlowReader.get() * -1);  // update expiratory flow waveform on display
    expFlowReader.updateVolume();                      // update expiratory volume counter
    
    // check for state transition
    State* nextState;
    if (millis() - breath.peepPauseTimer >= MIN_PEEP_PAUSE) {
        // transition to next state
        expPressureReader.setPeep();  // record the peep as the current pressure
        nextState = ExpHoldStateVC::enter();
    } else {
        // continue in current state
        nextState = this;
    }
    return nextState;
}

/*********************************************************/
/* ExpHoldStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
static State* ExpHoldStateVC::enter() {
    if (instance==0) instance = new ExpHoldStateVC();  // one-time setup

    breath.beginHoldExpiration();
    return instance;
}

// perform maintenance, and perhaps transition to new state
State* ExpHoldStateVC::update() {
    // To update flow graph, flip sign of expiratory flow sensor to show flow out of lungs 
    display.updateFlowWave(expFlowReader.get() * -1);  // update flow waveform on display
    expFlowReader.updateVolume();                      // update expiratory flow counter

    // Check if patient triggers inhale or state timed out 
    bool patientTriggered = expPressureReader.get() < expPressureReader.peep() - display.sensitivity();
    bool timeout = (millis()  > breath.targetCycleEndTime); //check if the expiratory hold timer has run out

    // check for state transition
    State* nextState;
    if (patientTriggered || timeout) { 
        // transition to next state
        if (display.isTurnedOff()) {
            // the user has turned off ventilation
            nextState = OffState::enter();
        } else {
            // enter breath cycle as usual
            nextState = InsStateVC::enter();
        }        
    } else {
        // continue in current state
        nextState = this;
    }
    return nextState;
}
