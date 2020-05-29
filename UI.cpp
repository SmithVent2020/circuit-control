/* Stubs for UI variables and functions
 *  
 */

#include "UI.h"

vc_settings_t vc_settings = {
  /* int   volume      = */ 500,   // Tidal volume
  /* int   bpm         = */ 35,    // Respiratory rate
  /* float ie          = */ 0.5,   // Inhale/exhale ratio
  /* float peak        = */ 35,    // peak pressure (PiP)
  /* int   o2          = */ 21,    // O2 concentration
  /* float sensitivity = */ 3,     // pressure sensitivity
  /* bool  inspHoldOn  = */ false  // Do we want to hold inspiration?
};

ps_settings_t ps_settings = {
  /* int   bpm         = */ 35,   // Respiratory rate
  /* float cycleOff    = */ 0.25, // cycle off % (peak flow % at which to stop pumping air)
  /* float peak        = */ 35,   // peak pressure (PiP) above peep
  /* float sensitivity = */ 3,    // pressure sensitivity
  /* int   o2          = */ 21,   // O2 concentration
  /* int   backup_bpm  = */ 35,   // Backup BPM
  /* float backup_ie   = */ 0.5   // Backup IE ratio
};
