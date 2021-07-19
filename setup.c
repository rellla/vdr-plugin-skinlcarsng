#include "config.h"
#include "setup.h"

cLCARSNGConfig tmpConfig;

/******************** MenuSetup ********************/
cLCARSNGSetup::cLCARSNGSetup() {

  tmpConfig = Config;

  int currentItem = Current();
  SetCols(35);

  menuView[0] = tr("scaled video");
  menuView[1] = tr("full screen");
  menuView[2] = tr("split screen");
  menuView[3] = tr("thin screen");

  Add(new cMenuEditIntItem(tr("Margin"),                      &tmpConfig.Margin, 0, 5));
  Add(new cMenuEditBoolItem(tr("Display Error 0"),            &tmpConfig.displayError0));

  Add(new cOsdItem("",                                        osUnknown, false));
  Add(new cOsdItem(tr("Display description:"),                osUnknown, false));
  Add(new cMenuEditBoolItem(tr("In menu epg"),                &tmpConfig.displInfoMenuEPG));
  Add(new cMenuEditBoolItem(tr("In menu timer"),              &tmpConfig.displInfoMenuTimer));
  Add(new cMenuEditBoolItem(tr("In menu recording"),          &tmpConfig.displInfoMenuRec));
  Add(new cMenuEditIntItem(tr("Waiting period Fade-in (ms)"), &tmpConfig.waitTimeFadein, 0, 5000, trVDR("off")));
  Add(new cMenuEditIntItem(tr("Waiting period Scroll (ms)"),  &tmpConfig.waitTimeScroll, 0, 5000, trVDR("off")));
  Add(new cMenuEditIntItem(tr("Fade-in time (ms)"),           &tmpConfig.fadeinTime, 0, 1000, trVDR("off")));
  Add(new cMenuEditIntItem(tr("ScrollPixel"),                 &tmpConfig.scrollPixel, 1, 10));
  Add(new cMenuEditIntItem(tr("Refreshrate (Frames/s)"),      &tmpConfig.framesPerSecond, 10, 60));

  Add(new cOsdItem("",                                        osUnknown, false));
  Add(new cOsdItem(tr("Menue view:"),                         osUnknown, false));
  Add(new cMenuEditStraItem(tr("main menue"),                 &tmpConfig.mcMainScaled, 2, menuView));
  Add(new cMenuEditStraItem(tr("schedule menue"),             &tmpConfig.mcScheduleScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("event menue"),                &tmpConfig.mcEventScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("channel menue"),              &tmpConfig.mcChannelScaled, 4, menuView));
  Add(new cMenuEditStraItem(tr("timer menue"),                &tmpConfig.mcTimerScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("recording menue"),            &tmpConfig.mcRecordingScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("setup menue"),                &tmpConfig.mcSetupScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("command menue"),              &tmpConfig.mcCommandScaled, 3, menuView));
  Add(new cMenuEditStraItem(tr("plugin menue"),               &tmpConfig.mcPluginScaled, 3, menuView));
//  Add(new cMenuEditStraItem(tr("Text menue"),                 &tmpConfig.mcTextScaled));
//  Add(new cMenuEditStraItem(tr("Folder menue"),               &tmpConfig.mcFolderScaled));
//  Add(new cMenuEditStraItem(tr("Cam menue"),                  &tmpConfig.mcCamScaled));
  Add(new cMenuEditStraItem(tr("Default view"),               &tmpConfig.mcDefaultScaled, 3, menuView));

  SetCurrent(Get(currentItem));
  Display();
  SetHelp(NULL, NULL, NULL, NULL);
}

void cLCARSNGSetup::Store(void) {
  Config = tmpConfig;
  SetupStore("Margin",             Config.Margin);
  SetupStore("DisplayError0",      Config.displayError0);
  SetupStore("DisplInfoMenuEPG",   Config.displInfoMenuEPG);
  SetupStore("DisplInfoMenuTimer", Config.displInfoMenuTimer);
  SetupStore("DisplInfoMenuRec",   Config.displInfoMenuRec);
  SetupStore("WaitTimeFadein",     Config.waitTimeFadein);
  SetupStore("WaitTimeScroll",     Config.waitTimeScroll);
  SetupStore("FadeInTime",         Config.fadeinTime);
  SetupStore("ScrollPixel",        Config.scrollPixel);
  SetupStore("FramesPerSecond",    Config.framesPerSecond);
  SetupStore("MainMenue",          Config.mcMainScaled);
  SetupStore("ScheduleMenue",      Config.mcScheduleScaled);
  SetupStore("ChannelMenue",       Config.mcChannelScaled);
  SetupStore("TimerMenue",         Config.mcTimerScaled);
  SetupStore("RecordingMenue",     Config.mcRecordingScaled);
  SetupStore("PluginMenue",        Config.mcPluginScaled);
  SetupStore("SetupMenue",         Config.mcSetupScaled);
  SetupStore("CommandMenue",       Config.mcCommandScaled);
  SetupStore("EventMenue",         Config.mcEventScaled);
//  SetupStore("TextMenue",          Config.mcTextScaled);
//  SetupStore("FolderMenue",        Config.mcFolderScaled);
//  SetupStore("CamMenue",           Config.mcCamScaled);
  SetupStore("DefaultMenue",       Config.mcDefaultScaled);
}
