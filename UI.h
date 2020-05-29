// Variables and subroutines to access the User Interface from other parts of
// the system.

#ifndef UI_h
#define UI_h

// Volume Control settings type
// @TODO: update these values from UI inputs
struct vc_settings_t {
  int   volume;          // Tidal volume
  int   bpm;             // Respiratory rate
  int   ie;              // Inhale/exhale ratio (%inhalation)
  float peak;            // peak pressure (PiP)
  int   o2concentration; // O2 concentration (0% to 100%)
  float sensitivity;     // pressure sensitivity
  bool  inspHoldOn;      // Do we want to hold inspiration?
};

// Global Volume Control settings
extern vc_settings_t vc_settings;

// Pressure support settings type.
// @TODO: update these values from UI inputs
struct ps_settings_t {
  int   bpm;             // Respiratory rate
  float cycleOff;        // cycle off % (peak flow % at which to stop pumping air)
  float peak;            // peak pressure (PiP) above peep
  float sensitivity;     // pressure sensitivity
  int   o2concentration; // O2 concentration (0% to 100%)
  int   backup_bpm;      // Backup BPM
  float backup_ie;       // Backup IE ratio
};

// Global Pressure Support settings
extern ps_settings_t ps_settings;

// UI will turn this true when the ON button is pressed
extern bool OnButtonPressed;

#endif // UI_h
