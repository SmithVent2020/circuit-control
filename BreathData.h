#ifndef BREATH_DATA_H
#define BREATH_DATA_H

// Controls state machine behavior and other operational characteristics
enum VentilatorMode {
    STANDBY_MODE,  // 0
    VC_MODE,       // 1
    PS_MODE        // 2
};

class BreathData {
  public:
    VentilatorMode mode;
    // add other parameters like BPM, IE ratio, timings, etc.
};

#endif
