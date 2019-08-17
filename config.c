/*
 * See the README file for copyright information and how to reach the author.
 */

#include <vdr/plugin.h>
#include "config.h"

cLCARSNGConfig Config;

cLCARSNGConfig::cLCARSNGConfig(void)
{
  Margin = 2;
//  mcUndefined = -1,
//  mcUnknown = 0,
  mcMainScaled = false;
  mcScheduleScaled = false;
//  mcScheduleNow,
//  mcScheduleNext,
  mcChannelScaled = false;
//  mcChannelEdit,
  mcTimerScaled = false;
//  mcTimerEdit,
  mcRecordingScaled = false;
//  mcRecordingInfo,
//  mcRecordingEdit,
  mcPluginScaled = false;
//  mcPluginSetup,
  mcSetupScaled = false;
//  mcSetupOsd,
//  mcSetupEpg,
//  mcSetupDvb,
//  mcSetupLnb,
//  mcSetupCam,
//  mcSetupRecord,
//  mcSetupReplay,
//  mcSetupMisc,
//  mcSetupPlugins,
  mcCommandScaled = false;
  mcEventScaled = false;
  mcTextScaled = false;
  mcFolderScaled = false;
  mcCamScaled = false;
}

bool cLCARSNGConfig::SetupParse(const char *Name, const char *Value)
{
  if      (!strcasecmp(Name, "Margin"))          Margin             = atoi(Value);
  else if (!strcasecmp(Name, "MainMenue"))       mcMainScaled       = atoi(Value);
  else if (!strcasecmp(Name, "ScheduleMenue"))   mcScheduleScaled   = atoi(Value);
  else if (!strcasecmp(Name, "ChannelMenue"))    mcChannelScaled    = atoi(Value);
  else if (!strcasecmp(Name, "TimerMenue"))      mcTimerScaled      = atoi(Value);
  else if (!strcasecmp(Name, "RecordingMenue"))  mcRecordingScaled  = atoi(Value);
  else if (!strcasecmp(Name, "PluginMenue"))     mcPluginScaled     = atoi(Value);
  else if (!strcasecmp(Name, "SetupMenue"))      mcSetupScaled      = atoi(Value);
  else if (!strcasecmp(Name, "CommandMenue"))    mcCommandScaled    = atoi(Value);
  else if (!strcasecmp(Name, "EventMenue"))      mcEventScaled      = atoi(Value);
  else if (!strcasecmp(Name, "TextMenue"))       mcTextScaled       = atoi(Value);
  else if (!strcasecmp(Name, "FolderMenue"))     mcFolderScaled     = atoi(Value);
  else if (!strcasecmp(Name, "CamMenue"))        mcCamScaled        = atoi(Value);
  else
     return false;
  return true;
}
