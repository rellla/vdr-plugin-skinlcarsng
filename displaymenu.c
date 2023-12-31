#include "config.h"
#include "lcarsng.h"
#include "displaymenu.h"
#include "status.h"

cBitmap cLCARSNGDisplayMenu::bmArrowUp(arrowup_xpm);
cBitmap cLCARSNGDisplayMenu::bmArrowDown(arrowdown_xpm);
cBitmap cLCARSNGDisplayMenu::bmTransferMode(play_xpm);

cRect availableRect;
cRect videoWindowRect;

// --- cLCARSNGDisplayMenu -------------------------------------------------

cLCARSNGDisplayMenu::cLCARSNGDisplayMenu(void)
{
  tallFont = cFont::CreateFont(Setup.FontOsd, Setup.FontOsdSize * 1.6);
  initial = true;
  message = false;
  viewmode = efullscreen;
  zoom = 0;
  lastMode = cmUnknown;
  lastChannel = NULL;
  lastEvent = NULL;
  lastRecording = NULL;
  lastSeen = -1;
#if APIVERSNUM < 20301
  lastTimersState = -1;
#endif
  lastSignalDisplay = 0;
  lastLiveIndicatorY = -1;
  lastLiveIndicatorTransferring = false;
  currentTitle = NULL;
  lastcurrentTitle = NULL;
  lastDiskUsageState = -1;
  lastDiskAlert = false;
  lastSystemLoad = -1;
  lastCountRecordings = -1;
  lastNumRecordingsInPath = -1;
  lastCountTimers = -1;
  const cFont *font = cFont::GetFont(fontOsd);
  lineHeight = font->Height();
  tinyFont = CreateTinyFont(lineHeight);
  frameColorFg = Theme.Color(clrMenuFrameFg);
  frameColorBg = Theme.Color(clrMenuFrameBg);
  frameColorBr = (Theme.Color(clrMenuFrameBr) == CLR_BLACK) ? frameColorBg : Theme.Color(clrMenuFrameBr);
  channelFrameColorBr = (Theme.Color(clrChannelFrameBr) == CLR_BLACK) ? Theme.Color(clrChannelFrameBg) : Theme.Color(clrChannelFrameBr);
  replayFrameColorBr = (Theme.Color(clrReplayFrameBr) == CLR_BLACK) ? Theme.Color(clrReplayFrameBg) : Theme.Color(clrReplayFrameBr);
  textColorBg = Theme.Color(clrMenuTextBg);
  currentIndex = -1;
  Margin = Config.Margin;
  drawDescription = NULL;
  volumeBox = NULL;
  lastVolume = statusMonitor->GetVolume();
  lastVolumeTime = time(NULL);

  // The outer frame:
  d = 5 * lineHeight;
  xa09 = cOsd::OsdWidth();
  yb15 = cOsd::OsdHeight();

  xa00 = 0;

  yt00 = 0;
  yt01 = yt00 + lineHeight + 2* Margin;
  yt02 = yt01 + lineHeight + Margin;
  yt03 = yt01 + d / 4;
  yt04 = yt02 + Gap;
  yt05 = yt00 + d / 2;
  yt06 = yt04 + 2 * lineHeight + 2 * Margin;
  yt07 = yt06 + Gap;
  yt08 = yt07 + 2 * lineHeight + 2 * Margin;
  yt09 = yt08 + Gap;
  yt10 = yt09 + 2 * lineHeight + 2 * Margin;

  // The color buttons in the main menu:
  int r = lineHeight;
  xd07 = xa09 - Gap;
  xd08 = xd07; // right border of DrawLive()/ DrawPlay()
  xd00 = xd07 - Gap - 16 * r;
  if ((float)(xd07 - xd00) / (float)xa09 > 0.33)
     zoom = 1;
  if ((float)(xd07 - xd00) / (float)xa09 > 0.4)
     zoom = 2;
  if ((float)(xd07 - xd00) / (float)xa09 > 0.52)
     zoom = 3;

  cLCARSNGDisplayMenu::SetCoordinateY(yt10);

  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop(), xa00, yt00, xa09 - 1, yb15 - 1);
}

