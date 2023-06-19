#include <vdr/plugin.h>
#include "config.h"

cLCARSNGConfig Config;

cLCARSNGConfig::cLCARSNGConfig(void)
{
  Margin = 2;
  displayError0 = 0;
  displInfoMenuEPG = 0;
  displInfoMenuTimer = 0;
  displInfoMenuRec = 0;
  displInfoChannel = 0;
  infoChanLines = 10;
  waitTimeFadein = 1000;
  waitTimeScroll = 1000;
  fadeinTime = 300;
  scrollPixel = 2;
  framesPerSecond = 25;
  mcUndefinedScaled = efullscreen;
  mcUnknownScaled = efullscreen;
  mcMainScaled = efullscreen;
  mcScheduleScaled = efullscreen;
  mcScheduleNowScaled = efullscreen;
  mcScheduleNextScaled = efullscreen;
  mcChannelScaled = efullscreen;
  mcChannelEditScaled = efullscreen;
  mcTimerScaled = efullscreen;
  mcTimerEditScaled = efullscreen;
  mcRecordingScaled = efullscreen;
  mcRecordingInfoScaled = efullscreen;
  mcRecordingEditScaled = efullscreen;
  mcPluginScaled = efullscreen;
  mcPluginSetupScaled = efullscreen;
  mcSetupScaled = efullscreen;
  mcSetupOsdScaled = efullscreen;
  mcSetupEpgScaled = efullscreen;
  mcSetupDvbScaled = efullscreen;
  mcSetupLnbScaled = efullscreen;
  mcSetupCamScaled = efullscreen;
  mcSetupRecordScaled = efullscreen;
  mcSetupReplayScaled = efullscreen;
  mcSetupMiscScaled = efullscreen;
  mcSetupPluginsScaled = efullscreen;
  mcCommandScaled = efullscreen;
  mcEventScaled = efullscreen;
  mcTextScaled = efullscreen;
  mcFolderScaled = efullscreen;
  mcCamScaled = efullscreen;
  mcDefaultScaled = efullscreen;
  displayGrid = 0;
}

bool cLCARSNGConfig::SetupParse(const char *Name, const char *Value)
{
  if      (!strcasecmp(Name, "Margin"))             Margin             = atoi(Value);
  else if (!strcasecmp(Name, "DisplayError0"))      displayError0      = atoi(Value);
  else if (!strcasecmp(Name, "DisplInfoMenuEPG"))   displInfoMenuEPG   = atoi(Value);
  else if (!strcasecmp(Name, "DisplInfoMenuTimer")) displInfoMenuTimer = atoi(Value);
  else if (!strcasecmp(Name, "DisplInfoMenuRec"))   displInfoMenuRec   = atoi(Value);
  else if (!strcasecmp(Name, "DisplInfoChannel")) { displInfoChannel   = atoi(Value);
#ifndef USE_ZAPCOCKPIT
          if (displInfoChannel > 1)
             displInfoChannel = 1;
#endif
     }
  else if (!strcasecmp(Name, "InfoChanLines"))      infoChanLines      = atoi(Value);
  else if (!strcasecmp(Name, "WaitTimeFadein"))     waitTimeFadein     = atoi(Value);
  else if (!strcasecmp(Name, "WaitTimeScroll"))     waitTimeScroll     = atoi(Value);
  else if (!strcasecmp(Name, "FadeInTime"))         fadeinTime         = atoi(Value);
  else if (!strcasecmp(Name, "ScrollPixel"))        scrollPixel        = atoi(Value);
  else if (!strcasecmp(Name, "FramesPerSecond"))    framesPerSecond    = atoi(Value);
  else if (!strcasecmp(Name, "MainMenue"))          mcMainScaled       = atoi(Value);
  else if (!strcasecmp(Name, "ScheduleMenue"))      mcScheduleScaled   = atoi(Value);
  else if (!strcasecmp(Name, "ChannelMenue"))       mcChannelScaled    = atoi(Value);
  else if (!strcasecmp(Name, "TimerMenue"))         mcTimerScaled      = atoi(Value);
  else if (!strcasecmp(Name, "RecordingMenue"))     mcRecordingScaled  = atoi(Value);
  else if (!strcasecmp(Name, "PluginMenue"))        mcPluginScaled     = atoi(Value);
  else if (!strcasecmp(Name, "SetupMenue"))         mcSetupScaled      = atoi(Value);
  else if (!strcasecmp(Name, "CommandMenue"))       mcCommandScaled    = atoi(Value);
  else if (!strcasecmp(Name, "EventMenue"))         mcEventScaled      = atoi(Value);
//  else if (!strcasecmp(Name, "TextMenue"))          mcTextScaled       = atoi(Value);
//  else if (!strcasecmp(Name, "FolderMenue"))        mcFolderScaled     = atoi(Value);
//  else if (!strcasecmp(Name, "CamMenue"))           mcCamScaled        = atoi(Value);
  else if (!strcasecmp(Name, "DefaultMenue"))       mcDefaultScaled    = atoi(Value);
  else if (!strcasecmp(Name, "DisplayGrid"))        displayGrid        = atoi(Value);
  else
     return false;
  return true;
}
