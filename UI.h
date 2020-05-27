// Variables and subroutines to access the User Interface from other parts of
// the system.

// Volume Control settings type
// @TODO: update these values from UI inputs
struct vc_settings_t {
  int   volume      = 500;   // Tidal volume
  int   bpm         = 35;    // Respiratory rate
  float ie          = 0.5;   // Inhale/exhale ratio
  float peak        = 35;    // peak pressure (PiP)
  int   o2          = 21;    // O2 concentration
  float sensitivity = 3;     // pressure sensitivity
  bool  inspHoldOn  = false; // Do we want to hold inspiration?
};

// Global Volume Control settings
extern vc_settings_t vc_settings;

// Pressure support settings type.
// @TODO: update these values from UI inputs
struct ps_settings_t {
  int   bpm         = 35;   // Respiratory rate
  float cycleOff    = 0.25; // cycle off % (peak flow % at which to stop pumping air)
  float peak        = 35;   // peak pressure (PiP) above peep
  float sensitivity = 3;    // pressure sensitivity
  int   o2          = 21;   // O2 concentration
  int   backup_bpm  = 35;   // Backup BPM
  float backup_ie   = 0.5;  // Backup IE ratio
};

// Global Pressure Support settings
extern ps_settings_t ps_settings;

// UI will turn this true when the ON button is pressed
extern bool OnButtonPressed;
