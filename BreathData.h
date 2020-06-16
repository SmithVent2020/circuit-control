#ifndef BREATH_DATA_H
#define BREATH_DATA_H

// Controls state machine behavior and other operational characteristics
/*enum VentilatorMode {
    STANDBY_MODE,  // 0
    VC_MODE,       // 1
    PS_MODE        // 2
};*/

class BreathData {
  public:
    //VentilatorMode mode;
    float targetExpVolume    = 0; // minimum target volume for expiration

    // Target time parametaers (in milliseconds). Calculated, not measured.
    unsigned long targetCycleEndTime;     // desired time at end of breath (at end of HOLD_EXP_STATE)
    unsigned long targetInspEndTime;      // desired time for end of INSP_STATE
    unsigned long targetExpDuration;      // Desired length of EXP_STATE (VC mode only)
    unsigned long targetExpEndTime;       // Desired time at end of EXP_STATE (VC mode only)

    // Timers (i.e., start times, in milliseconds):
    unsigned long cycleTimer;        // Start time of start of current breathing cycle
    unsigned long inspHoldTimer;     // Start time of inpsiratory hold state
    unsigned long expTimer;          // Start time of expiration cycle (including exp hold & peep pause)
    unsigned long peepPauseTimer;    // Start time of peep pause

    // Timer results (intervals, in milliseconds):
    unsigned long cycleDuration;      // Measured length of a whole inspiration-expiration cycle
    unsigned long inspDuration;       // Measured length of inspiration (not including inspiratory hold)
    unsigned long expDuration;        // Measured length of expiration (including peep pause and expiratory hold)
    unsigned long targetInspDuration; //desired duration of inspiration


    float desiredInspFlow; //desired inspiratory flowrate
    bool onButton = true;  //should be connected to a physical on button


    float tidalVolumeInsp = 0.0; //measured inspiratory tidal volume
    float tidalVolumeExp = 0.0;  //measured expiratory tidal volume

    float lastPeep = 0.0/0.0; //PEEP from last breath
    float lastPeak = 0.0/0.0; //peak pressure from last breath

    // update functions
    void beginInspiration();
    void beginHoldInspiration();
    void beginExpiration();
    void beginPeepPause();
    void beginHoldExpiration();
};

#endif
