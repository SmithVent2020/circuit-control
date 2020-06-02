/* Stubs for UI variables and functions
 *
 */

#include "UI.h"

vc_settings_t vc_settings = {
  /* int   volume          = */ 300,   // Tidal volume cc
  /* int   bpm             = */ 15,    // Respiratory rate
  /* int   inspPercent;    = */ 33,    // % of breath that is inhalation (ie / (ie + 1))
  /* float peak            = */ 35,    // peak pressure (PiP)
  /* int   o2concentration = */ 21,   // O2 concentration, in percent
  /* float sensitivity     = */ 3,     // pressure sensitivity
  /* bool  inspHoldOn      = */ false  // Do we want to hold inspiration?
};


ps_settings_t ps_settings = {
  /* int   bpm             = */ 35,   // Respiratory rate
  /* float cycleOff        = */ 0.25, // cycle off % (peak flow % at which to stop pumping air)
  /* float peak            = */ 35,   // peak pressure (PiP) above peep
  /* float sensitivity     = */ 3,    // pressure sensitivity
  /* int   o2concentration = */ 100,  // O2 concentration, in percent
  /* int   backup_bpm      = */ 35,   // Backup BPM
  /* float backup_ie       = */ 0.5   // Backup IE ratio
};
