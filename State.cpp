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

}

// performs actions necessary upon exiting a state
void OffState::exit() {

}

// perform maintenance, and perhaps transition to new state
State OffState::update() {

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
    
}

// performs actions necessary upon exiting a state
void InsState::exit() {
    
}

// perform maintenance, and perhaps transition to new state
State InsState::update() {
    
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

}

// performs actions necessary upon exiting a state
void InsHoldState::exit() {

}

// perform maintenance, and perhaps transition to new state
State InsHoldState::update() {

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

}

// performs actions necessary upon exiting a state
void SustState::exit() {

}

// perform maintenance, and perhaps transition to new state
State SustState::update() {

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

}

// performs actions necessary upon exiting a state
void ExpState::exit() {

}

// perform maintenance, and perhaps transition to new state
State ExpState::update() {

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

}

// performs actions necessary upon exiting a state
void PeepState::exit() {

}

// perform maintenance, and perhaps transition to new state
State PeepState::update() {

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

}

// performs actions necessary upon exiting a state
void ExpHoldState::exit() {

}

// perform maintenance, and perhaps transition to new state
State ExpHoldState::update() {

}