cLCARSNGDisplayMenu::~cLCARSNGDisplayMenu()
{
  delete drawDescription;
  delete volumeBox;
  delete tallFont;
  delete tinyFont;
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayMenu::SetCoordinateY(int y)
{
  // The outer frame:

  xa01 = xa00 + d / 2 + Margin;
  xa02 = xa00 + d + Margin;
  xa03 = xa02 + lineHeight;
  xa04 = xa02 + d / 4;
  xa05 = xa02 + d - (zoom * lineHeight);
  xa06 = xa05 + Gap;
  xa08 = xa09 - lineHeight;
  xa07 = xa08 - Gap;

  yc00 = y + Gap;
  yc05 = yc00 + 3 * lineHeight + Gap / 2; // Button in der Mitte
  yc04 = yc05 - lineHeight - Margin;
  yc03 = yc04 - lineHeight - Margin;
  yc02 = yc04 - d / 4;
  yc01 = yc05 - d / 2;

  yc06 = yc05 + Gap; // Button in der Mitte
  yc07 = yc06 + lineHeight + Margin;
  yc08 = yc07 + lineHeight + Margin;
  yc09 = yc07 + d / 4;
  yc10 = yc06 + d / 2;
  yc11 = yc06 + 3 * lineHeight + Gap / 2;

  yb00 = yc11 + Gap;
  yb01 = yb00 + 2 * lineHeight + 2 * Margin; //Date Time
  yb02 = yb01 + Gap;
  yb03 = yb02 + 2 * lineHeight + 2 * Margin; // Free
  yb04 = yb03 + Gap;
  yb05 = yb04 + 2 * lineHeight + 2 * Margin; // Load %
  yb06 = yb05 + Gap;
  yb07 = yb06 + 2 * lineHeight + 2 * Margin; //Recordings
  yb08 = yb07 + Gap;
  yb081 = yb08 + 2 * lineHeight + 2 * Margin; //Timer
  yb082 = yb081 + Gap;

  yb14 = yb15 - lineHeight - 2 * Margin;
  yb13 = yb14 - lineHeight - Margin;
  yb12 = yb14 - d / 4;
  yb11 = yb15 - d / 2;
  yb10 = yb13 - lineHeight - Margin; // VDR
  yb09 = yb10 - Gap;

  // Compensate for large font size:
  if (yb09 - yb082 < lineHeight) {
     yb081 = yb09;
     yb082 = 0; // drop empty rectangle
     }
  if (yb09 - yb08 < 2 * lineHeight) {
     yb07 = yb09;
     yb08 = 0; // drop "TIMER" display
     }
  if (yb09 - yb06 < 2 * lineHeight) {
     yb05 = yb09;
     yb06 = 0; // drop "RECORDINGS" display
     }
  if (yb09 - yb04 < 2 * lineHeight) {
     yb03 = yb09;
     yb04 = 0; // drop "LOAD" display
     }
  if (yb09 - yb02 < 2 * lineHeight) {
     yb01 = yb09;
     yb02 = 0; // drop "DISK" display
     }
  // Anything else is just insanely large...

  // The main command menu:
  xm00 = xa03;
  xm01 = xa05;
  xm02 = xm01 + Gap;
  xm08 = (xa09 + xa00) / 2;
  xm07 = xm08 - lineHeight - 2 * Margin;
  xm06 = xm07 - lineHeight / 2;
  xm05 = xm06 - lineHeight / 2;
  xm04 = xm05 - lineHeight;
  xm03 = xm04 - Gap;
  ym00 = yc08; // Bezug Menü oben
  ym01 = ym00 + lineHeight / 2;
  ym02 = ym01 + lineHeight / 2;
  ym03 = ym02 + Gap;
  ym07 = yb15;
  ym06 = ym07 - lineHeight / 2;
  ym05 = ym06 - lineHeight / 2;
  ym04 = ym05 - Gap;

  // The status area:
  xs00 = xm08 + Gap + lineHeight + Gap;
  xs13 = xa09; //cOsd::OsdWidth()
  xs12 = xa08;
  xs11 = xa07;
  xs05 = (xs00 + xs11 + Gap) / 2;
  xs04 = xs05 - lineHeight / 2;
  xs03 = xs04 - lineHeight / 2;
  xs02 = xs03 - 2 * lineHeight;
  xs01 = xs02 - Gap;
  xs06 = xs05 + Gap;
  xs07 = xs06 + lineHeight / 2;
  xs08 = xs07 + lineHeight / 2;
  xs09 = xs08 + 2 * lineHeight;
  xs10 = xs09 + Gap;
  ys00 = yc06; // Bezug Status oben
  ys01 = ys00 + lineHeight + Margin;
  ys02 = ys01 + lineHeight / 2;
  ys04 = ys01 + lineHeight;
  ys03 = ys04 - Gap;
  ys05 = yb15;

  // The item area (just to have them initialized, actual setting will be done in SetMenuCategory():

  xi00 = 0;
  xi01 = 0;
  xi02 = 0;
  xi03 = 1;
  yi00 = 0;
  yi01 = 1;

  // The color buttons in submenus:
  xb00 = xa06;
  xb15 = xa07;
  int w = (xa08 - xa06) / 4;
  xb01 = xb00 + lineHeight / 2;
  xb02 = xb01 + Gap;
  xb04 = xb00 + w;
  xb03 = xb04 - Gap;
  xb05 = xb04 + lineHeight / 2;
  xb06 = xb05 + Gap;
  xb08 = xb04 + w;
  xb07 = xb08 - Gap;
  xb09 = xb08 + lineHeight / 2;
  xb10 = xb09 + Gap;
  xb12 = xb08 + w;
  xb11 = xb12 - Gap;
  xb13 = xb12 + lineHeight / 2;
  xb14 = xb13 + Gap;

  // The color buttons in the main menu:
  int xd = (viewmode == escaledvideo) ? (zoom) ? xm08 : xm05 : xa09;
  int yd = (MenuCategory() == mcChannel && viewmode == esmalscreen) ? yb15 : yc04;
  int r = lineHeight;
  xd07 = xd - Gap;
  xd08 = xa09 - Gap;
  int fac = (zoom > 1) ? 0.75 : 1;
  xd06 = xd07 - (fac * 0.5 * r);
  xd05 = xd06 - (7 - zoom - ((zoom > 1) ? 1 : 0)) * r;
  xd04 = xd05 - (fac * 0.5 * r);
  xd03 = xd04 - Gap;
  xd02 = xd03 - (fac * 0.5 * r);
  xd01 = xd02 - (7 - zoom - ((zoom > 1) ? 1 : 0)) * r;
  xd00 = xd01 - (fac * 0.5 * r);
  yd00 = yt00;
  yd05 = yd - 3 * Gap;
  yd04 = yd05 - fac * r; // Button in der Mitte
  yd03 = yd04 - Gap;
  yd02 = yd03 - fac * r; // Button in der Mitte
  yd01 = yd02 - Gap;

  xs = 0;
}

void cLCARSNGDisplayMenu::SetMenuCategory(eMenuCategory MenuCategory)
{
  if (initial || MenuCategory != cSkinDisplayMenu::MenuCategory()) {
     cSkinDisplayMenu::SetMenuCategory(MenuCategory);
     initial = true;
     viewmode = efullscreen;
     lastLiveIndicatorY = -1;
     SetCoordinateY(yt10);
     osd->DrawRectangle(xa00, yt00, xa09 - 1, yb15 - 1, Theme.Color(clrBackground));
     switch (MenuCategory) {
        case mcChannel:
           if (MenuCategory == mcChannel) {
              viewmode = Config.mcChannelScaled;
              }
//        case mcChannelEdit:
//           if (MenuCategory == mcChannelEdit) {
//              viewmode = Config.mcChannelScaled;
//              }
        case mcMain:
           if (MenuCategory == mcMain) {
              viewmode = Config.mcMainScaled;
              }
        case mcSetup:
           if (MenuCategory == mcSetup) {
              viewmode = Config.mcSetupScaled;
              }
        case mcCommand:
           if (MenuCategory == mcCommand) {
              viewmode = Config.mcCommandScaled;
              }
        case mcSchedule:
           if (MenuCategory == mcSchedule) {
              viewmode = Config.mcScheduleScaled;
              }
        case mcScheduleNow:
           if (MenuCategory == mcScheduleNow) {
              viewmode = Config.mcScheduleScaled;
              }
        case mcScheduleNext:
           if (MenuCategory == mcScheduleNext) {
              viewmode = Config.mcScheduleScaled;
              }
        case mcEvent:
           if (MenuCategory == mcEvent) {
              viewmode = Config.mcEventScaled;
              }
        case mcRecording:
           if (MenuCategory == mcRecording) {
              viewmode = Config.mcRecordingScaled;
              }
        case mcRecordingInfo:
           if (MenuCategory == mcRecordingInfo) {
              viewmode = Config.mcRecordingScaled;
              }
        case mcRecordingEdit:
           if (MenuCategory == mcRecordingEdit) {
              viewmode = Config.mcRecordingScaled;
              }
        case mcTimer:
           if (MenuCategory == mcTimer) {
              viewmode = Config.mcTimerScaled;
              }
        case mcTimerEdit:
           if (MenuCategory == mcTimerEdit) {
              viewmode = Config.mcTimerScaled;
              }
           break;
        default:
           viewmode = Config.mcDefaultScaled;
        }
     if (MenuCategory == mcChannel && viewmode == esmalscreen) {
        SetCoordinateY(yt06);
        osd->DrawRectangle(xa00, yt00, xa09 - 1, yb15 - 1, clrTransparent);
        yi00 = yt04 + lineHeight;
        yi01 = ym04;
        xi00 = xm00;
        xi01 = xm03;
        xi02 = xm04;
        xi03 = xm05;
        DrawMainFrameChannel();
        DrawMainBracket();
        }
     else if ((MenuCategory ==  mcMain) || ((MenuCategory == mcSetup) && viewmode != efullscreen)) {
        if (viewmode != escaledvideo) {
           (zoom) ? SetCoordinateY(yt06) : SetCoordinateY(yt08);
           }
        if (viewmode == escaledvideo) {
           (zoom) ? (zoom > 1) ? SetCoordinateY(yt06) : SetCoordinateY(yt08) : SetCoordinateY(yt10);
           }
        if (viewmode != efullscreen || (MenuCategory == mcMain)) {
           if (viewmode == escaledvideo)
              osd->DrawRectangle(xs00, 0, xa09, yc06 - 1, clrTransparent);
           yi00 = ym03;
           yi01 = ym04;
           xi00 = xm00;
           xi01 = xm03;
           xi02 = xm04;
           xi03 = xm05;
#if APIVERSNUM < 20301
           lastTimersState = -1;
#else
           timersStateKey.Reset();
#endif
           DrawMainFrameLower();
           DrawMainBracket();
           DrawStatusElbows();
           }
        }
     else if (viewmode == efullscreen) {
        SetCoordinateY(yt06);
        yi00 = yt02;
        yi01 = yb13;
        xi00 = xa03;
        xi01 = xa07;
        xi02 = xa08;
        xi03 = xa09;
        DrawMenuFrame();
        }
     else {
        if (viewmode == esplitscreen) {
           (zoom) ? SetCoordinateY(yt06) : SetCoordinateY(yt08);
           }
        if (viewmode == escaledvideo) {
           (zoom) ? (zoom > 1) ? SetCoordinateY(yt06) : SetCoordinateY(yt08) : SetCoordinateY(yt10);
           }
        if (viewmode == escaledvideo)
           osd->DrawRectangle( xs00, 0, xa09, yc06 - 1, clrTransparent);
        yi00 = ym00;
        yi01 = ym07;
        xi00 = xa03;
        xi01 = xa07;
        xi02 = xa08;
        xi03 = xa09;
        DrawMainFrameLower();
        DrawMainBracket();
        }
     }
}

void cLCARSNGDisplayMenu::DrawMainFrameUpper(tColor Color, tColor ColorBg)
{
  // Top left rectangles:
  DrawRectangleOutline(osd, xa00, yt00, xa02 - 1, yt02 - 1, Color, ColorBg, 15);
  DrawRectangleOutline(osd, xa00, yt04, xa02 - 1, yt06 - 1, Color, ColorBg, 15);
  if (zoom < 2)
     DrawRectangleOutline(osd, xa00, yt07, xa02 - 1, yt08 - 1, Color, ColorBg, 15);
  if ((zoom < 1) && (!(viewmode == esplitscreen || (!(viewmode == escaledvideo) && (MenuCategory() == mcMain)))))
     DrawRectangleOutline(osd, xa00, yt09, xa02 - 1, yt10 - 1, Color, ColorBg, 15);
  // Upper elbow:
  DrawRectangleOutline(osd, xa00, yc00, xa01 - 1, yc01 - 1, Color, ColorBg, 3);
  DrawRectangleOutline(osd, xa01, yc00, xa02 - 1, yc05 - 1, Color, ColorBg, 14);
  DrawRectangleOutline(osd, xa02 - Margin, yc04, xa05 - 1, yc05 - 1, Color, ColorBg, 14);
  osd->DrawEllipse  (xa00, yc01, xa01 - 1, yc05 - 1, Color, 3);
  osd->DrawEllipse  (xa00 + Margin, yc01, xa01 - 1, yc05 - 1 - Margin, ColorBg, 3);
  osd->DrawEllipse  (xa02, yc02, xa04 - 1, yc04 - 1, Color, -3);
  osd->DrawEllipse  (xa02 - Margin, yc02, xa04 - 1, yc04 - 1 + Margin, ColorBg, -3);
  if (viewmode == esplitscreen || (viewmode == efullscreen && (MenuCategory() == mcMain))) {
     // Upper delimiter:
     DrawRectangleOutline(osd, xa06, yc04 + lineHeight / 2, xm08 - 1, yc05 - 1, Color, ColorBg, 15);
     // Top right rectangles:
     DrawRectangleOutline(osd, xm08 + Gap, yc04, xs00 - Gap - 1, yc05 - 1, Color, ColorBg, 15);
     if (viewmode == esplitscreen && (MenuCategory() != mcSetup)) {
        DrawRectangleOutline(osd, xs00, yc04, xs11 - 1, yc05 - 1, Color, ColorBg, 15);
        }
     else {
        DrawRectangleOutline(osd, xs00, yc04, xs05 - 1, yc05 - 1, Color, ColorBg, 15);
        DrawRectangleOutline(osd, xs06, yc04, xa07 - 1, yc05 - 1, Color, ColorBg, 15);
        }
     DrawRectangleOutline(osd, xa08, yc04, xa09 - 1, yc05 - 1, Color, ColorBg, 15);
     }
  else {
     // Upper delimiter:
     DrawRectangleOutline(osd, xa06, yc04 + lineHeight / 2, xm03 - 1, yc05 - 1, Color, ColorBg, 15);
     // Top right rectangles:
     int x = (zoom) ? xs00 - xm08 : 0;
     DrawRectangleOutline(osd, xm07 + x, yt00, xm08 + x - 1, yc04 - Gap - 1, Color, ColorBg, 15);
     DrawRectangleOutline(osd, xm04, yc04 + lineHeight / 2, xm07 + x + Margin, yc05 - 1, Color, ColorBg, 11);
     DrawRectangleOutline(osd, xm07 + x, yc04, xm07 + x + lineHeight / 2, yc04 + lineHeight / 2, Color, ColorBg, 3);
     osd->DrawEllipse  (xm07 + x + Margin, yc04, xm08 + x - 1, yc05 - 1, Color, 4);
     osd->DrawEllipse  (xm07 + x + Margin, yc04 + Margin, xm08 + x - 1 - Margin, yc05 - 1 - Margin, ColorBg, 4);
     osd->DrawEllipse  (xm06 + x, yc04, xm07 + x, yc04 + lineHeight / 2, Color, -4);
     osd->DrawEllipse  (xm06 + x, yc04 + Margin, xm07 + x + Margin, yc04 + lineHeight / 2 + Margin, ColorBg, -4);
     }
}

void cLCARSNGDisplayMenu::DrawMainFrameLower(void)
{
// Mitte unterer Ellenbogen
  const cFont *font = cFont::GetFont(fontOsd);
  // Lower elbow:
  DrawRectangleOutline(osd, xa00, yc10, xa01 - 1, yc11 - 1, frameColorBr, frameColorBg, 9);
  DrawRectangleOutline(osd, xa01, yc06, xa02 - 1, yc11 - 1, frameColorBr, frameColorBg, 14);
  DrawRectangleOutline(osd, xa02 - Margin, yc06, xa05 - 1, yc07 - 1, frameColorBr, frameColorBg, 14);
  osd->DrawEllipse  (xa00, yc06, xa01 - 1, yc10 - 1, frameColorBr, 2);
  osd->DrawEllipse  (xa00 + Margin, yc06 + Margin, xa01 - 1, yc10 - 1, frameColorBg, 2);
  osd->DrawEllipse  (xa02, yc07, xa04 - 1, yc09 - 1, frameColorBr, -2);
  osd->DrawEllipse  (xa02 - Margin, yc07 - Margin, xa04 - 1, yc09 - 1, frameColorBg, -2);
  // Lower delimiter:
  if (!viewmode == escaledvideo && (MenuCategory() == mcMain)) {
     DrawRectangleOutline(osd, xa06, yc06, xm08 - 1, yc07 - lineHeight / 2 - 1, frameColorBr, frameColorBg, 15);
     }
  else if (viewmode == esplitscreen && !(MenuCategory() == mcRecording)) {
     DrawRectangleOutline(osd, xa06, yc06, xm08 - 1, yc07 - lineHeight / 2 - 1, frameColorBr, frameColorBg, 15);
     }
  else {
     DrawRectangleOutline(osd, xa06, yc06, xm03 - 1, yc07 - lineHeight / 2 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xm04, yc06, xm08 - 1, yc07 - 1, frameColorBr, frameColorBg, 15);
     }
  DrawRectangleOutline(osd, xm08 + Gap, yc06, xs00 - Gap - 1, yc07 - 1, frameColorBr, frameColorBg, 15);
  // VDR version:
  osd->DrawRectangle(xa00, yb10, xa02 - 1, yb15 - 1, frameColorBr);
  osd->DrawText(xa00 + Margin, yb10 + Margin, cString::sprintf("%s-%s", "VDR", VDRVERSION), frameColorFg, frameColorBg, font, xa02 - xa00 - 2 * Margin, yb15 - yb10 - 2 * Margin, taTop | taRight | taBorder);
  osd->DrawText(xa00 + Margin, yb15 - lineHeight - Margin, "LCARSNG", frameColorFg, frameColorBg, font, xa02 - xa00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);
}

void cLCARSNGDisplayMenu::DrawMainFrameChannel(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  // Upper elbow:
  DrawRectangleOutline(osd, xa00, yt05, xa01 - 1, yt06 - 1, frameColorBr, frameColorBg, 9);
  DrawRectangleOutline(osd, xa01, yt00, xa02 - 1, yt06 - 1, frameColorBr, frameColorBg, 14);
  DrawRectangleOutline(osd, xa02  - Margin, yt00, xa05 - 1, yt01 - 1, frameColorBr, frameColorBg, 14);
  osd->DrawEllipse  (xa00, yt00, xa01 - 1, yt05 - 1, frameColorBr, 2);
  osd->DrawEllipse  (xa00 + Margin, yt00 + Margin, xa01 - 1, yt05 - 1, frameColorBg, 2);
  osd->DrawEllipse  (xa02, yt01, xa04 - 1, yt03 - 1, frameColorBr, -2);
  osd->DrawEllipse  (xa02 - Margin, yt01 - Margin, xa04 - 1, yt03 - 1, frameColorBg, -2);
  DrawRectangleOutline(osd, xm02, yt00, xm03 -1, yt01 - 1, frameColorBr, frameColorBg, 15);
  DrawRectangleOutline(osd, xm04, yt00, xm07 - Gap - 1, yt01 - 1, frameColorBr, frameColorBg, 15);
  DrawRectangleOutline(osd, xm07, yt00, xm07 + lineHeight / 2 -1, yt01 - 1, frameColorBr, frameColorBg, 11);
  osd->DrawEllipse  (xm07 + lineHeight / 2, yt00, xm08 - 1, yt01 - 1, frameColorBr, 5);
  osd->DrawEllipse  (xm07 + lineHeight / 2, yt00 + Margin, xm08 - 1 - Margin, yt01 - 1 - Margin, frameColorBg, 5);
  // Center part:
  DrawRectangleOutline(osd, xa00, yc00, xa02 - 1, yc11 - 1, frameColorBr, frameColorBg, 15);
//  DrawRectangleOutline(osd, xa00, yt06 + Gap, xa02 - 1, yb00 - 1 - Gap, frameColorBr, frameColorBg, 15);
  // VDR version:
  osd->DrawRectangle(xa00, yb10, xa02 - 1, yb15 - 1, frameColorBr);
  osd->DrawText(xa00 + Margin, yb10 + Margin, cString::sprintf("%s-%s", "VDR", VDRVERSION), frameColorFg, frameColorBg, font, xa02 - xa00 - 2 * Margin, yb15 - yb10 - 2 * Margin, taTop | taRight | taBorder);
  osd->DrawText(xa00 + Margin, yb15 - lineHeight - Margin, "LCARSNG", frameColorFg, frameColorBg, font, xa02 - xa00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);
}

void cLCARSNGDisplayMenu::DrawMainButton(const char *Text, int x0, int x1, int x2, int x3, int y0, int y1, tColor ColorFg, tColor ColorBg, const cFont *Font)
{
  int h = y1 - y0;
  osd->DrawEllipse(x0, y0, x1 - 1, y1 - 1, ColorBg, 7);
  osd->DrawText(x1, y0, Text, ColorFg, ColorBg, Font, x2 - x1, h, taCenter);
  osd->DrawEllipse(x2, y0, x3 - 1, y1 - 1, ColorBg, 5);
}

void cLCARSNGDisplayMenu::DrawMenuFrame(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  // Upper elbow:
  DrawRectangleOutline(osd, xa00, yt05, xa01 - 1, yt06 - 1, frameColorBr, frameColorBg, 9);
  DrawRectangleOutline(osd, xa01, yt00, xa02 - 1, yt06 - 1, frameColorBr, frameColorBg, 14);
  DrawRectangleOutline(osd, xa02 - Margin, yt00, xa05 - 1, yt01 - 1, frameColorBr, frameColorBg, 14);
  osd->DrawEllipse  (xa00, yt00, xa01 - 1, yt05 - 1, frameColorBr, 2);
  osd->DrawEllipse  (xa00 + Margin, yt00 + Margin, xa01 - 1, yt05 - 1, frameColorBg, 2);
  osd->DrawEllipse  (xa02, yt01, xa04 - 1, yt03 - 1, frameColorBr, -2);
  osd->DrawEllipse  (xa02 - Margin, yt01 - Margin, xa04 - 1, yt03 - 1, frameColorBg, -2);
  DrawRectangleOutline(osd, xa08, yt00, xa08 + lineHeight / 2 - 1, yt01 - 1, frameColorBr, frameColorBg, 11);
  osd->DrawRectangle(xa08 + lineHeight / 2, yt00, xa09 - 1, yt00 + lineHeight / 2 - 1, clrTransparent);
  osd->DrawEllipse  (xa08 + lineHeight / 2, yt00, xa09 - 1, yt01 - 1, frameColorBr, 5);
  osd->DrawEllipse  (xa08 + lineHeight / 2, yt00 + Margin, xa09 - 1 - Margin, yt01 - 1 - Margin, frameColorBg, 5);
  // Center part:
  DrawRectangleOutline(osd, xa00, yc00, xa02 - 1, yc11 - 1, frameColorBr, frameColorBg, 15);
//  DrawRectangleOutline(osd, xa00, yt06 + Gap, xa02 - 1, yb00 - 1 - Gap, frameColorBr, frameColorBg, 15);
  // Lower elbow:
  DrawRectangleOutline(osd, xa00, yb10, xa01 - 1, yb11 - 1, frameColorBr, frameColorBg, 3);
  DrawRectangleOutline(osd, xa01, yb10, xa02 - 1, yb15 - 1, frameColorBr, frameColorBg, 14);
  DrawRectangleOutline(osd, xa02 - Margin, yb14, xa05 - 1, yb15 - 1, frameColorBr, frameColorBg, 14);
  osd->DrawEllipse  (xa00, yb11, xa01 - 1, yb15 - 1, frameColorBr, 3);
  osd->DrawEllipse  (xa00 + Margin, yb11, xa01 - 1, yb15 - 1 - Margin, frameColorBg, 3);
  osd->DrawEllipse  (xa02, yb12, xa04 - 1, yb14 - 1, frameColorBr, -3);
  osd->DrawEllipse  (xa02 - Margin, yb12, xa04 - 1, yb14 - 1 + Margin, frameColorBg, -3);
  DrawRectangleOutline(osd, xa08, yb14, xa08 + lineHeight / 2 - 1, yb15 - 1, frameColorBr, frameColorBg, 11);
  osd->DrawRectangle(xa08 + lineHeight / 2, yb14 + lineHeight / 2, xa09 - 1, yb15 - 1, clrTransparent);
  osd->DrawEllipse  (xa08 + lineHeight / 2, yb14, xa09 - 1, yb15 - 1, frameColorBr, 5);
  osd->DrawEllipse  (xa08 + lineHeight / 2, yb14 + Margin, xa09 - 1  - Margin, yb15 - 1 - Margin, frameColorBg, 5);
  osd->DrawText(xa00 + Margin, yb10 + Margin, cString::sprintf("%s-%s", "VDR", VDRVERSION), frameColorFg, frameColorBg, font, xa02 - xa00 - 2 * Margin, yb11 - yb10 - Margin, taTop | taRight | taBorder);
  // Color buttons:
  tColor lutBg[] = { clrButtonRedBg, clrButtonGreenBg, clrButtonYellowBg, clrButtonBlueBg };
  osd->DrawRectangle(xb00, yb14, xb01 - 1, yb15 - 1, Theme.Color(lutBg[Setup.ColorKey0]));
  osd->DrawRectangle(xb04, yb14, xb05 - 1, yb15 - 1, Theme.Color(lutBg[Setup.ColorKey1]));
  osd->DrawRectangle(xb08, yb14, xb09 - 1, yb15 - 1, Theme.Color(lutBg[Setup.ColorKey2]));
  osd->DrawRectangle(xb12, yb14, xb13 - 1, yb15 - 1, Theme.Color(lutBg[Setup.ColorKey3]));
}

void cLCARSNGDisplayMenu::DrawDate(void)
{
  cString s = DayDateTime();
  if (initial)
     DrawRectangleOutline(osd, xa00, yb00, xa02 - 1, yb01 - 1, frameColorBr, frameColorBg, 15);
  if (initial || !*lastDate || strcmp(s, lastDate)) {
     const cFont *font = cFont::GetFont(fontOsd);
     tColor ColorFg = Theme.Color(clrDateFg);
     tColor ColorBg = Theme.Color(clrDateBg);
     lastDate = s;
     const char *t = strrchr(s, ' ');
     osd->DrawText(xa00 + Margin, yb01 - lineHeight - Margin, t, ColorFg, ColorBg, font, xa02 - xa00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);
     s.Truncate(t - s);
     osd->DrawText(xa00 + Margin, yb00 + Margin, s, ColorFg, ColorBg, font, xa02 - xa00 - 2 * Margin, lineHeight, taTop | taRight | taBorder);
     }
}

void cLCARSNGDisplayMenu::DrawDisk(void)
{
  if (yb02) {
     if (cVideoDiskUsage::HasChanged(lastDiskUsageState) || initial || currentTitle != lastcurrentTitle) { // must call HasChanged() first, or it shows an outdated value in the 'initial' case!
        const cFont *font = cFont::GetFont(fontOsd);
        int DiskUsage = cVideoDiskUsage::UsedPercent();
        bool DiskAlert = DiskUsage > DISKUSAGEALERTLIMIT;
        tColor ColorFg = DiskAlert ? Theme.Color(clrAlertFg) : frameColorFg;
        tColor ColorBg = DiskAlert ? Theme.Color(clrAlertBg) : frameColorBg;
        int minutes = FreeMB(currentTitle, (MenuCategory() == mcRecording));
        lastcurrentTitle = currentTitle;
        if (initial || DiskAlert != lastDiskAlert) {
           DrawRectangleOutline(osd, xa00, yb02, xa02 - 1, yb03 - 1, frameColorBr, frameColorBg, 15);
           osd->DrawText(xa00 + Margin, yb02 + Margin, tr("DISK"), ColorFg, ColorBg, tinyFont, xa02 - xa00 - 2 * Margin, yb03 - yb02 - 2 * Margin, taTop | taLeft | taBorder);
           }
        osd->DrawText(xa01, yb02 + Margin, cString::sprintf("%02d%s", DiskUsage, "%"), ColorFg, ColorBg, font, xa02 - xa01 - Margin, lineHeight, taBottom | taRight | taBorder);
        osd->DrawText(xa00 + Margin, yb03 - lineHeight - Margin, cString::sprintf("%02d:%02d", minutes / 60, minutes % 60), ColorFg, ColorBg, font, xa02 - xa00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);
        lastDiskAlert = DiskAlert;
        }
     }
}

void cLCARSNGDisplayMenu::DrawLoad(void)
{
  if (yb04) {
     tColor ColorFg = frameColorFg;
     tColor ColorBg = frameColorBg;
     if (initial) {
        DrawRectangleOutline(osd, xa00, yb04, xa02 - 1, yb05 - 1, frameColorBr, frameColorBg, 15);
        osd->DrawText(xa00 + Margin, yb04 + Margin, tr("LOAD"), ColorFg, ColorBg, tinyFont, xa02 - xa00 - 2 * Margin, yb05 - yb04 - 2 * Margin, taTop | taLeft | taBorder);
        }
     double SystemLoad;
     if (getloadavg(&SystemLoad, 1) > 0) {
        if (initial || SystemLoad != lastSystemLoad) {
           osd->DrawText(xa00 + Margin, yb05 - lineHeight - Margin, cString::sprintf("%.1f", SystemLoad), ColorFg, ColorBg, cFont::GetFont(fontOsd), xa02 - xa00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);
           lastSystemLoad = SystemLoad;
           }
        }
     }
}

void cLCARSNGDisplayMenu::DrawNumRecordingsInPath(void)
{
#if APIVERSNUM > 20301
#ifdef NUMRECORDINGSINPATH
  if (cMenuRecordings::IsDelRecMenu()) {
     osd->DrawRectangle(xm04 + Margin, yc06 + Margin, xm08 - Margin - 1, yc07 - Margin - 1, frameColorBg);
     lastNumRecordingsInPath = -1;
     return;
     }

  const cFont *font = cFont::GetFont(fontOsd);
  int NumRecordingsInPath = 0;
  {
  LOCK_RECORDINGS_READ;
  NumRecordingsInPath = Recordings->GetNumRecordingsInPath(cMenuRecordings::GetActualPath());
  }
  if (NumRecordingsInPath > 0)
     if (initial || NumRecordingsInPath != lastNumRecordingsInPath) {
        osd->DrawText(xm04 + Margin, yc06 + Margin, cString::sprintf("%i", NumRecordingsInPath), frameColorFg, frameColorBg, font, xm08 - xm04 - 1 - 2 * Margin, lineHeight - Margin, taBottom | taRight | taBorder);
        lastNumRecordingsInPath = NumRecordingsInPath;
        }
#endif
#endif
}

void cLCARSNGDisplayMenu::DrawCountRecordings(void)
{
  if (yb06) {
     const cFont *font = cFont::GetFont(fontOsd);
     tColor ColorFg = frameColorFg;
     tColor ColorBg = frameColorBg;
     int CountRecordings = 0;
     if (initial) {
        DrawRectangleOutline(osd, xa00, yb06, xa02 - 1, yb07 - 1, frameColorBr, frameColorBg, 15);
        osd->DrawText(xa00 + Margin, yb06 + Margin, tr("RECORDINGS"), ColorFg, ColorBg, tinyFont, xa02 - xa00 - 2 * Margin, yb07 - yb06 - 2 * Margin, taTop | taLeft | taBorder);
        }
     {
#if APIVERSNUM > 20300
     LOCK_RECORDINGS_READ;
#endif
     CountRecordings = Recordings->Count();
     }
     if (initial || CountRecordings != lastCountRecordings) {
        osd->DrawText(xa00 + Margin, yb07 - lineHeight - Margin, cString::sprintf("%i", CountRecordings), ColorFg, ColorBg, font, xa02 - xa00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);
        lastCountRecordings = CountRecordings;
        }
     }
}

void cLCARSNGDisplayMenu::DrawCountTimers(void)
{
  if (yb08) {
     const cFont *font = cFont::GetFont(fontOsd);
     tColor ColorFg = frameColorFg;
     tColor ColorBg = frameColorBg;
     int CountTimers = 0;
     if (initial) {
        DrawRectangleOutline(osd, xa00, yb08, xa02 - 1, yb081 - 1, frameColorBr, frameColorBg, 15);
        osd->DrawText(xa00 + Margin, yb08 + Margin, tr("TIMER"), ColorFg, ColorBg, tinyFont, xa02 - xa00 - 2 * Margin, yb081 - yb08 - 2 * Margin, taTop | taLeft | taBorder);
        }
#if APIVERSNUM > 20300
     LOCK_TIMERS_READ;
     for (const cTimer *Timer = Timers->First(); Timer; Timer = Timers->Next(Timer)) {
#else
     for (cTimer *Timer = Timers.First(); Timer; Timer = Timers.Next(Timer)) {
#endif
        if (Timer->HasFlags(tfActive))
           CountTimers++;
        }
     if (initial || CountTimers != lastCountTimers) {
        osd->DrawText(xa00 + Margin, yb081 - lineHeight - Margin, itoa(CountTimers), ColorFg, ColorBg, font, xa02 - xa00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);
        lastCountTimers = CountTimers;
        }
     }
}

void cLCARSNGDisplayMenu::DrawMainBracket(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  tColor Color = Theme.Color(clrMenuMainBracket);
  int y0, y1, y2, y3;
  if (MenuCategory() == mcChannel && viewmode == esmalscreen) {
     y0 = yt00 + lineHeight *2;
     y1 = y0 + lineHeight / 2;
     y2 = y1 + lineHeight / 2;
     y3 = y2 + Gap;
     }
  else {
     y0 = ym00;
     y1 = ym01;
     y2 = ym02;
     y3 = ym03;
     }
  if (MenuCategory() == mcMain || MenuCategory() == mcSetup || (MenuCategory() == mcChannel && viewmode == esmalscreen)) {
     osd->DrawRectangle(xm00, y0, xm01 - 1, y1 - 1, Color);
     if (MenuCategory() == mcMain) {
        osd->DrawRectangle(xm02, y0, xm07 - 1, y1 - 1, Color);
        }
     else {
        osd->DrawRectangle(xm02, y0, xm03 - 1, y1 - 1, Color);
        osd->DrawRectangle(xm04, y0, xm07 - 1, y1 - 1, Color);
        }
     osd->DrawEllipse  (xm07, y0, xm08 - 1, y2 - 1, Color, 1);
     osd->DrawEllipse  (xm06, y1, xm07 - 1, y2 - 1, Color, -1);
     osd->DrawRectangle(xm07, y3, xm08 - 1, ym04 - 1, Color);
     osd->DrawEllipse  (xm06, ym05, xm07 - 1, ym06 - 1, Color, -4);
     osd->DrawEllipse  (xm07, ym05, xm08 - 1, ym07 - 1, Color, 4);
     if (MenuCategory() == mcMain) {
        osd->DrawRectangle(xm02, ym06, xm07 - 1, ym07 - 1, Color);
        }
     else {
        osd->DrawRectangle(xm02, ym06, xm03 - 1, ym07 - 1, Color);
        osd->DrawRectangle(xm04, ym06, xm07 - 1, ym07 - 1, Color);
        }
     osd->DrawRectangle(xm00, ym06, xm01 - 1, ym07 - 1, Color);
     }
  if (MenuCategory() == mcSetup) {
     DrawRectangleOutline(osd, xm02, ys00, xm08 - 1, ys01 - 1, frameColorBr, frameColorBg, 15);
     osd->DrawText(xm02 + Margin, ys00 + Margin, tr("Setup"), frameColorFg, frameColorBg, font, xm08 - xm02 - 2 * Margin, lineHeight - Margin, taBottom | taLeft | taBorder);
     }
//  if (MenuCategory() == mcCommand)
//     osd->DrawText(xm02, ys00, tr("Commands"), Theme.Color(clrMenuFrameFg), frameColor, font, xm04 - xm02 - Gap, lineHeight, taBottom | taLeft | taBorder);
  if (MenuCategory() == mcChannel && viewmode == esmalscreen)
     osd->DrawText(xm02 + Margin, yt00 + Margin, tr("Channels"), frameColorFg, frameColorBg, font, xm03 - xm02 - 2 * Margin, lineHeight - Margin, taBottom | taLeft | taBorder);
}

void cLCARSNGDisplayMenu::DrawStatusElbows(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  osd->DrawRectangle(xs00, ys00, xs01 - 1, ys01 - 1, frameColorBr);
  osd->DrawText     (xs00 + Margin, ys00 + Margin, tr("TIMERS"), frameColorFg, frameColorBg, font, xs01 - xs00 - 2 * Margin, lineHeight - Margin, taBottom | taLeft | taBorder);
  DrawRectangleOutline(osd, xs02, ys00, xs03 - 1, ys01 - 1, frameColorBr, frameColorBg, 11);
  osd->DrawEllipse  (xs03, ys00, xs05 - 1, ys01 - 1, frameColorBr, 1);
  osd->DrawEllipse  (xs03, ys00 + Margin, xs05 - 1 - Margin, ys01 - 1 - Margin, frameColorBg, 1);
  DrawRectangleOutline(osd, xs04, ys01 - Margin, xs05 - 1, ys03 - 1, frameColorBr, frameColorBg, 13);
  osd->DrawEllipse  (xs03, ys01, xs04 - 1, ys02 - 1, frameColorBr, -1);
  osd->DrawEllipse  (xs03, ys01 - Margin, xs04 - 1 + Margin, ys02 - 1, frameColorBg, -1);
  DrawRectangleOutline(osd, xs04, ys04, xs05 - 1, ys05 - 1, frameColorBr, frameColorBg, 15);
  osd->DrawRectangle(xs10, ys00, xs11 - 1, ys01 - 1, frameColorBr);
  osd->DrawText     (xs10 + Margin, ys00 + Margin, tr("DEVICES"), frameColorFg, frameColorBg, font, xs11 - xs10 - 2 * Margin, lineHeight - Margin, taBottom | taRight | taBorder);
  DrawRectangleOutline(osd, xs08, ys00, xs09 - 1, ys01 - 1, frameColorBr, frameColorBg, 14);
  osd->DrawEllipse  (xs06, ys00, xs08 - 1, ys01 - 1, frameColorBr, 2);
  osd->DrawEllipse  (xs06 + Margin, ys00 + Margin, xs08 - 1, ys01 - 1 - Margin, frameColorBg, 2);
  DrawRectangleOutline(osd, xs06, ys01 - Margin, xs07 - 1, ys03 - 1, frameColorBr, frameColorBg, 13);
  osd->DrawEllipse  (xs07, ys01, xs08 - 1, ys02 - 1, frameColorBr, -2);
  osd->DrawEllipse  (xs07 - Margin, ys01 - Margin, xs08 - 1, ys02 - 1, frameColorBg, -2);
  DrawRectangleOutline(osd, xs06, ys04, xs07 - 1, ys05 - 1, frameColorBr, frameColorBg, 15);
  DrawRectangleOutline(osd, xs12, ys00, xs13 - 1, ys01 - 1, frameColorBr, frameColorBg, 15);
}

void cLCARSNGDisplayMenu::DrawFrameDisplay(void)
{
  if (initial) {
     if (yb082)
        DrawRectangleOutline(osd, xa00, yb082, xa02 - 1, yb09 - 1, frameColorBr, frameColorBg, 15);
     }
  DrawDate();
  DrawDisk();
  DrawLoad();
  DrawCountRecordings();
  DrawCountTimers();
  if ((MenuCategory() == mcRecording) && viewmode != efullscreen)
     DrawNumRecordingsInPath();
}

void cLCARSNGDisplayMenu::DrawScrollbar(int Total, int Offset, int Shown, bool CanScrollUp, bool CanScrollDown)
{
  int x0, x1, tt, tb;
  tColor ClearColor;
  if (MenuCategory() == mcMain || MenuCategory() == mcSetup) {
     x0 = xm07;
     x1 = xm08;
     tt = ym03;
     tb = ym04;
     ClearColor = Theme.Color(clrMenuMainBracket);
     }
  else if (MenuCategory() == mcChannel && viewmode == esmalscreen) {
     x0 = xm07;
     x1 = xm08;
     tt = yt04 + lineHeight;
     tb = ym04;
     ClearColor = Theme.Color(clrMenuMainBracket);
     }
  else {
     x0 = xa02 + Gap;
     x1 = x0 + lineHeight / 2;
     ClearColor = Theme.Color(clrBackground);
     int d = TextFrame;
     if (viewmode != efullscreen) {
        tt = yb00;
        tb = yb10 - Gap;
        }
     else {
        tt = yc00;
        tb = yc11;
        if (CanScrollUp)
           osd->DrawBitmap(xa02 - bmArrowUp.Width() - d, tt + d, bmArrowUp, Theme.Color(clrMenuScrollbarArrow), frameColorBg);
        else
           osd->DrawRectangle(xa02 - bmArrowUp.Width() - d, tt + d, xa02 - d - 1, tt + d + bmArrowUp.Height() - 1, frameColorBg);
        if (CanScrollDown)
           osd->DrawBitmap(xa02 - bmArrowDown.Width() - d, tb - d - bmArrowDown.Height(), bmArrowDown, Theme.Color(clrMenuScrollbarArrow), frameColorBg);
        else
           osd->DrawRectangle(xa02 - bmArrowDown.Width() - d, tb - d - bmArrowDown.Height(), xa02 - d - 1, tb - d - 1, frameColorBg);
        }
     }
  if (Total > 0 && Total > Shown) {
     int sw = x1 - x0;
     int sh = max(int((tb - tt) * double(Shown) / Total + 0.5), sw);
     int st = min(int(tt + (tb - tt) * double(Offset) / Total + 0.5), tb - sh);
     int sb = min(st + sh, tb);
     osd->DrawRectangle(x0, tt, x1 - 1, tb - 1, Theme.Color(clrMenuScrollbarTotal));
     osd->DrawRectangle(x0, st, x1 - 1, sb - 1, Theme.Color(clrMenuScrollbarShown));
     }
  else if (MenuCategory() != mcMain && MenuCategory() != mcSetup && !(MenuCategory() == mcChannel && viewmode == esmalscreen))
     osd->DrawRectangle(x0, tt, x1 - 1, tb - 1, ClearColor);
}

void cLCARSNGDisplayMenu::DrawTimer(const cTimer *Timer, int y, bool MultiRec)
{
  // The timer data:
  bool Alert = !Timer->Recording() && Timer->Pending();
  tColor ColorFg = Alert ? Theme.Color(clrAlertFg) : Theme.Color(clrTimerFg);
  tColor ColorBg = Alert ? Theme.Color(clrAlertBg) : Theme.Color(clrTimerBg);
  osd->DrawRectangle(xs00, y, xs03 - 1, y + lineHeight - 1, ColorBg);
  cString Date;
  if (Timer->Recording())
#if APIVERSNUM > 20502
     Date = cString::sprintf("-%s", *TimeString(Timer->StopTimeEvent()));
#else
     Date = cString::sprintf("-%s", *TimeString(Timer->StopTime()));
#endif
  else {
     time_t Now = time(NULL);
#if APIVERSNUM > 20502
     time_t StartTime = Timer->StartTimeEvent();
#else
     time_t StartTime = Timer->StartTime();
#endif
     cString Today = WeekDayName(Now);
     cString Time = TimeString(StartTime);
     cString Day = WeekDayName(StartTime);
     if (StartTime > Now + 6 * SECSINDAY)
        Date = DayDateTime(StartTime);
     else if (strcmp(Day, Today) != 0)
        Date = cString::sprintf("%s %s", *Day, *Time);
     else
        Date = Time;
     }
  if (Timer->Flags() & tfVps)
     Date = cString::sprintf("VPS %s", *Date);
#ifdef SWITCHONLYPATCH
  if (Timer->Flags() & tfSwitchOnly)
     Date = cString::sprintf("UST %s", *Date);
#endif
  const cChannel *Channel = Timer->Channel();
  const cEvent *Event = Timer->Event();
  int d = max(TextFrame / 2, 1);
  if (Channel) {
     osd->DrawText(xs00 + d, y, Channel->Name(), ColorFg, ColorBg, tinyFont, xs03 - xs00 - d);
     osd->DrawText(xs03 - tinyFont->Width(Date) - d, y, Date, ColorFg, ColorBg, tinyFont);
     }
  if (Event)
     osd->DrawText(xs00 + d, y + lineHeight - tinyFont->Height(), Event->Title(), ColorFg, ColorBg, tinyFont, xs03 - xs00 - 2 * d);
#if APIVERSNUM > 20300
  // The remote timer indicator:
  if (Timer->Remote())
     osd->DrawRectangle(xs00 - (lineHeight - Gap) / 2, y, xs00 - Gap - 1, y + lineHeight - 1, Timer->Recording() ? Theme.Color(clrMenuTimerRecording) : ColorBg);
#endif
  // The timer recording indicator:
#if APIVERSNUM > 20300
  else if (Timer->Recording())
#else
  if (Timer->Recording())
#endif
     osd->DrawRectangle(xs03 + Gap, y - (MultiRec ? Gap : 0), xs04 - Gap / 2 - 1, y + lineHeight - 1, Theme.Color(clrMenuTimerRecording));
}

void cLCARSNGDisplayMenu::DrawTimers(void)
{
#if APIVERSNUM > 20300
  if (const cTimers *Timers = cTimers::GetTimersRead(timersStateKey)) {
#else
  if (Timers.Modified(lastTimersState)) {
#endif
     deviceRecording.Clear();
     const cFont *font = cFont::GetFont(fontOsd);
#if APIVERSNUM > 20300
     osd->DrawRectangle(xs00 - (lineHeight - Gap) / 2, ys04, xs04 - 1, ys05 - 1, Theme.Color(clrBackground));
#else
     osd->DrawRectangle(xs00, ys04, xs04 - 1, ys05 - 1, Theme.Color(clrBackground));
#endif
     osd->DrawRectangle(xs07, ys04, xs13 - 1, ys05 - 1, Theme.Color(clrBackground));
#if APIVERSNUM > 20300
     cSortedTimers SortedTimers(Timers);
#else
     cSortedTimers SortedTimers;
#endif
     cVector<int> FreeDeviceSlots;
     int NumDevices = 0;
     int y = ys04;
     // Timers and recording devices:
     while (1) {
           int NumTimers = 0;
           const cDevice *Device = NULL;
           for (int i = 0; i < SortedTimers.Size(); i++) {
               if (y + lineHeight > ys05)
                  break;
               if (const cTimer *Timer = SortedTimers[i]) {
                  if (Timer->Recording()) {
#if APIVERSNUM > 20300
                     if (Timer->Remote()) {
                        if (!Device && Timer->HasFlags(tfActive)) {
                           DrawTimer(Timer, y, false);
                           FreeDeviceSlots.Append(y);
                           y += lineHeight + Gap;
                           }
                        else
                           continue;
                        }
                     else if (cRecordControl *RecordControl = cRecordControls::GetRecordControl(Timer)) {
#else
                     if (cRecordControl *RecordControl = cRecordControls::GetRecordControl(Timer)) {
#endif
                        if (!Device || Device == RecordControl->Device()) {
                           DrawTimer(Timer, y, NumTimers > 0);
                           NumTimers++;
                           if (!Device) {
                              Device = RecordControl->Device();
                              deviceOffset[Device->DeviceNumber()] = y;
                              deviceRecording[Device->DeviceNumber()] = true;
                              NumDevices++;
                              }
                           else
                              FreeDeviceSlots.Append(y);
                           y += lineHeight + Gap;
                           }
                        else
                           continue;
                        }
                     SortedTimers[i] = NULL;
                     }
                  else if (!Device && Timer->HasFlags(tfActive)) {
                     DrawTimer(Timer, y, false);
                     FreeDeviceSlots.Append(y);
                     y += lineHeight + Gap;
                     SortedTimers[i] = NULL;
                     }
                  }
               }
           if (!Device)
              break;
           }
     // Devices currently not recording:
     int Slot = 0;
     for (int i = 0; i < cDevice::NumDevices(); i++) {
         if (const cDevice *Device = cDevice::GetDevice(i)) {
            if (Device->NumProvidedSystems()) {
               if (!deviceRecording[Device->DeviceNumber()]) {
                  if (Slot < FreeDeviceSlots.Size()) {
                     y = FreeDeviceSlots[Slot];
                     Slot++;
                     }
                  if (y + lineHeight > ys05)
                     break;
                  deviceOffset[Device->DeviceNumber()] = y;
                  y += lineHeight + Gap;
                  NumDevices++;
                  }
               }
            }
         }
     // Total number of active timers:
     int NumTimers = 0;
#if APIVERSNUM > 20300
     for (const cTimer *Timer = Timers->First(); Timer; Timer = Timers->Next(Timer)) {
#else
     for (cTimer *Timer = Timers.First(); Timer; Timer = Timers.Next(Timer)) {
#endif
         if (Timer->HasFlags(tfActive))
            NumTimers++;
         }
     osd->DrawText(xs02 + Margin, ys00 + Margin, itoa(NumTimers), frameColorFg, frameColorBg, font, xs03 - xs02 - 2 * Margin, ys01 - ys00 - 2 * Margin, taBottom | taLeft | taBorder);
     osd->DrawText(xs08 + Margin, ys00 + Margin, itoa(NumDevices), frameColorFg, frameColorBg, font, xs09 - xs08 - 2 * Margin, ys01 - ys00 - 2 * Margin, taBottom | taRight | taBorder);
     lastSignalDisplay = 0;
     initial = true; // forces redrawing of devices
#if APIVERSNUM > 20300
     timersStateKey.Remove();
#endif
     }
}

void cLCARSNGDisplayMenu::DrawDevice(const cDevice *Device)
{
  int dn = Device->DeviceNumber();
  int y = deviceOffset[dn];
  if (y + lineHeight <= ys05) {
     if (DrawDeviceData(osd, Device, xs08, y, xs11, y + lineHeight, xs, tinyFont, lastDeviceType[dn], lastCamSlot[dn], initial)) {
        // Make sure signal meters are redrawn:
        lastSignalStrength[dn] = -1;
        lastSignalQuality[dn] = -1;
        lastSignalDisplay = 0;
        }
     // The device recording indicator:
     if (deviceRecording[dn])
        osd->DrawRectangle(xs07 + Gap / 2, y, xs08 - Gap - 1, y + lineHeight - 1, Theme.Color(clrMenuDeviceRecording));
     }
}

void cLCARSNGDisplayMenu::DrawDevices(void)
{
  for (int i = 0; i < cDevice::NumDevices(); i++) {
      if (const cDevice *Device = cDevice::GetDevice(i)) {
         if (Device->NumProvidedSystems())
            DrawDevice(Device);
         }
      }
}

void cLCARSNGDisplayMenu::DrawLiveIndicator(void)
{
  cDevice *Device = cDevice::PrimaryDevice();
  int y = -1;
  bool Transferring = Device->Transferring();
  if (!Device->Replaying() || Transferring)
     y = deviceOffset[cDevice::ActualDevice()->DeviceNumber()];
  if (initial || y != lastLiveIndicatorY || Transferring != lastLiveIndicatorTransferring) {
     if (lastLiveIndicatorY >= 0)
        osd->DrawRectangle(xs12, lastLiveIndicatorY, xs13 - 1, lastLiveIndicatorY + lineHeight - 1, Theme.Color(clrBackground));
     if (y > 0) {
        osd->DrawRectangle(xs12, y, xs12 + lineHeight / 2 - 1, y + lineHeight - 1, channelFrameColorBr);
        osd->DrawEllipse  (xs12 + lineHeight / 2, y, xs13 - 1, y + lineHeight - 1, channelFrameColorBr, 5);
        if (Transferring) {
           int w = bmTransferMode.Width();
           int h = bmTransferMode.Height();
           int b = w * w + h * h; // the diagonal of the bitmap (squared)
           int c = lineHeight * lineHeight; // the diameter of the circle (squared)
           const cBitmap *bm = &bmTransferMode;
           if (b > c) {
              // the bitmap doesn't fit, so scale it down:
              double f = sqrt(double(c) / (2 * b));
              bm = bmTransferMode.Scaled(f, f);
              }
           osd->DrawBitmap((xs12 + xs13 - bm->Width()) / 2, y + (lineHeight - bm->Height()) / 2, *bm, Theme.Color(clrChannelFrameFg), channelFrameColorBr);
           if (bm != &bmTransferMode)
              delete bm;
           }
        }
     lastLiveIndicatorY = y;
     lastLiveIndicatorTransferring = Transferring;
     }
}

void cLCARSNGDisplayMenu::DrawSignals(void)
{
  time_t Now = time(NULL);
  if (initial || Now - lastSignalDisplay >= SIGNALDISPLAYDELTA) {
     for (int i = 0; i < cDevice::NumDevices(); i++) {
         if (const cDevice *Device = cDevice::GetDevice(i)) {
            if (Device->NumProvidedSystems()) {
               if (int y = deviceOffset[i])
                  DrawDeviceSignal(osd, Device, xs + lineHeight / 2, y, xs11, y + lineHeight, lastSignalStrength[i], lastSignalQuality[i], initial);
               }
            }
         }
     lastSignalDisplay = Now;
     }
}

void cLCARSNGDisplayMenu::DrawLive(const cChannel *Channel)
{
  if (lastMode != cmLive) {
     initial = true;
     lastMode = cmLive;
     }
  int w = tallFont->Width(tr("LIVE")) + 2 * Gap;
  tColor ColorBg = (viewmode == escaledvideo) ? Theme.Color(clrBackground) : textColorBg;
  if (initial) {
     DrawMainFrameUpper(channelFrameColorBr, Theme.Color(clrChannelFrameBg));
     if (zoom && (viewmode == escaledvideo))
        osd->DrawText(xa01, yc04  + Margin, tr("LIVE"), Theme.Color(clrChannelFrameFg), Theme.Color(clrChannelFrameBg), cFont::GetFont(fontOsd), xa05 - xa01 - Margin, lineHeight - Margin, taRight | taBorder);
     else
        osd->DrawText(xd08 - w, yd00, tr("LIVE"), channelFrameColorBr, clrTransparent, tallFont, w, tallFont->Height(), taRight | taBorder);
     }
  if (!Channel)
     return;
  if (initial || Channel != lastChannel || strcmp(Channel->Name(), lastChannelName)) {
     int x1 = (viewmode == escaledvideo) ? xd07 : xd07 - w - lineHeight;
     int y1 = zoom ? yt00 : yt04;
     int y2 = zoom ? yt07 : yt09;
     osd->DrawText(xa00 + Margin, y1 + Margin, itoa(Channel->Number()), Theme.Color(clrChannelFrameFg), Theme.Color(clrChannelFrameBg), tallFont, xa02 - xa00 - 2 * Margin, 2 * lineHeight, taTop | taRight | taBorder);
     w = tallFont->Width(Channel->Name());
     osd->DrawRectangle(xa03, y1  + Margin, x1, y1  + Margin + tallFont->Height(), Theme.Color(clrBackground));
     osd->DrawText(xa03, y1  + Margin, Channel->Name(), Theme.Color(clrChannelName), ColorBg, tallFont, min(w, x1 - xa03), tallFont->Height(), taTop | taLeft);
//     int x = xa00 + (yc03 - yc02); // compensate for the arc
     osd->DrawText(xa00 + 3 * Margin, y2 + Margin, cSource::ToString(Channel->Source()), Theme.Color(clrChannelFrameFg), Theme.Color(clrChannelFrameBg), cFont::GetFont(fontOsd), xa02 - xa00 - 4 * Margin, lineHeight, taTop | taRight | taBorder);
     lastChannel = Channel;
     lastChannelName = Channel->Name();
     DrawSeen(0, 0);
     }
  // The current programm:
#if APIVERSNUM > 20300
  LOCK_SCHEDULES_READ;
  if (const cSchedule *Schedule = Schedules->GetSchedule(Channel)) {
     const cEvent *Event = Schedule->GetPresentEvent();
     if (initial || Event != lastEvent) {
        DrawInfo(Event, true);
        lastEvent = Event;
        lastSeen = -1;
        }
     int Current = 0;
     int Total = 0;
     if (Event) {
        time_t t = time(NULL);
        if (t > Event->StartTime())
           Current = t - Event->StartTime();
        Total = Event->Duration();
        }
     DrawSeen(Current, Total);
#else
  cSchedulesLock SchedulesLock;
  if (const cSchedules *Schedules = cSchedules::Schedules(SchedulesLock)) {
     if (const cSchedule *Schedule = Schedules->GetSchedule(Channel)) {
        const cEvent *Event = Schedule->GetPresentEvent();
        if (initial || Event != lastEvent) {
           DrawInfo(Event, true);
           lastEvent = Event;
           lastSeen = -1;
           }
        int Current = 0;
        int Total = 0;
        if (Event) {
           time_t t = time(NULL);
           if (t > Event->StartTime())
              Current = t - Event->StartTime();
           Total = Event->Duration();
           }
        DrawSeen(Current, Total);
        }
#endif
     }
}

void cLCARSNGDisplayMenu::DrawPlay(cControl *Control)
{
  if (lastMode != cmPlay) {
     initial = true;
     lastMode = cmPlay;
     }
  int w = tallFont->Width(tr("PLAY")) + 2 * Gap;
  tColor ColorBg = (viewmode == escaledvideo) ? Theme.Color(clrBackground) : textColorBg;
  if (initial) {
     DrawMainFrameUpper(replayFrameColorBr, Theme.Color(clrReplayFrameBg));
     if (zoom && (viewmode == escaledvideo))
        osd->DrawText(xa01, yc04  + Margin, tr("PLAY"), Theme.Color(clrReplayFrameFg), Theme.Color(clrReplayFrameBg), cFont::GetFont(fontOsd), xa05 - xa01 - Margin, lineHeight - Margin, taRight | taBorder);
     else
        osd->DrawText(xd08 - w, yd00, tr("PLAY"), replayFrameColorBr, ColorBg, tallFont, w, tallFont->Height(), taRight | taBorder);
     }
  // The current progress:
  int Current = 0;
  int Total = 0;
  if (Control->GetIndex(Current, Total))
     DrawSeen(Current, Total);
  // The current recording:
  int x1 = (viewmode == escaledvideo) ? xd07 : xd07 - w - lineHeight;
  int y1 = zoom ? yt00 : yt04;
  int y2 = zoom ? yt04 : yt07;
  if (const cRecording *Recording = Control->GetRecording()) {
     if (initial || Recording != lastRecording) {
        const cFont *font = cFont::GetFont(fontOsd);
        osd->DrawRectangle(xa03, y1  + Margin, x1, y1  + Margin + tallFont->Height(), Theme.Color(clrBackground));
        if (const cRecordingInfo *Info = Recording->Info()) {
           w = tallFont->Width(Info->ChannelName());
           osd->DrawText(xa03, y1 + Margin, Info->ChannelName(), Theme.Color(clrChannelName), ColorBg, tallFont, min(w, x1 - xa03), tallFont->Height(), taTop | taLeft);
           DrawInfo(Info->GetEvent(), false);
           }
        else {
           w = tallFont->Width(Recording->Name());
           osd->DrawText(xa03, y1 + Margin, Recording->Name(), Theme.Color(clrEventTitle), ColorBg, tallFont, min(w, x1 - xa03), tallFont->Height(), taTop | taLeft);
           }
        osd->DrawText(xa00 + Margin, y2 + Margin, ShortDateString(Recording->Start()), Theme.Color(clrReplayFrameFg), Theme.Color(clrReplayFrameBg), font, xa02 - xa00  - 2 * Margin, 0, taTop | taRight | taBorder);
        osd->DrawText(xa00 + Margin, y2 + lineHeight + Margin, TimeString(Recording->Start()), Theme.Color(clrReplayFrameFg), Theme.Color(clrReplayFrameBg), font, xa02 - xa00  - 2 * Margin, 0, taBottom | taRight | taBorder);
        lastRecording = Recording;
        }
     }
  else {
     cString Header = Control->GetHeader();
     if (initial || !*lastHeader || strcmp(Header, lastHeader)) {
        w = tallFont->Width(Header);
        osd->DrawRectangle(xa03, y1  + Margin, x1, y1  + Margin + tallFont->Height(), Theme.Color(clrBackground));
        osd->DrawText(xa03, y1+ Margin, Header, Theme.Color(clrMenuText), ColorBg, tallFont, min(w, x1 - xa03), tallFont->Height(), taTop | taLeft);
        lastHeader = Header;
        }
     }
}

void cLCARSNGDisplayMenu::DrawInfo(const cEvent *Event, bool WithTime)
{
  if (Event) {
     const cFont *font = cFont::GetFont(fontOsd);
     const cFont *fontsml = cFont::GetFont(fontSml);
     int y = (zoom ? yt04 : yt07) + Margin;
     int x = (viewmode == escaledvideo) ? xd07 : xd00 - lineHeight;
     tColor ColorBg = (viewmode == escaledvideo) ? Theme.Color(clrBackground) : textColorBg;
     int w = font->Width(Event->Title());
     osd->DrawRectangle(xa03, y, x, y + lineHeight, Theme.Color(clrBackground));
     osd->DrawText(xa03, y, Event->Title(), Theme.Color(clrEventTitle), ColorBg, font, min (w, x - xa03), lineHeight, taBottom | taLeft);
     if (WithTime)
        osd->DrawText(xa00 + Margin, y, Event->GetTimeString(), Theme.Color(clrChannelFrameFg), Theme.Color(clrChannelFrameBg), font, xa02 - xa00 - 2 * Margin, lineHeight, taTop | taRight | taBorder);
     y += lineHeight;
     w = fontsml->Width(Event->ShortText());
     osd->DrawRectangle(xa03, y + ((lineHeight - fontsml->Height()) / 2), x, y + lineHeight, Theme.Color(clrBackground));
     osd->DrawText(xa03, y + ((lineHeight - fontsml->Height()) / 2), Event->ShortText(), Theme.Color(clrEventShortText), ColorBg, fontsml, min (w, x - xa03), fontsml->Height(), taBottom | taLeft);
     if (WithTime)
        osd->DrawText(xa00 + Margin, y, cString::sprintf("-%s", *Event->GetEndTimeString()), Theme.Color(clrChannelFrameFg), Theme.Color(clrChannelFrameBg), font, xa02 - xa00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);
     }
}

void cLCARSNGDisplayMenu::DrawSeen(int Current, int Total)
{
// progress bar

  int x = (!(viewmode == escaledvideo) && (MenuCategory() == mcMain)) ? xm08 : xm03;
  int Seen = (Total > 0) ? min(x - xm02, int((x - xm02) * double(Current) / Total)) : 0;
  if (initial || Seen != lastSeen) {
     int y0 = yc04 - ShowSeenExtent;
     int y1 = yc04 + lineHeight / 2 - Gap / 2;
     osd->DrawRectangle(xm02, y0, xm02 + Seen - 1, y1 - 1, Theme.Color(clrSeen));
     osd->DrawRectangle(xm02 + Seen, y0, x - 1, y1 - 1, Theme.Color(clrBackground));
     lastSeen = Seen;
     }
}

void cLCARSNGDisplayMenu::DrawTextScrollbar(void)
{
  if (textScroller.CanScroll())
     DrawScrollbar(textScroller.Total(), textScroller.Offset(), textScroller.Shown(), textScroller.CanScrollUp(), textScroller.CanScrollDown());
}

void cLCARSNGDisplayMenu::Scroll(bool Up, bool Page)
{
  cSkinDisplayMenu::Scroll(Up, Page);
  DrawTextScrollbar();
}

int cLCARSNGDisplayMenu::MaxItems(void)
{
  switch (MenuCategory()) {
     case mcMain:
        return (ym04 - ym03) / lineHeight;
        break;
     case mcChannel:
        if (viewmode == esmalscreen) {
           return (ym04 - yt04 - lineHeight) / lineHeight;
           break;
           }
     default:
        if (viewmode == efullscreen)
           return (yb13 - yt02) / lineHeight;
        else if (MenuCategory() == mcSetup)
           return (ym04 - ym03) / lineHeight;
        else
           return (ym07 - ym00) / lineHeight;
     }
}

void cLCARSNGDisplayMenu::Clear(void)
{
  DELETENULL(drawDescription);
  animatedInfo.Event = NULL;
  animatedInfo.Timer = NULL;
  animatedInfo.Recording = NULL;
  animatedInfo.Channel = NULL;
  textScroller.Reset();
  osd->DrawRectangle(xi00, yi00, xi03 - 1, yi01 - 1, Theme.Color(clrBackground));
}

void cLCARSNGDisplayMenu::SetTitle(const char *Title)
{
  const cFont *font = cFont::GetFont(fontOsd);
  currentTitle = NULL;
  switch (MenuCategory()) {
     case mcMain:
        break;
     case mcChannel:
        if (viewmode == esmalscreen)
           break;
     case mcRecording:
        currentTitle = Title;
     default:
        if (viewmode != efullscreen) {
           if (MenuCategory() == mcSetup)
              return;
           DrawRectangleOutline(osd, xs00, ys00, xs11, ys01 - 1, frameColorBr, frameColorBg, 15);
           osd->DrawText(xs00 + Margin, ys00 + Margin, Title, frameColorFg, frameColorBg, font, xs11 - xs00 - 1 - 2 * Margin, lineHeight - Margin, taBottom | taRight | taBorder);
           DrawRectangleOutline(osd, xs12, ys00, xs13 - 1, ys01 - 1, frameColorBr, frameColorBg, 15);
           }
        else {
           int w = min(font->Width(Title), xa07 - xa06 - Gap);
           DrawRectangleOutline(osd, xa06, yt00, xa07 - 1, yt01 - 1, frameColorBr, frameColorBg, 15);
           osd->DrawText(xa07 - w - Gap, yt00 + Margin, Title, frameColorFg, frameColorBg, font, w + Gap - Margin, lineHeight - Margin, taRight);
           }
     }
}

void cLCARSNGDisplayMenu::SetButtons(const char *Red, const char *Green, const char *Yellow, const char *Blue)
{
  const char *lutText[] = { Red, Green, Yellow, Blue };
  tColor lutFg[] = { clrButtonRedFg, clrButtonGreenFg, clrButtonYellowFg, clrButtonBlueFg };
  tColor lutBg[] = { clrButtonRedBg, clrButtonGreenBg, clrButtonYellowBg, clrButtonBlueBg };
  const cFont *font = cFont::GetFont(fontSml);
  if (viewmode != efullscreen || (MenuCategory() == mcMain) || (MenuCategory() == mcChannel && viewmode == esmalscreen)) {
     DrawMainButton(lutText[Setup.ColorKey0], xd00, xd01, xd02, xd03, yd02, yd03, Theme.Color(lutFg[Setup.ColorKey0]), Theme.Color(lutBg[Setup.ColorKey0]), font);
     DrawMainButton(lutText[Setup.ColorKey1], xd04, xd05, xd06, xd07, yd02, yd03, Theme.Color(lutFg[Setup.ColorKey1]), Theme.Color(lutBg[Setup.ColorKey1]), font);
     DrawMainButton(lutText[Setup.ColorKey2], xd00, xd01, xd02, xd03, yd04, yd05, Theme.Color(lutFg[Setup.ColorKey2]), Theme.Color(lutBg[Setup.ColorKey2]), font);
     DrawMainButton(lutText[Setup.ColorKey3], xd04, xd05, xd06, xd07, yd04, yd05, Theme.Color(lutFg[Setup.ColorKey3]), Theme.Color(lutBg[Setup.ColorKey3]), font);
     }
  else {
     int h = yb15 - yb14;
     osd->DrawText(xb02, yb14, lutText[Setup.ColorKey0], Theme.Color(lutFg[Setup.ColorKey0]), Theme.Color(lutBg[Setup.ColorKey0]), font, xb03 - xb02, h, taLeft | taBorder);
     osd->DrawText(xb06, yb14, lutText[Setup.ColorKey1], Theme.Color(lutFg[Setup.ColorKey1]), Theme.Color(lutBg[Setup.ColorKey1]), font, xb07 - xb06, h, taLeft | taBorder);
     osd->DrawText(xb10, yb14, lutText[Setup.ColorKey2], Theme.Color(lutFg[Setup.ColorKey2]), Theme.Color(lutBg[Setup.ColorKey2]), font, xb11 - xb10, h, taLeft | taBorder);
     osd->DrawText(xb14, yb14, lutText[Setup.ColorKey3], Theme.Color(lutFg[Setup.ColorKey3]), Theme.Color(lutBg[Setup.ColorKey3]), font, xb15 - xb14, h, taLeft | taBorder);
     }
}

void cLCARSNGDisplayMenu::SetMessage(eMessageType Type, const char *Text)
{
  if (Text) {
     DELETENULL(drawDescription);
     DELETENULL(volumeBox);
     message = true;
     tColor ColorFg = Theme.Color(clrMessageStatusFg + 2 * Type);
     tColor ColorBg = Theme.Color(clrMessageStatusBg + 2 * Type);
     int x0, x1, y0, y1, lx, ly;
     x0 = xb00;
     x1 = xb15 - 1;
     y0 = yb14 - 2 * Margin;
     y1 = yb15 - 1;
     lx = x1 - x0 - 2 * Margin;
     ly = y1 - y0 - 2 * Margin;
     osd->SaveRegion(x0, y0, x1, y1);
     DrawRectangleOutline(osd, x0, y0, x1, y1, ColorFg, ColorBg, 15);
     osd->DrawText(x0 + Margin, y0 + Margin, Text, ColorFg, ColorBg, cFont::GetFont(fontSml), lx, ly, taCenter);
     }
  else {
     osd->RestoreRegion();
     message = false;
     }
}
bool cLCARSNGDisplayMenu::SetItemEvent(const cEvent *Event, int Index, bool Current, bool Selectable, const cChannel *Channel, bool WithDate, eTimerMatch TimerMatch, bool TimerActive)
{
  if (animatedInfo.Event == Event && !Current) {
     DELETENULL(drawDescription);
     animatedInfo.Event = NULL;
     }
  if (Current) {
     animatedInfo.Event = Event;
     animatedInfo.Index = Index;
     animatedInfo.Current = Current;
     animatedInfo.Selectable = Selectable;
     animatedInfo.Channel = Channel;
     animatedInfo.WithDate = WithDate;
     animatedInfo.TimerMatch = TimerMatch;
     animatedInfo.TimerActive = TimerActive;
     }
  return false;
}

bool cLCARSNGDisplayMenu::SetItemTimer(const cTimer *Timer, int Index, bool Current, bool Selectable)
{
  if (animatedInfo.Timer == Timer && !Current) {
     DELETENULL(drawDescription);
     animatedInfo.Timer = NULL;
     }
  if (Current) {
     animatedInfo.Timer = Timer;
     animatedInfo.Index = Index;
     animatedInfo.Current = Current;
     animatedInfo.Selectable = Selectable;
     }
  return false;
}

bool cLCARSNGDisplayMenu::SetItemRecording(const cRecording *Recording, int Index, bool Current, bool Selectable, int Level, int Total, int New)
{
  if (animatedInfo.Recording == Recording && !Current) {
     DELETENULL(drawDescription);
     animatedInfo.Recording = NULL;
     }
  if (Current && !Total) {
     animatedInfo.Recording = Recording;
     animatedInfo.Index = Index;
     animatedInfo.Current = Current;
     animatedInfo.Selectable = Selectable;
     animatedInfo.Level = Level;
     animatedInfo.Total = Total;
     animatedInfo.New = New;
     }
  return false;
}

void cLCARSNGDisplayMenu::SetItem(const char *Text, int Index, bool Current, bool Selectable)
{
  int y = yi00 + Index * lineHeight;
  tColor ColorFg, ColorBg;
  if (Current) {
     if (TwoColors) {
        ColorFg = Theme.Color(clrBackground);
        ColorBg = Theme.Color(clrMenuFrameBg);
        }
     else {
        ColorFg = Theme.Color(clrMenuItemCurrentFg);
        ColorBg = Theme.Color(clrMenuItemCurrentBg);
        }
     osd->DrawRectangle(xi00, y, xi01 - 1, y + lineHeight - 1, ColorBg);
     osd->DrawRectangle(xi02, y, xi02 + lineHeight / 2 - 1, y + lineHeight - 1, ColorBg);
     osd->DrawEllipse  (xi02 + lineHeight / 2, y, xi03 - 1, y + lineHeight - 1, ColorBg, 5);
     currentIndex = Index;
     }
  else {
     ColorFg = Theme.Color(Selectable ? clrMenuItemSelectable : clrMenuItemNonSelectable);
     ColorBg = (viewmode == escaledvideo) ? Theme.Color(clrBackground) : textColorBg;
     osd->DrawRectangle(xi00, y, xi03 - 1, y + lineHeight - 1, Theme.Color(clrBackground));
     }
  const cFont *font = cFont::GetFont(fontOsd);
  for (int i = 0; i < MaxTabs; i++) {
      const char *s = GetTabbedText(Text, i);
      if (s) {
         int xt = xi00 + 2 * TextSpacing + Tab(i);
         int tabWidth = 0; 
         if (Tab(i + 1))  
            tabWidth = Tab(i + 1) - Tab(i);
         else 
            tabWidth = xi01 - xt;
         if (!DrawProgressBar(xt, y, tabWidth - TextSpacing, s, ColorFg, ColorBg)) {
            osd->DrawText(xt, y, s, ColorFg, ColorBg, font, xi01 - xt);
            }
         }
      if (!Tab(i + 1))
         break;
      }
  SetEditableWidth(xi02 - xi00 - TextSpacing - Tab(1));
}

bool cLCARSNGDisplayMenu::DrawProgressBar(int x, int y, int width, const char *text, tColor ColorFg, tColor ColorBg) {
  if (strlen(text) <= 5 || text[0] != '[' || text[strlen(text) - 1] != ']')
     return false;
  const char *p = text + 1;
  int total = 0;
  int now = 0;
  for (; *p != ']'; ++p) {
     if (*p == ' ' || *p == '|') {
        ++total;
        if (*p == '|')
           ++now;
        }
     else {
        return false;
        }
     }
  int y0 = y + lineHeight / 4;
  int y1 = y0 + lineHeight / 2;
  osd->DrawRectangle(x, y, x + width, y + lineHeight - 1, ColorBg);
  osd->DrawRectangle(x, y0, x + width, y1, ColorFg);
  osd->DrawRectangle(x + 2, y0 + 2, x + width - 2, y1 - 2, ColorBg);
  double progress = (double)now / (double)total;
  osd->DrawRectangle(x + 3, y0 + 3, x + (width - 3) * progress, y1 - 3, ColorFg);
  return true;
}

void cLCARSNGDisplayMenu::SetScrollbar(int Total, int Offset)
{
  DrawScrollbar(Total, Offset, MaxItems(), Offset > 0, Offset + MaxItems() < Total);
}

void cLCARSNGDisplayMenu::SetEvent(const cEvent *Event)
{
  if (!Event)
     return;
  const cFont *font = cFont::GetFont(fontOsd);
  int xl = xi00;
  int y = yi00;
  cTextScroller ts;
  char t[32];
  tColor ColorBg = (viewmode == escaledvideo) ? Theme.Color(clrBackground) : textColorBg;
  snprintf(t, sizeof(t), "%s  %s - %s", *Event->GetDateString(), *Event->GetTimeString(), *Event->GetEndTimeString());
  int w = font->Width(t) + Gap;
  ts.Set(osd, xl, y, min(w, xi01 - xl), yi01 - y, t, font, Theme.Color(clrEventTime), ColorBg);
  if (Event->Vps() && Event->Vps() != Event->StartTime()) {
     cString buffer = cString::sprintf(" VPS: %s ", *Event->GetVpsString());
     const cFont *font = cFont::GetFont(fontSml);
     w = font->Width(buffer);
     osd->DrawText(xi01 - w, y, buffer, Theme.Color(clrMenuFrameFg), frameColorBg, font, w);
     int yb = y + font->Height();
     osd->DrawRectangle(xi02, y, xi02 + lineHeight / 2 - 1, yb - 1, frameColorBg);
     osd->DrawEllipse  (xi02 + lineHeight / 2, y, xi03 - 1, yb - 1, frameColorBg, 5);
     }
  y += ts.Height();
  if (Event->ParentalRating()) {
     cString buffer = cString::sprintf(" %s ", *Event->GetParentalRatingString());
     const cFont *font = cFont::GetFont(fontSml);
     w = font->Width(buffer);
     osd->DrawText(xi01 - w, y, buffer, Theme.Color(clrMenuFrameFg), frameColorBg, font, w);
     int yb = y + font->Height();
     osd->DrawRectangle(xi02, y, xi02 + lineHeight / 2 - 1, yb - 1, frameColorBg);
     osd->DrawEllipse  (xi02 + lineHeight / 2, y, xi03 - 1, yb - 1, frameColorBg, 5);
     }
  y += font->Height();
  w = font->Width(Event->Title()) + Gap;
  ts.Set(osd, xl, y, min(w, xi01 - xl), yi01 - y, Event->Title(), font, Theme.Color(clrEventTitle), ColorBg);
  y += ts.Height();
  if (!isempty(Event->ShortText())) {
     const cFont *font = cFont::GetFont(fontSml);
     w = font->Width(Event->ShortText()) + Gap;
     ts.Set(osd, xl, y, min(w, xi01 - xl), yi01 - y, Event->ShortText(), font, Theme.Color(clrEventShortText), ColorBg);
     y += ts.Height();
     }
  y += font->Height();
  if (!isempty(Event->Description())) {
     int yt = y;
     int yb = yi01;
     textScroller.Set(osd, xl, yt, xi01 - xl, yb - yt, Event->Description(), font, Theme.Color(clrEventDescription), ColorBg);
     DrawTextScrollbar();
     }
}

void cLCARSNGDisplayMenu::SetRecording(const cRecording *Recording)
{
  if (!Recording)
     return;
  const cRecordingInfo *Info = Recording->Info();
  const cFont *font = cFont::GetFont(fontOsd);
  int xl = xi00;
  int y = yi00;
  cTextScroller ts;
  tColor ColorBg = (viewmode == escaledvideo) ? Theme.Color(clrBackground) : textColorBg;
  cString t = cString::sprintf("%s  %s  %s", *DateString(Recording->Start()), *TimeString(Recording->Start()), Info->ChannelName() ? Info->ChannelName() : "");
  ts.Set(osd, xl, y, xi01 - xl, yi01 - y, t, font, Theme.Color(clrEventTime), ColorBg);
  y += ts.Height();
  int xt = xi01;
  if (Info->GetEvent()->ParentalRating()) {
     cString buffer = cString::sprintf(" %s ", *Info->GetEvent()->GetParentalRatingString());
     const cFont *font = cFont::GetFont(fontSml);
     int w = font->Width(buffer);
     int yb = y + font->Height();
     DrawRectangleOutline(osd, xt - w - 2 * Margin, y - Margin, xi01 - 1, yb + Margin - 1, frameColorBr, frameColorBg, 15);
     osd->DrawText(xt - w - Margin, y, buffer, Theme.Color(clrMenuFrameFg), frameColorBg, font, w);
     xt -= w + xi02 - xi01 + 2 * Margin;
     }
#if (APIVERSNUM >= 20505)
  if (Info->Errors() >= (1 - Config.displayError0)) {
     cString buffer = cString::sprintf(" %d %s ", Info->Errors(), tr("errors"));
     const cFont *font = cFont::GetFont(fontSml);
     int w = font->Width(buffer);
     int yb = y + font->Height();
     DrawRectangleOutline(osd, xt - w - 2 * Margin, y - Margin, xt - 1, yb + Margin - 1, frameColorBr, frameColorBg, 15);
     osd->DrawText(xt - w - Margin, y, buffer, Theme.Color(clrMenuFrameFg), frameColorBg, font, w);
     xt -= w + xi02 - xi01 + 2 * Margin;
     }
#endif
  if (xt != xi01) {
     const cFont *font = cFont::GetFont(fontSml);
     int yb = y + font->Height();
     DrawRectangleOutline(osd, xi02, y - Margin, xi02 + lineHeight / 2 - 1, yb + Margin - 1, frameColorBr, frameColorBg, 11);
     osd->DrawEllipse  (xi02 + lineHeight / 2, y - Margin, xi03 - 1, yb + Margin - 1, frameColorBr, 5);
     osd->DrawEllipse  (xi02 + lineHeight / 2, y, xi03 - Margin - 1, yb - 1, frameColorBg, 5);
     }
  y += font->Height();
  const char *Title = Info->Title();
  if (isempty(Title))
     Title = Recording->Name();
  ts.Set(osd, xl, y, xi01 - xl, yi01 - y, Title, font, Theme.Color(clrEventTitle), ColorBg);
  y += ts.Height();
  if (!isempty(Info->ShortText())) {
     const cFont *font = cFont::GetFont(fontSml);
     ts.Set(osd, xl, y, xi01 - xl, yi01 - y, Info->ShortText(), font, Theme.Color(clrEventShortText), ColorBg);
     y += ts.Height();
     }
  y += font->Height();
  std::stringstream sstrInfo;
  if (!isempty(Info->Description())) {
     sstrInfo << Info->Description() << std::endl;
     }
  const char *aux = NULL;
  aux = Info->Aux();
  if (aux) {
     std::string strAux = aux;
     std::string auxEpgsearch = StripXmlTag(strAux, "epgsearch");
     if (!auxEpgsearch.empty()) {
        std::string searchTimer = StripXmlTag(auxEpgsearch, "searchtimer");
        if (!searchTimer.empty()) {
           sstrInfo << std::endl << tr("Search timer:") << " " << searchTimer;
           }
        }
     std::string str_tvscraper = StripXmlTag(strAux, "tvscraper");
     if (!str_tvscraper.empty()) {
        std::string causedby = StripXmlTag(str_tvscraper, "causedBy");
        std::string reason = StripXmlTag(str_tvscraper, "reason");
        if (!causedby.empty() && !reason.empty()) {
           sstrInfo << std::endl << "TVScraper: " << tr("caused by:") << " " << causedby << ", "
                                 << tr("reason:") << " " << reason;
           }
        }
     std::string str_vdradmin = StripXmlTag(strAux, "vdradmin-am");
     if (!str_vdradmin.empty()) {
        std::string pattern = StripXmlTag(str_vdradmin, "pattern");
        if (!pattern.empty()) {
           sstrInfo << std::endl << "VDRadmin-AM: " << tr("search pattern:") << " " << pattern;
           }
        }
     }
  if (!isempty(sstrInfo.str().c_str())) {
     int yt = y;
     int yb = yi01;
     textScroller.Set(osd, xl, yt, xi01 - xl, yb - yt, sstrInfo.str().c_str(), font, Theme.Color(clrEventDescription), ColorBg);
     DrawTextScrollbar();
     }
}

void cLCARSNGDisplayMenu::SetText(const char *Text, bool FixedFont)
{
  tColor ColorBg = (viewmode == escaledvideo) ? Theme.Color(clrBackground) : textColorBg;
  textScroller.Set(osd, xi00, yi00, GetTextAreaWidth(), yi01 - yi00, Text, GetTextAreaFont(FixedFont), Theme.Color(clrMenuText), ColorBg);
  DrawTextScrollbar();
}

int cLCARSNGDisplayMenu::GetTextAreaWidth(void) const
{
  return xi01 - xi00;
}

const cFont *cLCARSNGDisplayMenu::GetTextAreaFont(bool FixedFont) const
{
  const cFont *font = cFont::GetFont(FixedFont ? fontFix : fontOsd);
  //XXX -> make a way to let the text define which font to use
  return font;
}

void cLCARSNGDisplayMenu::DrawVolume(void)
{
   if (!message) {
      int volume = statusMonitor->GetVolume();
      if (volume != lastVolume) {
         if (!volumeBox)
            volumeBox = new cLCARSNGVolumeBox(osd, cRect(0, yb15 - lineHeight, xa09, lineHeight));
         volumeBox->SetVolume(volume, MAXVOLUME, volume ? false : true);
         lastVolumeTime = time(NULL);
	 lastVolume = volume;
         }
      else {
         if (volumeBox && (time(NULL) - lastVolumeTime > 2))
            DELETENULL(volumeBox);
      }
   }
}

void cLCARSNGDisplayMenu::Flush(void)
{
  int Width;
  int Height;
  double Aspect;
  cDevice *Device = cDevice::PrimaryDevice();
  cDevice::PrimaryDevice()->GetOsdSize(Width, Height, Aspect);
  int xrand = (Width - xa09) / 2;
  int yrand = (Height - yb15) / 2;
  cRect videoWindowRect( xs00 + xrand, yrand + Gap, xs11 - xs00, yc05 - yrand / 2);
  if (initial) {
     if (viewmode == escaledvideo)
        availableRect = cDevice::PrimaryDevice()->CanScaleVideo(videoWindowRect);
     else
        availableRect = cDevice::PrimaryDevice()->CanScaleVideo(cRect::Null);
     }
  DrawFrameDisplay();
  switch (MenuCategory()) {
     case mcMain:
     case mcSetup:
        if ((viewmode != efullscreen) || (MenuCategory() == mcMain)) {
           DrawTimers();
           DrawDevices();
           DrawLiveIndicator();
           DrawSignals();
           }
     default:
        if ((viewmode != efullscreen && viewmode != esmalscreen) || (MenuCategory() == mcMain)) {
#if APIVERSNUM > 20402
           cMutexLock ControlMutexLock;
#endif
           if (!Device->Replaying() || Device->Transferring()) {
#if APIVERSNUM > 20300
              LOCK_CHANNELS_READ;
              const cChannel *Channel = Channels->GetByNumber(cDevice::PrimaryDevice()->CurrentChannel());
#else
              const cChannel *Channel = Channels.GetByNumber(cDevice::PrimaryDevice()->CurrentChannel());
#endif
              DrawLive(Channel);
              }
#if APIVERSNUM > 20402
           else if (cControl *Control = cControl::Control(ControlMutexLock, true))
#else
           else if (cControl *Control = cControl::Control(true))
#endif
              DrawPlay(Control);
           }
     }
  DrawVolume();
#ifdef DRAWGRID
  DrawGrid();
#endif
  if (!(drawDescription && drawDescription->IsRunning()))
     osd->Flush();
  if (initial) {
     cDevice::PrimaryDevice()->ScaleVideo(availableRect);
     }
  if (!message && !drawDescription) {
     if ((Config.displInfoMenuEPG && (MenuCategory() == mcSchedule || MenuCategory() == mcScheduleNow || MenuCategory() == mcScheduleNext) && animatedInfo.Event != NULL)
         || (Config.displInfoMenuTimer && MenuCategory() == mcTimer && animatedInfo.Timer != NULL)
         || (Config.displInfoMenuRec && MenuCategory() == mcRecording && animatedInfo.Recording != NULL)) {
        animatedInfo.x0 = xs00;
        animatedInfo.x1 = xs11;
        animatedInfo.y0 = yi00;
        animatedInfo.y1 = yi00 + (int)((yi01 - yi00) / lineHeight) * lineHeight;
        animatedInfo.viewmode = viewmode;
        animatedInfo.textColorBg = textColorBg;
        animatedInfo.titleColorFg = Theme.Color(clrEventTitle);
        animatedInfo.shortTextColorFg = Theme.Color(clrEventShortText);
        animatedInfo.descriptionColorFg = Theme.Color(clrEventDescription);
        animatedInfo.frameColorBr = frameColorBr;
        drawDescription = new cDrawDescription(osd, animatedInfo);
        }
     }	             
  initial = false;
}

#ifdef DRAWGRID
void cLCARSNGDisplayMenu::DrawGrid(void)
{
  int left = xa00;
  int right = xa09 - 1;
  int top = yt00;
  int offset = lineHeight / 2;
  int bottom = yb15 - 1;
  tColor gridColor = Theme.Color(clrDeviceFg);
  cFont *TinyFont = cFont::CreateFont(Setup.FontOsd, 14);

  int xa[10] = { xa00, xa01, xa02, xa03, xa04, xa05, xa06, xa07, xa08, xa09 };
  int yt[11] = { yt00, yt01, yt02, yt03, yt04, yt05, yt06, yt07, yt08, yt09, yt10 };
  int yc[12] = { yc00, yc01, yc02, yc03, yc04, yc05, yc06, yc07, yc08, yc09, yc10, yc11 };
  int yb[18] = { yb00, yb01, yb02, yb03, yb04, yb05, yb06, yb07, yb08, yb081, yb082, yb09, yb10, yb11, yb12, yb13, yb14, yb15 };
  int xm[9] = { xm00, xm01, xm02, xm03, xm04, xm05, xm06, xm07, xm08 };
  int ym[8] = { ym00, ym01, ym02, ym03, ym04, ym05, ym06, ym07 };
  int xs[15] = { xs00, xs01, xs02, xs03, xs04, xs05, xs06, xs07, xs08, xs09, xs10, xs11, xs12, xs13, *xs };
  int ys[6] = { ys00, ys01, ys02, ys03, ys04, ys05 };
  int xi[4] = { xi00, xi01, xi02, xi03 };
  int yi[2] = { yi00, yi01 };
  int xb[16] = { xb00, xb01, xb02, xb03, xb04, xb05, xb06, xb07, xb08, xb09, xb10, xb11, xb12, xb13, xb14, xb15 };
  int xd[8] = { xd00, xd01, xd02, xd03, xd04, xd05, xd06, xd07, xd08 };
  int yd[6] = { yd00, yd01, yd02, yd03, yd04, yd05 };

  char strxa[10][6] = { "xa00", "xa01", "xa02", "xa03", "xa04", "xa05", "xa06", "xa07", "xa08", "xa09" };
  char strxm[9][6] = { "xm00", "xm01", "xm02", "xm03", "xm04", "xm05", "xm06", "xm07", "xm08" };
  char strxs[15][6] = { "xs00", "xs01", "xs02", "xs03", "xs04", "xs05", "xs06", "xs07", "xs08", "xs09", "xs10", "xs11", "xs12", "xs13", "xs" };
  char strxi[4][6] = { "xi00", "xi01", "xi02", "xi03" };
  char strxb[16][6] = { "xb00", "xb01", "xb02", "xb03", "xb04", "xb05", "xb06", "xb07", "xb08", "xb09", "xb10", "xb11", "xb12", "xb13", "xb14", "xb15" };
  char strxd[8][6] = { "xd00", "xd01", "xd02", "xd03", "xd04", "xd05", "xd06", "xd07", "xd08" };

  for (int i = 0; strxa[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(xa[i], top, xa[i] + 1, bottom, gridColor);
    osd->DrawText(xa[i], top + offset, cString(strxa[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; strxm[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(xm[i], top, xm[i] + 1, bottom, gridColor);
    osd->DrawText(xm[i], top + offset, cString(strxm[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; strxs[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(xs[i], top, xs[i] + 1, bottom, gridColor);
    osd->DrawText(xs[i], top + offset, cString(strxs[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; strxi[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(xi[i], top, xi[i] + 1, bottom, gridColor);
    osd->DrawText(xi[i], top + offset, cString(strxi[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; strxb[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(xb[i], top, xb[i] + 1, bottom, gridColor);
    osd->DrawText(xb[i], top + offset, cString(strxb[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; strxd[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(xd[i], top, xd[i] + 1, bottom, gridColor);
    osd->DrawText(xd[i], top + offset, cString(strxd[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  char stryt[11][6] = { "yt00", "yt01", "yt02", "yt03", "yt04", "yt05", "yt06", "yt07", "yt08", "yt09", "yt10" };
  char stryc[12][6] = { "yc00", "yc01", "yc02", "yc03", "yc04", "yc05", "yc06", "yc07", "yc08", "yc09", "yc10", "yc11" };
  char stryb[18][6] = { "yb00", "yb01", "yb02", "yb03", "yb04", "yb05", "yb06", "yb07", "yb08", "yb081", "yb082", "yb09", "yb10", "yb11", "yb12", "yb13", "yb14", "yb15" };
  char strym[8][6] = { "ym00", "ym01", "ym02", "ym03", "ym04", "ym05", "ym06", "ym07" };
  char strys[6][6] = { "ys00", "ys01", "ys02", "ys03", "ys04", "ys05" };
  char stryi[2][6] = { "yi00", "yi01" };
  char stryd[6][6] = { "yd00", "yd01", "yd02", "yd03", "yd04", "yd05" };

  offset = 0;
  for (int i = 0; stryt[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(left, yt[i], right, yt[i] + 1, gridColor);
    osd->DrawText(left + offset, yt[i], cString(stryt[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; stryc[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(left, yc[i], right, yc[i] + 1, gridColor);
    osd->DrawText(left + offset, yc[i], cString(stryc[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; stryb[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(left, yb[i], right, yb[i] + 1, gridColor);
    osd->DrawText(left + offset, yb[i], cString(stryb[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; strym[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(left, ym[i], right, ym[i] + 1, gridColor);
    osd->DrawText(left + offset, ym[i], cString(strym[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; strys[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(left, ys[i], right, ys[i] + 1, gridColor);
    osd->DrawText(left + offset, ys[i], cString(strys[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; stryi[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(left, yi[i], right, yi[i] + 1, gridColor);
    osd->DrawText(left + offset, yi[i], cString(stryi[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }

  for (int i = 0; stryd[i][0]; i++) {
    if ((i % 3) == 0)
      offset = 0;
    osd->DrawRectangle(left, yd[i], right, yd[i] + 1, gridColor);
    osd->DrawText(left + offset, yd[i], cString(stryd[i]), gridColor, clrTransparent, TinyFont);
    offset = offset + lineHeight;
  }
}
#endif

// --- cDrawDescription ----------------------------------------------------

cDrawDescription::cDrawDescription(cOsd *osd, AnimatedInfo_t animatedInfo) : cThread("LCARS DisplDesc")
{
  this->osd = osd;
  aI = animatedInfo;
  lineHeight = cFont::GetFont(fontSml)->Height();

  Start();
}

cDrawDescription::~cDrawDescription()
{
  Cancel(2);
  if (TextPixmap)
     osd->DestroyPixmap(TextPixmap);
  if (BracketPixmap)
     osd->DestroyPixmap(BracketPixmap);
  if (BackgroundPixmap)
     osd->DestroyPixmap(BackgroundPixmap);
}

void cDrawDescription::DrawBracket(void)
{
  int x0 = aI.x0;
  int x1 = aI.x1;
  int y0 = aI.y0;
  int y1 = aI.y1;
  if (BackgroundPixmap = osd->CreatePixmap(-1, cRect(x0, y0, x1 - x0, y1 - y0))) {
     BackgroundPixmap->SetAlpha(0);
     BackgroundPixmap->Fill((aI.viewmode == escaledvideo) ? Theme.Color(clrBackground) : aI.textColorBg);
     }

  BracketPixmap = osd->CreatePixmap(-1, cRect(x0, y0, x1 - x0, y1 - y0));
  if (!BracketPixmap)
     return;

  BracketPixmap->SetAlpha(0);
  BracketPixmap->Fill(clrTransparent);
  tColor Color = Theme.Color(clrMenuMainBracket);
  int lineHeight = cFont::GetFont(fontOsd)->Height("A");
  int x00, x01, x02, y00, y01, y02, y03, y04;
  x00 = 0;
  x01 = x00 + lineHeight;
  x02 = x1 - x01;
  y00 = 0;
  y01 = y00 + lineHeight / 2;
  y02 = y01 + lineHeight / 2;
  y04 = y1 - y0;
  y03 = y04 - lineHeight;
  BracketPixmap->DrawRectangle(cRect(x01, y00, x02, lineHeight / 2), Color);
  BracketPixmap->DrawEllipse  (cRect(x00, y00, lineHeight, y02), Color, 2);
  BracketPixmap->DrawEllipse  (cRect(x01, y01, lineHeight / 2, lineHeight / 2), Color, -2);
  BracketPixmap->DrawRectangle(cRect(x00, y02, lineHeight, y03 - y02), Color);
  BracketPixmap->DrawEllipse  (cRect(x00, y03, lineHeight, lineHeight), Color, 3);
  BracketPixmap->DrawEllipse  (cRect(x01, y03, lineHeight / 2, lineHeight / 2), Color, -3);
  BracketPixmap->DrawRectangle(cRect(x01, y04 - lineHeight / 2, x02, lineHeight / 2), Color);
}

void cDrawDescription::Draw(void)
{
  if (!BracketPixmap)
     return;

  const cEvent *Event = NULL;
  const cTimer *Timer = NULL;
  const cRecording *Recording = NULL;
  const cRecordingInfo *Info = NULL;

  if (aI.Recording) {
     Recording = aI.Recording;
     Info = Recording->Info();
     if (!Info || !Info->Description())
        return;
  } else if (aI.Event) {
     Event = aI.Event;
     if (!Event->Description())
        return;
  } else if (aI.Timer) {
     Timer = aI.Timer;
     Event = Timer->Event();
     if (!Event || !Event->Description())
        return;
  } else
     return;

  const char *titel = NULL;
  const char *shortText = NULL;
  const char *s = NULL;
  const cFont *font = cFont::GetFont(fontSml);
  cString parentalRating = "";
  BackgroundPixmap->SetAlpha(0);
  BracketPixmap->SetAlpha(0);
  tColor textColorBg = clrTransparent;

  if (Recording) {
     s = Info->Description();                   // text
     titel = Info->ChannelName();
     shortText = Info->ShortText();
     if (Info->GetEvent()->ParentalRating())
        parentalRating = Info->GetEvent()->GetParentalRatingString();
     }
  else {
     s = Event->Description();                  // text
     titel = Event->Title();
     shortText = Event->ShortText();
     if (Event->ParentalRating())
        parentalRating = Event->GetParentalRatingString();
     }
  
  int x0 = aI.x0 + 1.5 * lineHeight;            // text left
  int x1 = aI.x1;                               // text right
  int y0 = aI.y0 + 1.3 * lineHeight;            // text top
  int y1 = aI.y1 - 1.3 * lineHeight;            // text bottom

  int rand = Config.Margin + Gap;
  int textwidth = x1 - x0 - rand;
  int x00 = 1.5 * lineHeight + rand;
  int y00 = 1.3 * lineHeight;

  int w = 0;
  if (!isempty(parentalRating)) {
      cString buffer = cString::sprintf(" %s ", *parentalRating);
      w = font->Width(buffer);
      BracketPixmap->DrawText(cPoint(x00 + textwidth - w, y00), buffer, Theme.Color(clrMenuMainBracket), textColorBg, font, w); // parental rating
      }

  if (!isempty(titel)) {
     BracketPixmap->DrawText(cPoint(x00, y00), titel, aI.titleColorFg, textColorBg, font, textwidth - w); // title
     y00 = y00 + 1.3 * lineHeight;
     y0 = y0 + 1.3 * lineHeight;
     }

  bool space = false;
#if (APIVERSNUM >= 20505)
  if (Recording && Info->Errors() >= (1 - Config.displayError0)) {
     cString buffer = cString::sprintf("%s %i ", tr("TS Errors:"), Info->Errors());
     w = font->Width(buffer);
     BracketPixmap->DrawText(cPoint(x00 + textwidth - w, y00), buffer, Theme.Color(clrMenuMainBracket), textColorBg, font, w); // error in recording
     space = true;
     w += Gap;
     }
#endif

  if (!isempty(shortText)) {
     BracketPixmap->DrawText(cPoint(x00, y00), shortText, aI.shortTextColorFg, textColorBg, font, textwidth - w); // shorttext
     space = true;
     }

  if (space)
     y0 = y0 + 1.3 * lineHeight;
  y0 = y0 + 0.4 * lineHeight;

  wrapper.Set(s, font, textwidth - Gap);
  int l0 = wrapper.Lines();                    // textlines

  if (!s || isempty(s))
    return;

  int height = y1 - y0;                        // max height
  int lines = (int)(height / lineHeight);      // visible lines

  int l1 = min(l0, lines);                     // visible textlines -> scrollwindow

  int pixmapwidth = x1 - x0;
  int pixmapHeigh = l1 * lineHeight;

  TextPixmap = osd->CreatePixmap(-1, cRect(x0, y0, pixmapwidth, pixmapHeigh), cRect(0, 0, pixmapwidth, l0 * lineHeight));
  if (!TextPixmap) {
     return;
     }

  TextPixmap->Fill(clrTransparent);
  TextPixmap->SetAlpha(0);

  for (int i = 0; i < l0; i++) {
     TextPixmap->DrawText(cPoint(Config.Margin + Gap, i * lineHeight), wrapper.GetLine(i), aI.descriptionColorFg, textColorBg, font, textwidth); // description
     }

}

void cDrawDescription::DoSleep(int duration) {
  int sleepSlice = 10;
  for (int i = 0; Running() && (i * sleepSlice < duration); i++)
     cCondWait::SleepMs(sleepSlice);
}

void cDrawDescription::Action(void)
{
  DrawBracket();
  Draw();

  if (!(BackgroundPixmap && BracketPixmap && TextPixmap))
     return;

  int fadeinDelay = Config.waitTimeFadein;
  int scrollDelay = Config.waitTimeScroll;

  DoSleep(fadeinDelay);

  //FadeIn
  if (Config.fadeinTime) {
     int FadeTime = Config.fadeinTime;
     int FadeFrameTime = FadeTime / 10;
     uint64_t Start = cTimeMs::Now();
     while (Running()) {
        uint64_t Now = cTimeMs::Now();
        double t = std::min(double(Now - Start) / FadeTime, 1.0);
        int Alpha = t * ALPHA_OPAQUE;
        cPixmap::Lock();
        BackgroundPixmap->SetLayer(0);
        BracketPixmap->SetLayer(1);
        TextPixmap->SetLayer(2);
        BackgroundPixmap->SetAlpha(Alpha);
        BracketPixmap->SetAlpha(Alpha);
        TextPixmap->SetAlpha(Alpha);
        cPixmap::Unlock();
        if (Running() && osd)
           osd->Flush();
        int Delta = cTimeMs::Now() - Now;
        if (Running() && (Delta < FadeFrameTime))
           cCondWait::SleepMs(FadeFrameTime - Delta);
        if ((int)(Now - Start) > FadeTime)
           break;
        }
     }

  int maxY = std::max(0, TextPixmap->DrawPort().Height() - TextPixmap->ViewPort().Height());
  if (maxY == 0)
     return;

  int drawPortY = 0;
  int FrameTime = (int)(1000 / Config.framesPerSecond);
  bool doSleep = false;

  DoSleep(scrollDelay);

  // Scroll
  while (Running()) {
     uint64_t Now = cTimeMs::Now();
     cPixmap::Lock();
     drawPortY = TextPixmap->DrawPort().Y();
     drawPortY -= Config.scrollPixel;
     cPixmap::Unlock();
     if (std::abs(drawPortY) > maxY) {
        doSleep = true;
        DoSleep(scrollDelay);
        drawPortY = 0;
        }
     cPixmap::Lock();
     if (Running())
        TextPixmap->SetDrawPortPoint(cPoint(0, drawPortY));
     cPixmap::Unlock();
     if (Running() && osd) {
        osd->Flush();
        }
     if (doSleep) {
        doSleep = false;
        DoSleep(scrollDelay);
        }
     int Delta = cTimeMs::Now() - Now;
     if (Running() && (Delta < FrameTime))
        cCondWait::SleepMs(FrameTime - Delta);
     }
}
