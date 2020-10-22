#include "BreathData.h"
#include "StateMachine.h"
#include "Constants.h"
#include "Display.h"
#include "Flow.h"
#include "Pressure.h"
#include "Valve.h"
#include "ProportionalValve.h"
#include "AlarmManager.h"
#include "BreathData.h"


// singleton instances of each state class
StateMachine::OffState StateMachine::OffState::OffState_singleton;
StateMachine::InsStateVC StateMachine::InsStateVC::InsStateVC_singleton;
StateMachine::InsHoldStateVC StateMachine::InsHoldStateVC::InsHoldStateVC_singleton;
StateMachine::ExpStateVC StateMachine::ExpStateVC::ExpStateVC_singleton;
StateMachine::PeepStateVC StateMachine::PeepStateVC::PeepStateVC_singleton;
StateMachine::ExpHoldStateVC StateMachine::ExpHoldStateVC::ExpHoldStateVC_singleton;
    
// Constructor initializes all the singleton state instances
// and initializes state to OffState.
StateMachine::StateMachine() {
             
    currentState = &OffState::OffState_singleton;    // we don't call enter() to bypass alarm
    
    // valve settings
    inspValve.endBreath(); // close the inspiratory valve
    expValve.open();       // keep expiratory valve open for safety (also does not use as much power)
}

// Holds data about the breath cycle.
BreathData StateMachine::breath;

// Calls the update for the current state, and enters a new state if necessary
void StateMachine::update() {
    State* nextState = currentState->update();
    if (nextState != currentState) {
        nextState->enter();
        currentState = nextState;
    }
}

/*********************************************************/
/* OffState                                              */
/*********************************************************/

void StateMachine::OffState::enter() {
    // if we enter this state other than at the beginning, sound an alarm        
    alarmMgr.activateAlarm(ALARM_SHUTDOWN);

    // valve settings
    inspValve.endBreath(); // close the inspiratory valve
    expValve.open();       // keep expiratory valve open for safety (also does not use as much power)
}

// perform maintenance, and perhaps transition to new state
StateMachine::State* StateMachine::OffState::update() {
    // for now, transition immediately to inspiration state
    return &InsStateVC::InsStateVC_singleton;
}

/*********************************************************/
/* InsStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
void StateMachine::InsStateVC::enter() {
    breath.beginInspiration();  // close valves, etc.

    // begin PID control based on desired flow and reset tidal volume
    inspValve.beginBreath(breath.desiredInspFlow); 
    inspFlowReader.resetVolume();   
}

// perform maintenance, and perhaps transition to new state
StateMachine::State* StateMachine::InsStateVC::update() {   
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
            nextState = (State*)&InsHoldStateVC::InsHoldStateVC_singleton;
        } else {
            //if inspiratory hold is not on, transition to EXP_STATE
            nextState = (State*)&ExpStateVC::ExpStateVC_singleton;
        }
    } else {
        // continue in current state
        nextState = (State*)&InsStateVC::InsStateVC_singleton;
        
        // adjust inspiratory valve as necessary to maintain targets
        inspValve.maintainBreath(breath.cycleTimer);
    }
    return nextState;    
}

/*********************************************************/
/* InsHoldStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
void StateMachine::InsHoldStateVC::enter() {
    breath.beginHoldInspiration();    //begin the INSP_HOLD_STATE
}

// perform maintenance, and perhaps transition to new state
StateMachine::State* StateMachine::InsHoldStateVC::update() {
    display.updateFlowWave(inspFlowReader.get()); //update flow waveform on display, based on current flow reading
    
    // check for state transition
    State* nextState;
    if (HOLD_INSP_DURATION <= millis() - breath.inspHoldTimer) {
        // transition to next state
        nextState = (State*)&ExpStateVC::ExpStateVC_singleton;

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
        nextState = (State*)&InsHoldStateVC::InsHoldStateVC_singleton;
    }
    return nextState;
}

/*********************************************************/
/* ExpStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
void StateMachine::ExpStateVC::enter() {
    breath.beginExpiration();
}

// perform maintenance, and perhaps transition to new state
StateMachine::State* StateMachine::ExpStateVC::update() {
    // To update flow graph, flip sign of expiratory flow sensor to show flow out of lungs 
    display.updateFlowWave(expFlowReader.get() * -1);
    expFlowReader.updateVolume(); 
    
    // check for state transition
    State* nextState;
    if (expFlowReader.getVolume() >= breath.targetExpVolume || millis() > breath.targetExpEndTime + EXP_TIME_SENSITIVITY){ 
        // transition to next state
        nextState = (State*)&PeepStateVC::PeepStateVC_singleton;
    } else {
        // continue in current state
        nextState = (State*)&ExpStateVC::ExpStateVC_singleton;
    }
    return nextState;
}

/*********************************************************/
/* PeepStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
void StateMachine::PeepStateVC::enter() {
    breath.beginPeepPause();
}

// perform maintenance, and perhaps transition to new state
StateMachine::State* StateMachine::PeepStateVC::update() {
    // To update flow graph, flip sign of expiratory flow sensor to show flow out of lungs 
    display.updateFlowWave(expFlowReader.get() * -1);  // update expiratory flow waveform on display
    expFlowReader.updateVolume();                      // update expiratory volume counter
    
    // check for state transition
    State* nextState;
    if (millis() - breath.peepPauseTimer >= MIN_PEEP_PAUSE) {
        // transition to next state
        expPressureReader.setPeep();  // record the peep as the current pressure
        nextState = (State*)&ExpHoldStateVC::ExpHoldStateVC_singleton;
    } else {
        // continue in current state
        nextState = (State*)&PeepStateVC::PeepStateVC_singleton;
    }
    return nextState;
}

/*********************************************************/
/* ExpHoldStateVC                                              */
/*********************************************************/

// performs actions necessary upon entering a state
void StateMachine::ExpHoldStateVC::enter() {
    breath.beginHoldExpiration();
}

// perform maintenance, and perhaps transition to new state
StateMachine::State* StateMachine::ExpHoldStateVC::update() {
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
            nextState = (State*)&OffState::OffState_singleton;
        } else {
            // enter breath cycle as usual
            nextState = (State*)&InsStateVC::InsStateVC_singleton;
        }        
    } else {
        // continue in current state
        nextState = (State*)&ExpHoldStateVC::ExpHoldStateVC_singleton;
    }
    return nextState;
}
