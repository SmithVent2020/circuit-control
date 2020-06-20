/**
 * Flow.h
 * Calculates and stores the key flow values in the breathing cycle.
 * All flow values are measured in SLPM
 */

#ifndef Flow_h
#define Flow_h


class Flow {
  public:
    Flow(int pin);

    // reads sensor, should be called at most once per main loop iteration
    void read();
    void reset();
    void calibrateToZero();

    // `get` can be called efficiently at will after `read` is called
    float get() const { return flow_rate_; }

    // The following functions integrate flow over time to get a computed volume.
    void resetVolume();
    void updateVolume();

    /** 
     * Returns accumulated volume (cc) assuming a pressure of one atm, which 
     * would make it equivalent to SLPM. Since maximum inspiratory pressure 
     * is rarely more than 20 cmH2O over atm, this measurement can be considered a
     * reasonably accurate measurement of actual volume
     * 
     * volume = SLPM * time (in min) * 1000
     */ 
    float getVolume() const { return accum_volume_; }

  private:
    int   sensor_pin_;
    float flow_rate_;

    // raw reading at 0 flow -> adjustment to future readings
    long zero_flow_offset_;  

    // Volume integraton
    unsigned long last_timepoint_; // Time of last call to `resetVolume` or `updateVolume`.
    float         accum_volume_;   // Accumulated volume in cc at one atm
};

// Flow sensors
extern Flow inspFlowReader;
extern Flow expFlowReader;

#endif // Flow_h
