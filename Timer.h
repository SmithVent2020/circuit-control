#ifndef Timer_h
#define Timer_h

#include <arduino.h>
#include <limits.h>

// @TODO: None of our timer functions handle time rollover.

// Counts up ms since started.
class Timer {
  public:
    // Create timer. Retroactively starts timing from system start.
    Timer() { }

    // Start (or restart) the timer
    void start() { startTime_ = millis(); }

    // Return time elapsed (ms) since last call to 'start()'
    unsigned long elapsed() const { return millis() - startTime_; }

  private:
    unsigned long startTime_ = 0;
};

// Counts down a specified number of milliseconds.
class CountdownTimer {
  public:
    static const unsigned long NEVER_EXPIRE = ULONG_MAX;

    // Create a countdown timer. Optionally specify time (ms) to countdown.
    // Timer is created in expired state.
    explicit CountdownTimer(unsigned long targetMs = 0) : targetMs_(targetMs) { }

    // Reset the countdown time (ms). New countdown takes effect on next call to `start()`.
    void setCountdown(unsigned long targetMs) { targetMs_ = targetMs; }

    // Start (or restart) the timer
    void start() { targetEndTime_ = millis() + targetMs_; }

    // Set the countdown and start the timer
    void start(unsigned long targetMs) {
        setCountdown(targetMs);
        start();
    }

    // Cancel the timer (so it will never expire).
    void cancel() { targetEndTime_ = NEVER_EXPIRE; }

    // Return true if countdown is complete.
    bool hasExpired() const { millis() >= targetEndTime_; }

    // Remaining time on the countdown (ms). Returns zero if timer has expired.
    unsigned long remaining() const {
        unsigned long now = millis();
        return now >= targetEndTime_ ? 0 : targetEndTime_ - now;
    }

    // Return count set on construction or from the last call to `reset()`.
    // Do not confuse with `remaining()`.
    unsigned long getCountdown() const { return targetMs_; }

  private:
    unsigned long targetMs_;
    unsigned long targetEndTime_ = 0;
};

#endif // Timer_h
