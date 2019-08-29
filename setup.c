/*
 * See the README file for copyright information and how to reach the author.
 */

#include "config.h"
#include "setup.h"

cLCARSNGConfig tmpConfig;

/******************** MenuSetup ********************/
cLCARSNGSetup::cLCARSNGSetup() {

  tmpConfig = Config;

  int currentItem = Current();
  SetCols(45);

  Add(new cMenuEditIntItem(tr("Margin"),          &tmpConfig.Margin, 0, 5));
  Add(new cOsdItem(tr("Scaled video in menue:"),  osUnknown, false));
  Add(new cMenuEditBoolItem(tr("MainMenue"),      &tmpConfig.mcMainScaled));
  Add(new cMenuEditBoolItem(tr("ScheduleMenue"),  &tmpConfig.mcScheduleScaled));
  Add(new cMenuEditBoolItem(tr("ChannelMenue"),   &tmpConfig.mcChannelScaled));
  Add(new cMenuEditBoolItem(tr("TimerMenue"),     &tmpConfig.mcTimerScaled));
  Add(new cMenuEditBoolItem(tr("RecordingMenue"), &tmpConfig.mcRecordingScaled));
//  Add(new cMenuEditBoolItem(tr("PluginMenue"),    &tmpConfig.mcPluginScaled));
  Add(new cMenuEditBoolItem(tr("SetupMenue"),     &tmpConfig.mcSetupScaled));
  Add(new cMenuEditBoolItem(tr("CommandMenue"),   &tmpConfig.mcCommandScaled));
  Add(new cMenuEditBoolItem(tr("EventMenue"),     &tmpConfig.mcEventScaled));
//  Add(new cMenuEditBoolItem(tr("TextMenue"),      &tmpConfig.mcTextScaled));
//  Add(new cMenuEditBoolItem(tr("FolderMenue"),    &tmpConfig.mcFolderScaled));
//  Add(new cMenuEditBoolItem(tr("CamMenue"),       &tmpConfig.mcCamScaled));

  SetCurrent(Get(currentItem));
  Display();
  SetHelp(NULL, NULL, NULL, NULL);
}

void cLCARSNGSetup::Store(void) {
  Config = tmpConfig;
  SetupStore("Margin",          Config.Margin);
  SetupStore("MainMenue",       Config.mcMainScaled);
  SetupStore("ScheduleMenue",   Config.mcScheduleScaled);
  SetupStore("ChannelMenue",    Config.mcChannelScaled);
  SetupStore("TimerMenue",      Config.mcTimerScaled);
  SetupStore("RecordingMenue",  Config.mcRecordingScaled);
//  SetupStore("PluginMenue",     Config.mcPluginScaled);
  SetupStore("SetupMenue",      Config.mcSetupScaled);
  SetupStore("CommandMenue",    Config.mcCommandScaled);
  SetupStore("EventMenue",      Config.mcEventScaled);
//  SetupStore("TextMenue",       Config.mcTextScaled);
//  SetupStore("FolderMenue",     Config.mcFolderScaled);
//  SetupStore("CamMenue",        Config.mcCamScaled);
}
