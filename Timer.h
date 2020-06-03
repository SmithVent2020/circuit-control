#ifndef Timer_h

#include <arduino.h>

// Counts up ms since started.
class Timer {
  public:
    Timer() { }

    // Start (or restart) the timer
    void start() { startTime_ = millis(); }

    // Return elapsed time in ms
    unsigned long elapsed() const { return millis() - startTime_; }

  private:
    unsigned long startTime_ = 0;
};

// Counts down a specified number of milliseconds.
class CountdownTimer {
  public:
    explicit CountdownTimer(unsigned long targetMs = 0) : targetMs_(targetMs) { }

    // Start (or restart) the timer
    void start() { targetEndTime_ = millis() + targetMs_; }

    // Reset the countdown time. Takes effect on next call to `start()`.
    void reset(unsigned long targetMs) { targetMs_ = targetMs; }

    // Return true if countdown is complete.
    bool isExpired() const { millis() >= targetEndTime_; }

    // Remaining time, in ms
    unsigned long remainingTime() const {
        unsigned long now = millis();
        return now >= targetEndTime_ ? 0 : targetEndTime_ - now;
    }

    // Return count set on construction or from the last call to `reset()`
    unsigned long getCount() const { return targetMs_; }

  private:
    unsigned long targetMs_;
    unsigned long targetEndTime_ = 0;
};

#endif // Timer_h
