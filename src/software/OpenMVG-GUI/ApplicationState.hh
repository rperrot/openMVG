#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_APPLICATION_STATE_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_APPLICATION_STATE_HH_

namespace openMVG_gui 
{
  // Information about the state of the application 
  // used by the interface to enable/disable some actions 
  enum ApplicationState 
  {
    // No project loaded 
    STATE_EMPTY , 
    // Project opened 
    STATE_PROJECT_OPENED , 
    // Features computed 
    STATE_FEATURES_COMPUTED ,
    // Matches computed 
    STATE_MATCHES_COMPUTED , 
    // SfM Computed 
    STATE_SFM_COMPUTED , 
    // Color structure Computed 
    STATE_COLOR_COMPUTED 
  } ;
} // namespace openMVG_gui 

#endif