enum viewMode {
    escaledvideo = 0,
    efullscreen,
    esplitscreen
};

class cLCARSNGConfig {
public:
  int Margin;
  int mcUndefinedScaled;
  int mcUnknownScaled;
  int mcMainScaled;
  int mcScheduleScaled;
  int mcScheduleNowScaled;
  int mcScheduleNextScaled;
  int mcChannelScaled;
  int mcChannelEditScaled;
  int mcTimerScaled;
  int mcTimerEditScaled;
  int mcRecordingScaled;
  int mcRecordingInfoScaled;
  int mcRecordingEditScaled;
  int mcPluginScaled;
  int mcPluginSetupScaled;
  int mcSetupScaled;
  int mcSetupOsdScaled;
  int mcSetupEpgScaled;
  int mcSetupDvbScaled;
  int mcSetupLnbScaled;
  int mcSetupCamScaled;
  int mcSetupRecordScaled;
  int mcSetupReplayScaled;
  int mcSetupMiscScaled;
  int mcSetupPluginsScaled;
  int mcCommandScaled;
  int mcEventScaled;
  int mcTextScaled;
  int mcFolderScaled;
  int mcCamScaled;
  int mcDefaultScaled;
  bool SetupParse(const char *Name, const char *Value);
  cLCARSNGConfig(void);
};

extern cLCARSNGConfig Config;
