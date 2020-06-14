#ifndef STATE_H
#define STATE_H

#include "BreathData.h"

/*
 *  Parent class for all instantiated state classes
 */
class State {
  public:
    virtual void enter();                  // performs actions necessary upon entering a state
    virtual void exit();                   // performs actions necessary upon exiting a state
    virtual State update();                // perform maintenance, and perhaps transition to new state

    static BreathData breath;              // holds shared information about the breath cycle
};

/*
 *  Specific states used by the ventilator control.
 *  Only a singleton instance of each is created, accessible via the get() method.
 */

// Standby mode.  This is the initial state.
class OffState : public State {
  public:
    void enter();                        // performs actions necessary upon entering the state
    void exit();                         // performs actions necessary upon exiting the state
    State update();                      // perform maintenance, and perhaps transition to next state
    static OffState create(BreathData);  // returns the OffState instance with breath data link
    static State get();                  // returns the singleton instance of this class
  private: 
    OffState();                          // to create singleton instance
    static OffState *instance;           // storage for singleton instance
};

// Inspiration begins a breath cycle
class InsState : public State {
  public:
    void enter();                        // performs actions necessary upon entering the state
    void exit();                         // performs actions necessary upon exiting the state
    State update();                      // perform maintenance, and perhaps transition to next state
    static State get();                  // returns the singleton instance of this class
  private: 
    InsState();                          // to create singleton instance
    static InsState *instance;           // storage for singleton instance
};

// USed in volume control mode during inspiration.
// Physician may order an inspiratory hold to measure plateau pressure
class InsHoldState : public State {
  public:
    void enter();                        // performs actions necessary upon entering the state
    void exit();                         // performs actions necessary upon exiting the state
    State update();                      // perform maintenance, and perhaps transition to next state
    static State get();                  // returns the singleton instance of this class
  private: 
    InsHoldState();                      // to create singleton instance
    static InsHoldState *instance;
};

// Expiration 
class ExpState : public State {
  public:
    void enter();                        // performs actions necessary upon entering the state
    void exit();                         // performs actions necessary upon exiting the state
    State update();                      // perform maintenance, and perhaps transition to next state
    static State get();                  // returns the singleton instance of this class
  private: 
    ExpState();                          // to create singleton instance
    static ExpState *instance;
};

// Used in pressure support mode during inspiration
class SustState : public State {
  public:
    void enter();                        // performs actions necessary upon entering the state
    void exit();                         // performs actions necessary upon exiting the state
    State update();                      // perform maintenance, and perhaps transition to next state
    static State get();                  // returns the singleton instance of this class
  private: 
    SustState();                         // to create singleton instance
    static SustState *instance;
};

// Measure PEEP pressure at end of expiration
class PeepState : public State {
  public:
    void enter();                        // performs actions necessary upon entering the state
    void exit();                         // performs actions necessary upon exiting the state
    State update();                      // perform maintenance, and perhaps transition to next state
    static State get();                  // returns the singleton instance of this class
  private: 
    PeepState();                         // to create singleton instance
    static PeepState *instance;
};

// Waiting for new breath to begin
class ExpHoldState : public State {
  public:
    void enter();                        // performs actions necessary upon entering the state
    void exit();                         // performs actions necessary upon exiting the state
    State update();                      // perform maintenance, and perhaps transition to next state
    static State get();                  // returns the singleton instance of this class
  private: 
    ExpHoldState();                      // to create singleton instance
    static ExpHoldState *instance;
};

#endif
