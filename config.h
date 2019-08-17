#include <vdr/menu.h>

class cLCARSNGConfig {
public:
  int Margin;
//  mcUndefined = -1,
//  mcUnknown = 0,
  int mcMainScaled;
  int mcScheduleScaled;
//  mcScheduleNow,
//  mcScheduleNext,
  int mcChannelScaled;
//  mcChannelEdit,
  int mcTimerScaled;
//  mcTimerEdit,
  int mcRecordingScaled;
//  mcRecordingInfo,
//  mcRecordingEdit,
  int mcPluginScaled;
//  mcPluginSetup,
  int mcSetupScaled;
//  mcSetupOsd,
//  mcSetupEpg,
//  mcSetupDvb,
//  mcSetupLnb,
//  mcSetupCam,
//  mcSetupRecord,
//  mcSetupReplay,
//  mcSetupMisc,
//  mcSetupPlugins,
  int mcCommandScaled;
  int mcEventScaled;
  int mcTextScaled;
  int mcFolderScaled;
  int mcCamScaled;
  bool SetupParse(const char *Name, const char *Value);
  cLCARSNGConfig(void);
};

extern cLCARSNGConfig Config;
