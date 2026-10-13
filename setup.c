#include "config.h"
#include "setup.h"

cLCARSNGConfig tmpConfig;

/******************** MenuSetup ********************/
cLCARSNGSetup::cLCARSNGSetup() {

  tmpConfig = Config;

  menuView[0] = tr("scaled video large");
  menuView[1] = tr("scaled video small");
  menuView[2] = tr("full screen");
  menuView[3] = tr("split screen");
  menuView[4] = tr("thin screen");

  volumeBar[0] = "LCARS";
  volumeBar[1] = tr("flat bar");
  volumeBar[2] = tr("blended color bar");

  infoChannel[0] = trVDR("no");
  infoChannel[1] = tr("auto display");
  infoChannel[2] = tr("2nd OK");

  replayEnd[0] = tr("length");
  replayEnd[1] = tr("endtime");

  replayCutted[0] = tr("uncutted");
  replayCutted[1] = tr("uncutted and cutted");
  replayCutted[2] = tr("cutted");

  zoomLevel[0] = tr("Auto");
  zoomLevel[1] = tr("Zoom Level 1");
  zoomLevel[2] = tr("Zoom Level 2");
  zoomLevel[3] = tr("Zoom Level 3");

  spacer = "   ";

  Set();
}

void cLCARSNGSetup::Set(void) {

  int currentItem = Current();

#ifdef USE_ZAPCOCKPIT
  int i = 3;
#else
  int i = 2;
#endif

  Clear();

  Add(new cMenuEditIntItem(tr("Margin"),                                                                            &tmpConfig.Margin, 0, 5));
  Add(new cMenuEditBoolItem(tr("Display Error 0"),                                                                  &tmpConfig.displayError0));
  Add(new cMenuEditStraItem(tr("Volume bar style"),                                                                 &tmpConfig.volumeBarStyle, 3, volumeBar));
  Add(new cMenuEditBoolItem(tr("Use scraper infos and pictures"),                                                   &tmpConfig.displayScraperInfo));

  Add(new cOsdItem("",                                                                                              osUnknown, false));
  Add(new cOsdItem(hk(tr("Display channel view:")),                                                                 osUser2));
  if (categorie == 2) {
     Add(new cMenuEditBoolItem(cString::sprintf("%s%s", *spacer, tr("Display poster")),                             &tmpConfig.displChPoster));
     Add(new cMenuEditBoolItem(cString::sprintf("%s%s", *spacer, tr("Swap events and timers")),                     &tmpConfig.swapTimers));
     Add(new cOsdItem("",                                                                                           osUnknown, false));
     }

  Add(new cOsdItem(hk(tr("Display replay view:")),                                                                  osUser3));
  if (categorie == 4) {
     Add(new cMenuEditBoolItem(cString::sprintf("%s%s", *spacer, tr("Display date and time of the recording")),     &tmpConfig.displReplayDateTime));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("Display end of replaying")),                   &tmpConfig.displReplayEnd, 2, replayEnd));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("Display of the times of a recording")),        &tmpConfig.displReplayCutted, 3, replayCutted));
     Add(new cMenuEditBoolItem(cString::sprintf("%s%s", *spacer, tr("Display poster")),                             &tmpConfig.displReplayPoster));
     Add(new cMenuEditIntItem(cString::sprintf("%s%s", *spacer, tr("Display error sign from x errors")),            &tmpConfig.displReplayErrors, 0, 500, trVDR("off")));
#if (APIVERSNUM >= 20605)
     Add(new cMenuEditBoolItem(cString::sprintf("%s%s", *spacer, tr("Display video properties from info file")),    &tmpConfig.displReplayVideoFormat));
#endif
     Add(new cOsdItem("",                                                                                           osUnknown, false));
     }

  Add(new cOsdItem(hk(tr("Display description:")),                                                                  osUser1));
  if (categorie == 1) {
     Add(new cMenuEditBoolItem(cString::sprintf("%s%s", *spacer, tr("In menu epg")),                                &tmpConfig.displInfoMenuEPG));
     Add(new cMenuEditBoolItem(cString::sprintf("%s%s", *spacer, tr("In menu timer")),                              &tmpConfig.displInfoMenuTimer));
     Add(new cMenuEditBoolItem(cString::sprintf("%s%s", *spacer, tr("In menu recording")),                          &tmpConfig.displInfoMenuRec));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("In display channel")),                         &tmpConfig.displInfoChannel, i, infoChannel));
     Add(new cMenuEditIntItem(cString::sprintf("%s%s", *spacer, tr("Max. number of info lines")),                   &tmpConfig.infoChanLines, 5, 20));
     Add(new cMenuEditIntItem(cString::sprintf("%s%s", *spacer, tr("Waiting period Fade-in (ms)")),                 &tmpConfig.waitTimeFadein, 0, 5000, trVDR("off")));
     Add(new cMenuEditIntItem(cString::sprintf("%s%s", *spacer, tr("Fade-in time (ms)")),                           &tmpConfig.fadeinTime, 0, 1000, trVDR("off")));
     Add(new cMenuEditIntItem(cString::sprintf("%s%s", *spacer, tr("Waiting period Scroll (ms)")),                  &tmpConfig.waitTimeScroll, 0, 5000, trVDR("off")));
     Add(new cMenuEditIntItem(cString::sprintf("%s%s", *spacer, tr("ScrollPixel")),                                 &tmpConfig.scrollPixel, 1, 10));
     Add(new cMenuEditIntItem(cString::sprintf("%s%s", *spacer, tr("Refreshrate (Frames/s)")),                      &tmpConfig.framesPerSecond, 10, 60));
     Add(new cMenuEditBoolItem(tr("Use alternate position"),                                                        &tmpConfig.displInfoAlt));
     Add(new cOsdItem("",                                                                                           osUnknown, false));
     }

  Add(new cOsdItem(hk(tr("Menue view:")),                                                                           osUser4));
  if (categorie == 8) {
     Add(new cMenuEditBoolItem(cString::sprintf("%s%s", *spacer, tr("Display poster")),                             &tmpConfig.displMenuPoster));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("main menue")),                                 &tmpConfig.mcMainScaled, 3, menuView));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("schedule menue")),                             &tmpConfig.mcScheduleScaled, 4, menuView));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("event menue")),                                &tmpConfig.mcEventScaled, 4, menuView));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("channel menue")),                              &tmpConfig.mcChannelScaled, 5, menuView));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("timer menue")),                                &tmpConfig.mcTimerScaled, 4, menuView));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("recording menue")),                            &tmpConfig.mcRecordingScaled, 4, menuView));
