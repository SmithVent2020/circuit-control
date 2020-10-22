#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "BreathData.h"

/*
 *  Class that handles all state transitions
 */
class StateMachine {
public:
    StateMachine();                  // constructor sets up state machine

    static BreathData breath;        // holds shared information about the breath cycle
        
    void update();                   // performs the update for the current state

    /*
    *  Parent class for all instantiated state classes
    */
    class State {
    public:
        virtual State* update() = 0;    // perform maintenance, and perhaps transition to new state
        virtual void enter() = 0;       // performs actions necessary upon entering the state
    };

private:
    State* currentState;  // pointer to the current state

    /*
    *  Specific states used by the ventilator control.
    *  The state machine keeps a singleton instance of each.
    */

    // Standby mode.  This is the initial state.
    class OffState : public State {
    public:
        static OffState OffState_singleton;
        
        void enter();                 // performs actions necessary upon entering the state
        State* update();              // perform maintenance, and perhaps transition to next state
    private: 
        OffState() {};                // default constructor
    };

    // Inspiration begins a breath cycle
    class InsStateVC : public State {
    public:
        static InsStateVC InsStateVC_singleton;
        
        void enter();                   // performs actions necessary upon entering the state
        State* update();                // perform maintenance, and perhaps transition to next state
    private: 
        InsStateVC() {};                // default constructor
    };

    // USed in volume control mode during inspiration.
    // Physician may order an inspiratory hold to measure plateau pressure
    class InsHoldStateVC : public State {
    public:
        static InsHoldStateVC InsHoldStateVC_singleton;

        void enter();                       // performs actions necessary upon entering the state
        State* update();                    // perform maintenance, and perhaps transition to next state
    private: 
        InsHoldStateVC() {};                // default constructor
    };

    // Expiration 
    class ExpStateVC : public State {
    public:
        static ExpStateVC ExpStateVC_singleton;

        void enter();                   // performs actions necessary upon entering the state
        State* update();                // perform maintenance, and perhaps transition to next state
    private: 
        ExpStateVC() {};                // default constructor
    };

    // Measure PEEP pressure at end of expiration
    class PeepStateVC : public State {
    public:
        static PeepStateVC PeepStateVC_singleton;

        void enter();                    // performs actions necessary upon entering the state
        State* update();                 // perform maintenance, and perhaps transition to next state
    private: 
        PeepStateVC() {};                // default constructor
    };

    // Waiting for new breath to begin
    class ExpHoldStateVC : public State {
    public:
        static ExpHoldStateVC ExpHoldStateVC_singleton;

        void enter();                       // performs actions necessary upon entering the state
        State* update();                    // perform maintenance, and perhaps transition to next state
    private: 
        ExpHoldStateVC() {};                // default constructor
    };
};

#endif
