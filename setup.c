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

  menuView[0] = tr("scaled video");
  menuView[1] = tr("fullscreen");
  menuView[2] = tr("splitscreen");

  Add(new cMenuEditIntItem(tr("Margin"),          &tmpConfig.Margin, 0, 5));

  Add(new cOsdItem(tr("Menue view:"),  osUnknown, false));

  Add(new cMenuEditStraItem(tr("main menue"),      &tmpConfig.mcMainScaled, 2, menuView));
  Add(new cMenuEditStraItem(tr("schedule menue"),  &tmpConfig.mcScheduleScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("event menue"),     &tmpConfig.mcEventScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("channel menue"),   &tmpConfig.mcChannelScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("timer menue"),     &tmpConfig.mcTimerScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("recording menue"), &tmpConfig.mcRecordingScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("setup menue"),     &tmpConfig.mcSetupScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("command menue"),   &tmpConfig.mcCommandScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("plugin menue"),    &tmpConfig.mcPluginScaled, 3, menuView));
//  Add(new cMenuEditStraItem(tr("Text menue"),      &tmpConfig.mcTextScaled));
//  Add(new cMenuEditStraItem(tr("Folder menue"),    &tmpConfig.mcFolderScaled));
//  Add(new cMenuEditStraItem(tr("Cam menue"),       &tmpConfig.mcCamScaled));
  Add(new cMenuEditStraItem(tr("Default view"),   &tmpConfig.mcDefaultScaled, 3, menuView));

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
  SetupStore("PluginMenue",     Config.mcPluginScaled);
  SetupStore("SetupMenue",      Config.mcSetupScaled);
  SetupStore("CommandMenue",    Config.mcCommandScaled);
  SetupStore("EventMenue",      Config.mcEventScaled);
//  SetupStore("TextMenue",       Config.mcTextScaled);
//  SetupStore("FolderMenue",     Config.mcFolderScaled);
//  SetupStore("CamMenue",        Config.mcCamScaled);
  SetupStore("DefaultMenue",    Config.mcDefaultScaled);
}
