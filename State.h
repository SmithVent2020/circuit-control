#ifndef STATE_H
#define STATE_H

#include "BreathData.h"

/*
 *  Parent class for all instantiated state classes
 */
class State {
  public:
    virtual State* update() = 0;           // perform maintenance, and perhaps transition to new state
    static BreathData* breath;             // holds shared information about the breath cycle
};

/*
 *  Specific states used by the ventilator control.
 *  Only a singleton instance of each is created, accessible via the get() method.
 */

// Standby mode.  This is the initial state.
class OffState : public State {
  public:
    static State* enter();               // performs actions necessary upon entering the state
    State* update();                     // perform maintenance, and perhaps transition to next state
    static State* begin(BreathData);     // returns the OffState instance with breath data link
  private: 
    OffState() {};                       // to create singleton instance
    static OffState *instance;           // storage for singleton instance
};

// Inspiration begins a breath cycle
class InsStatePS : public State {
  public:
    static State* enter();               // performs actions necessary upon entering the state
    State* update();                     // perform maintenance, and perhaps transition to next state
  private: 
    InsStatePS() {};                       // to create singleton instance
    static InsStatePS *instance;           // storage for singleton instance
};

// USed in volume control mode during inspiration.
// Physician may order an inspiratory hold to measure plateau pressure
class InsHoldStatePS : public State {
  public:
    static State* enter();               // performs actions necessary upon entering the state
    State* update();                     // perform maintenance, and perhaps transition to next state
  private: 
    InsHoldStatePS() {};                   // to create singleton instance
    static InsHoldStatePS *instance;
};

// Expiration 
class ExpStateVC : public State {
  public:
    static State* enter();               // performs actions necessary upon entering the state
    State* update();                     // perform maintenance, and perhaps transition to next state
  private: 
    ExpStateVC() {};                       // to create singleton instance
    static ExpStateVC *instance;
};

// Used in pressure support mode during inspiration
class SustStatePS : public State {
  public:
    static State* enter();               // performs actions necessary upon entering the state
    State* update();                     // perform maintenance, and perhaps transition to next state
  private: 
    SustStatePS() {};                      // to create singleton instance
    static SustStatePS *instance;
};

// Measure PEEP pressure at end of expiration
class PeepStateVC : public State {
  public:
    static State* enter();               // performs actions necessary upon entering the state
    State* update();                     // perform maintenance, and perhaps transition to next state
  private: 
    PeepStateVC() {};                      // to create singleton instance
    static PeepStateVC *instance;
};

// Waiting for new breath to begin
class ExpHoldStateVC : public State {
  public:
    static State* enter();               // performs actions necessary upon entering the state
    State* update();                     // perform maintenance, and perhaps transition to next state
  private: 
    ExpHoldStateVC() {};                   // to create singleton instance
    static ExpHoldStateVC *instance;
};

#endif