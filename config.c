/*
 * See the README file for copyright information and how to reach the author.
 */

#include <vdr/plugin.h>
#include "config.h"

cLCARSNGConfig Config;

cLCARSNGConfig::cLCARSNGConfig(void)
{
  Margin = 2;
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
}

bool cLCARSNGConfig::SetupParse(const char *Name, const char *Value)
{
  if      (!strcasecmp(Name, "Margin"))          Margin             = atoi(Value);
  else if (!strcasecmp(Name, "MainMenue"))       mcMainScaled       = atoi(Value);
  else if (!strcasecmp(Name, "ScheduleMenue"))   mcScheduleScaled   = atoi(Value);
  else if (!strcasecmp(Name, "ChannelMenue"))    mcChannelScaled    = atoi(Value);
  else if (!strcasecmp(Name, "TimerMenue"))      mcTimerScaled      = atoi(Value);
  else if (!strcasecmp(Name, "RecordingMenue"))  mcRecordingScaled  = atoi(Value);
//  else if (!strcasecmp(Name, "PluginMenue"))     mcPluginScaled     = atoi(Value);
  else if (!strcasecmp(Name, "SetupMenue"))      mcSetupScaled      = atoi(Value);
  else if (!strcasecmp(Name, "CommandMenue"))    mcCommandScaled    = atoi(Value);
  else if (!strcasecmp(Name, "EventMenue"))      mcEventScaled      = atoi(Value);
//  else if (!strcasecmp(Name, "TextMenue"))       mcTextScaled       = atoi(Value);
//  else if (!strcasecmp(Name, "FolderMenue"))     mcFolderScaled     = atoi(Value);
//  else if (!strcasecmp(Name, "CamMenue"))        mcCamScaled        = atoi(Value);
  else if (!strcasecmp(Name, "DefaultMenue"))    mcDefaultScaled    = atoi(Value);
  else
     return false;
  return true;
}