//     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("recording info menue")),                       &tmpConfig.mcRecordingInfoScaled, 4, menuView));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("setup menue")),                                &tmpConfig.mcSetupScaled, 4, menuView));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("command menue")),                              &tmpConfig.mcCommandScaled, 4, menuView));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("plugin menue")),                               &tmpConfig.mcPluginScaled, 4, menuView));
//     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("Text menue")),                                 &tmpConfig.mcTextScaled));
//     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("Folder menue")),                               &tmpConfig.mcFolderScaled));
//     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("Cam menue")),                                  &tmpConfig.mcCamScaled));
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("Default view")),                               &tmpConfig.mcDefaultScaled, 4, menuView));            
     Add(new cMenuEditStraItem(cString::sprintf("%s%s", *spacer, tr("Zoom level")),                                 &tmpConfig.displZoomLevel, 4, zoomLevel));
     }

#ifdef DRAWGRID
  Add(new cOsdItem("",                                                                                              osUnknown, false));
  Add(new cOsdItem(tr("Debug:"),                                                                                    osUnknown, false));
  Add(new cMenuEditBoolItem(tr("Display grid"),                                                                     &tmpConfig.displayGrid));
#endif

  SetCurrent(Get(currentItem));
  Display();
  SetHelp(NULL, NULL, NULL, NULL);
}

eOSState cLCARSNGSetup::ProcessKey(eKeys Key)
{
  eOSState state = cMenuSetupPage::ProcessKey(Key);

  switch (state) {
    case osUser1: if (categorie != 1)
                     categorie = 1;
                  else
                     categorie = 0;
                  break;
    case osUser2: if (categorie != 2)
                     categorie = 2;
                  else
                     categorie = 0;
                  break;
    case osUser3: if (categorie != 4)
                     categorie = 4;
                  else
                     categorie = 0;
                  break;
    case osUser4: if (categorie != 8)
                     categorie = 8;
                  else
                     categorie = 0;
                  break;
    default:      break;
    }

  if (oldcategorie != categorie) {
     Set();
     oldcategorie = categorie;
     state = osContinue;
     }
  return state;
}

void cLCARSNGSetup::Store(void) {
  Config = tmpConfig;
  SetupStore("Margin",             Config.Margin);
  SetupStore("DisplayError0",      Config.displayError0);
  SetupStore("DisplayScraperInfo", Config.displayScraperInfo);
  SetupStore("DisplInfoMenuEPG",   Config.displInfoMenuEPG);
  SetupStore("DisplInfoMenuTimer", Config.displInfoMenuTimer);
  SetupStore("DisplInfoMenuRec",   Config.displInfoMenuRec);
  SetupStore("DisplInfoChannel",   Config.displInfoChannel);
  SetupStore("InfoChanLines",      Config.infoChanLines);
  SetupStore("WaitTimeFadein",     Config.waitTimeFadein);
  SetupStore("WaitTimeScroll",     Config.waitTimeScroll);
  SetupStore("FadeInTime",         Config.fadeinTime);
  SetupStore("ScrollPixel",        Config.scrollPixel);
  SetupStore("FramesPerSecond",    Config.framesPerSecond);
  SetupStore("DisplInfoAlt",       Config.displInfoAlt);
  SetupStore("VolumeBarStyle",     Config.volumeBarStyle);
  SetupStore("SwapTimers",         Config.swapTimers);
  SetupStore("DisplayErrInRepl",   Config.displReplayErrors);
  SetupStore("DisplMenuPoster",    Config.displMenuPoster);
  SetupStore("DisplChPoster",      Config.displChPoster);
  SetupStore("DisplReplayDateTime", Config.displReplayDateTime);
  SetupStore("DisplReplayEnd",     Config.displReplayEnd);
  SetupStore("DisplReplayCutted",  Config.displReplayCutted);
  SetupStore("DisplReplayPoster",  Config.displReplayPoster);
  SetupStore("DisplReplayVideoFormat", Config.displReplayVideoFormat);
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
  SetupStore("DisplZoomLevel",     Config.displZoomLevel);
  SetupStore("DisplayGrid",        Config.displayGrid);
}
