/**
 * Flow.h
 * Calculates and stores the key flow values in the breathing cycle.
 */

#ifndef Flow_h
#define Flow_h

// Flow sensor type.
// The `read` function should be called at most once per main loop iteration.
// The `get` and `peak` functions can be called efficiently at will after
// the `read` function has been called.
class Flow {
  public:
    Flow(int pin);
    void read();
    void setPeakFlow();
    void reset();
    void calibrateToZero();
    

    void setPeakAndReset() {
      peak_flow_ = current_peak_;
      current_peak_ = 0.0;
    }

    // Values returned are in SLPM.
    float get() const { return flow_rate_; }
    float peak() const { return peak_flow_; }

    // The following functions integrate flow over time to get a computed volume.
    void resetVolume();
    void updateVolume();

    // Return accumulated volume (cc) assuming a pressure of one atm.
    // The returned unit is the cc equivilent to SLPM, i.e., SLPM * 1min * 1000.
    // Since maximum inspiration pressure is rarely more than 20 cmH2O over atm, this measurement can be considered a
    // reasonably accurate measurement of actual volume.
    float getVolume() const { return accum_volume_; }

    
    

  private:
    int   sensor_pin_;
    float flow_rate_;

    // only needed for pressure support
    float peak_flow_;
    float current_peak_;
    long zeroed_sensor_min_ = 1023*500; //sensor minimum usually around 500, calebrated in Setup

    // Volume integraton
    unsigned long last_timepoint_; // Time of last call to `resetVolume` or `updateVolume`.
    float         accum_volume_;   // Accumulated volume in cc at one atm
    
};

// Flow sensors
extern Flow inspFlowReader;
extern Flow expFlowReader;

#endif // Flow_h
