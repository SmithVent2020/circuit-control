/* Stubs for UI variables and functions
 *
 */

#include "UI.h"

vc_settings_t vc_settings = {
  /* int   volume          = */ 500,   // Tidal volume cc
  /* int   bpm             = */ 20,    // Respiratory rate
  /* int   inspPercent;    = */ 33,    // % of breath that is inhalation (ie / (ie + 1))
  /* float peak            = */ 35,    // peak pressure (PiP)
  /* int   o2concentration = */ 60,    // O2 concentration, in percent
  /* float sensitivity     = */ 3,     // pressure sensitivity
  /* bool  inspHoldOn      = */ true  // Do we want to hold inspiration?
};

vc_display_t vc_display = {
  /*int pip =*/              0,  //cmH2O
  /*int pPlat; =*/           0, //cmH2O
  /*int PEEP;=*/             0,  //cmH2O
  /*long insptidalVolume;=*/ 0,  //mL (cc)
  /*long expTidalVolume;=*/  0,  //mL (cc)
  /*long minuteVolume; =*/   0,  //L/min
  /*long respiratoryRate;=*/ 0,  //bpm
  /*long FIO2; =*/           0  //%
  
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
