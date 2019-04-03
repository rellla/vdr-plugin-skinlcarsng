/*
 * skinlcars.c: A VDR skin with Star Trek's "LCARS" layout
 *
 * See the main source file 'vdr.c' for copyright information and
 * how to reach the author.
 *
 * $Id: skinlcars.c 4.1 2015/09/01 10:07:07 kls Exp $
 */

#include "lcarsng.h"
#include "displaymenu.h"
#include <vdr/font.h>
#include <vdr/menu.h>
#include <vdr/osd.h>
#if APIVERSNUM > 20101
#include <vdr/positioner.h>
#endif
#include <vdr/themes.h>
#include <vdr/thread.h>
#include <vdr/tools.h>
#include <vdr/videodir.h>
#include <sys/statvfs.h>
#include <string>

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
  lastDiskUsageState = -1;
  lastDiskAlert = false;
  lastSystemLoad = -1;
  const cFont *font = cFont::GetFont(fontOsd);
  lineHeight = font->Height();
  tinyFont = CreateTinyFont(lineHeight);
  frameColor = Theme.Color(clrMenuFrameBg);
  currentIndex = -1;
  // The outer frame:
  int d = 5 * lineHeight;
  xa00 = 0;
  xa01 = xa00 + d / 2;
  xa02 = xa00 + d;
  xa03 = xa02 + lineHeight;
  xa04 = xa02 + d / 4;
  xa05 = xa02 + d;
  xa06 = xa05 + Gap;
  xa09 = cOsd::OsdWidth();
  xa08 = xa09 - lineHeight;
  xa07 = xa08 - Gap;

  yt00 = 0;
  yt01 = yt00 + lineHeight;
  yt02 = yt01 + lineHeight;
  yt03 = yt01 + d / 4;
  yt04 = yt02 + Gap;
  yt05 = yt00 + d / 2;
  yt06 = yt04 + 2 * lineHeight;
  yt07 = yt06 + Gap;
  yt08 = yt07 + 2 * lineHeight;

//  yc00 = yt06 + Gap;
  yc00 = yt08 + Gap;
//  yc05 = yc00 + 3 * lineHeight + Gap / 2; // Button in der Mitte
  yc05 = yc00 + 5 * lineHeight + Gap / 2;
  yc04 = yc05 - lineHeight;
  yc03 = yc04 - lineHeight;
  yc02 = yc04 - d / 4;
  yc01 = yc05 - d / 2;

  yc06 = yc05 + Gap;
//  yc06 = yc05 + 2 * lineHeight + Gap; // Button in der Mitte
  yc07 = yc06 + lineHeight;
  yc08 = yc07 + lineHeight;
  yc09 = yc07 + d / 4;
  yc10 = yc06 + d / 2;
  yc11 = yc06 + 3 * lineHeight + Gap / 2;

  yb00 = yc11 + Gap;
  yb01 = yb00 + 2 * lineHeight;
  yb02 = yb01 + Gap;
  yb03 = yb02 + 2 * lineHeight; // Load
  yb04 = yb03 + Gap;
  yb05 = yb04 + 2 * lineHeight; // Load %
  yb06 = yb05 + Gap;
  yb07 = yb06 + 2 * lineHeight; //Recordings
  yb08 = yb07 + Gap;
  yb081 = yb08 + 2 * lineHeight; //Timer
  yb082 = yb081 + Gap;

  yb15 = cOsd::OsdHeight();
  yb14 = yb15 - lineHeight;
  yb13 = yb14 - lineHeight;
  yb12 = yb14 - d / 4;
  yb11 = yb15 - d / 2;
//  yb10 = yb13 - Gap - 2 * lineHeight; // VDR
  yb10 = yb13 - Gap - lineHeight;
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
  xm02 = xa06;
  xm08 = (xa09 + xa00) / 2;
  xm07 = xm08 - lineHeight;
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
  ys01 = ys00 + lineHeight;
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
  int r = lineHeight;
//  xd07 = xa09;
  xd07 = xm05 - Gap;
  xd06 = xd07 - r;
  xd05 = xd06 - 6 * r;
  xd04 = xd05 - r;
  xd03 = xd04 - Gap;
  xd02 = xd03 - r;
  xd01 = xd02 - 6 * r;
  xd00 = xd01 - r;
  yd00 = yt00;
  yd05 = yc04 - 3 * Gap;
//  yd05 = yc06 - Gap; // Button in der Mitte
  yd04 = yd05 - r; //0.85 * r; // Button in der Mitte
  yd03 = yd04 - Gap;
  yd02 = yd03 - r; //0.85 * r; // Button in der Mitte
  yd01 = yd02 - Gap;

  xs = 0;

  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop(), xa00, yt00, xa09 - 1, yb15 - 1);
  DrawMenuFrame();
}

cLCARSNGDisplayMenu::~cLCARSNGDisplayMenu()
{
  delete tallFont;
  delete tinyFont;
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayMenu::SetMenuCategory(eMenuCategory MenuCategory)
{
  if (initial || MenuCategory != cSkinDisplayMenu::MenuCategory()) {
     cSkinDisplayMenu::SetMenuCategory(MenuCategory);
     initial = true;
     osd->DrawRectangle(xa00, yt00, xa09 - 1, yb15 - 1, Theme.Color(clrBackground));
     switch (MenuCategory) {
        case mcMain:
        case mcSetup:
//        case mcCommand:
           osd->DrawRectangle( xs00, 0, xa09, yc06 - 1, clrTransparent);
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
           break;
        case mcChannel:
           osd->DrawRectangle(xa00, yt00, xa09 - 1, yb15 - 1, clrTransparent);
           yi00 = yt04 + lineHeight;
           yi01 = ym04;
           xi00 = xm00;
           xi01 = xm03;
           xi02 = xm04;
           xi03 = xm05;
           DrawMainFrameChannel();
           DrawMainBracket();
           break;
        case mcCommand:
        case mcSchedule:
        case mcScheduleNow:
        case mcScheduleNext:
        case mcEvent:
        case mcRecording:
        case mcRecordingInfo:
        case mcRecordingEdit:
        case mcTimer:
        case mcTimerEdit:
           osd->DrawRectangle( xs00, 0, xa09, yc06 - 1, clrTransparent);
           yi00 = ym00;
           yi01 = ym07;
           xi00 = xa03;
           xi01 = xa07;
           xi02 = xa08;
           xi03 = xa09;
           DrawMainFrameLower();
           DrawMainBracket();
           break;
        default:
           yi00 = yt02;
           yi01 = yb13;
           xi00 = xa03;
           xi01 = xa07;
           xi02 = xa08;
           xi03 = xa09;
           DrawMenuFrame();
        }
     }
}

void cLCARSNGDisplayMenu::DrawMainFrameUpper(tColor Color)
{
  // Top left rectangles:
//  osd->DrawRectangle(xa00, yt00, xa02 - 1, yt02 - 1, Color);
  osd->DrawRectangle(xa00, yt00, xa02 - 1, yt08 - 1, Color);
//  osd->DrawRectangle(xa00, yt04, xa02 - 1, yt06 - 1, Color);
//  osd->DrawRectangle(xa00, yt07, xa02 - 1, yt08 - 1, Color);
  osd->DrawRectangle(xa00, yt02, xa02 - 1, yt04 - 1, Theme.Color(clrBackground));
  osd->DrawRectangle(xa00, yt06, xa02 - 1, yt07 - 1, Theme.Color(clrBackground));
  // Upper elbow:
  osd->DrawRectangle(xa00, yc00, xa01 - 1, yc01 - 1, Color);
  osd->DrawEllipse  (xa00, yc01, xa01 - 1, yc05 - 1, Color, 3);
  osd->DrawRectangle(xa01, yc00, xa02 - 1, yc05 - 1, Color);
  osd->DrawEllipse  (xa02, yc02, xa04 - 1, yc04 - 1, Color, -3);
  osd->DrawRectangle(xa02, yc04, xa05 - 1, yc05 - 1, Color);
  osd->DrawRectangle(xa00, yc00 + 2 * lineHeight, xa02 - 1, yc00 + 2 * lineHeight + Gap, Theme.Color(clrBackground));
  // Upper delimiter:
  osd->DrawRectangle(xa06, yc04 + lineHeight / 2, xm03 - 1, yc05 - 1, Color);
  osd->DrawRectangle(xm03 + Gap, yc04 + lineHeight / 2, xm07 - 1, yc05 - 1, Color);
  // Top right rectangles:
  osd->DrawRectangle(xm07, yt00, xm08 - 1, yc04 -Gap - 1, Color);
  osd->DrawEllipse  (xm07, yc04, xm08 - 1, yc05 - 1, Color, 4);
  osd->DrawEllipse  (xm05, yc04, xm07 - 1, yc05 - 1 - lineHeight / 2, Color, -4);
}

void cLCARSNGDisplayMenu::DrawMainFrameLower(void)
{
// unterer gelber Rahmen
  const cFont *font = cFont::GetFont(fontOsd);
  // Lower elbow:
  osd->DrawRectangle(xa00, yc10, xa01 - 1, yc11 - 1, frameColor);
  osd->DrawEllipse  (xa00, yc06, xa01 - 1, yc10 - 1, frameColor, 2);
  osd->DrawRectangle(xa01, yc06, xa02 - 1, yc11 - 1, frameColor);
  osd->DrawEllipse  (xa02, yc07, xa04 - 1, yc09 - 1, frameColor, -2);
  osd->DrawRectangle(xa02, yc06, xa05 - 1, yc07 - 1, frameColor);
  // Lower delimiter:
  osd->DrawRectangle(xa06, yc06, xm03 - 1, yc07 - lineHeight / 2 - 1, frameColor);
  osd->DrawRectangle(xm03 + Gap, yc06, xm08 - 1, yc07 - 1, frameColor);
  osd->DrawRectangle(xm08 + Gap, yc06, xs00 - Gap - 1, yc07 - 1, frameColor);
  // VDR version:
  osd->DrawRectangle(xa00, yb10, xa02 - 1, yb15 - 1, frameColor);
  osd->DrawText(xa00, yb10, cString::sprintf("%s-%s", "VDR", VDRVERSION), Theme.Color(clrMenuFrameFg), frameColor, font, xa02 - xa00, yb11 - yb10, taTop | taRight | taBorder);
  osd->DrawText(xa00, yb15 - lineHeight, "LCARSNG", Theme.Color(clrMenuFrameFg), frameColor, font, xa02 - xa00, lineHeight, taBottom | taRight | taBorder);
}

void cLCARSNGDisplayMenu::DrawMainFrameChannel(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  // Upper elbow:
  osd->DrawRectangle(xa00, yt05, xa01 - 1, yt06 - 1, frameColor);
  osd->DrawRectangle(xa00, yt00, xa01 - 1, yt05 - 1, clrTransparent);
  osd->DrawEllipse  (xa00, yt00, xa01 - 1, yt05 - 1, frameColor, 2);
  osd->DrawRectangle(xa01, yt00, xa02 - 1, yt06 - 1, frameColor);
  osd->DrawEllipse  (xa02, yt01, xa04 - 1, yt03 - 1, frameColor, -2);
  osd->DrawRectangle(xa02, yt00, xa05 - 1, yt01 - 1, frameColor);
  osd->DrawRectangle(xm04, yt00, xm07 - Gap - 1, yt01 - 1, frameColor);
  osd->DrawRectangle(xm07, yt00, xm07 + lineHeight / 2 -1, yt01 - 1, frameColor);
  osd->DrawEllipse  (xm07 + lineHeight / 2, yt00, xm08 - 1, yt01 - 1, frameColor, 5);
  // Center part:
  osd->DrawRectangle(xa00, yt06 + Gap, xa02 - 1, yc00 - 1 - Gap, frameColor);
  osd->DrawRectangle(xa00, yc00, xa02 - 1, yc11 - 1, frameColor);
  // VDR version:
  osd->DrawRectangle(xa00, yb10, xa02 - 1, yb15 - 1, frameColor);
  osd->DrawText(xa00, yb10, cString::sprintf("%s-%s", "VDR", VDRVERSION), Theme.Color(clrMenuFrameFg), frameColor, font, xa02 - xa00, yb11 - yb10, taTop | taRight | taBorder);
  osd->DrawText(xa00, yb15 - lineHeight, "LCARSNG", Theme.Color(clrMenuFrameFg), frameColor, font, xa02 - xa00, lineHeight, taBottom | taRight | taBorder);
}

void cLCARSNGDisplayMenu::DrawMainButton(const char *Text, int x0, int x1, int x2, int x3, int y0, int y1, tColor ColorFg, tColor ColorBg, const cFont *Font)
{
  int h = y1 - y0;
  osd->DrawEllipse(x0, y0, x1 - 1, y1 - 1, ColorBg, 7);
//  osd->DrawText(x1, y0, Text, ColorFg, ColorBg, Font, x2 - x1, h, taBottom | taRight);
  osd->DrawText(x1, y0, Text, ColorFg, ColorBg, Font, x2 - x1, h, taCenter);
  osd->DrawEllipse(x2, y0, x3 - 1, y1 - 1, ColorBg, 5);
}

void cLCARSNGDisplayMenu::DrawMenuFrame(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  // Upper elbow:
  osd->DrawRectangle(xa00, yt05, xa01 - 1, yt06 - 1, frameColor);
  osd->DrawRectangle(xa00, yt00, xa01 - 1, yt05 - 1, clrTransparent);
  osd->DrawEllipse  (xa00, yt00, xa01 - 1, yt05 - 1, frameColor, 2);
  osd->DrawRectangle(xa01, yt00, xa02 - 1, yt06 - 1, frameColor);
  osd->DrawEllipse  (xa02, yt01, xa04 - 1, yt03 - 1, frameColor, -2);
  osd->DrawRectangle(xa02, yt00, xa05 - 1, yt01 - 1, frameColor);
  osd->DrawRectangle(xa06, yt00, xa07 - 1, yt01 - 1, frameColor);
  osd->DrawRectangle(xa08, yt00, xa08 + lineHeight / 2 - 1, yt01 - 1, frameColor);
  osd->DrawRectangle(xa08 + lineHeight / 2, yt00, xa09 - 1, yt00 + lineHeight / 2 - 1, clrTransparent);
  osd->DrawEllipse  (xa08 + lineHeight / 2, yt00, xa09 - 1, yt01 - 1, frameColor, 5);
  // Center part:
  osd->DrawRectangle(xa00, yt06 + Gap, xa02 - 1, yc00 - 1 - Gap, frameColor);
  osd->DrawRectangle(xa00, yc00, xa02 - 1, yc11 - 1, frameColor);
  // Lower elbow:
  osd->DrawRectangle(xa00, yb10, xa02 - 1, yb11 - 1, frameColor);
  osd->DrawRectangle(xa00, yb11, xa01 - 1, yb15 - 1, clrTransparent);
  osd->DrawEllipse  (xa00, yb11, xa01 - 1, yb15 - 1, frameColor, 3);
  osd->DrawRectangle(xa01, yb11, xa02 - 1, yb15 - 1, frameColor);
  osd->DrawEllipse  (xa02, yb12, xa04 - 1, yb14 - 1, frameColor, -3);
  osd->DrawRectangle(xa02, yb14, xa05 - 1, yb15 - 1, frameColor);
  osd->DrawRectangle(xa08, yb14, xa08 + lineHeight / 2 - 1, yb15 - 1, frameColor);
  osd->DrawRectangle(xa08 + lineHeight / 2, yb14 + lineHeight / 2, xa09 - 1, yb15 - 1, clrTransparent);
  osd->DrawEllipse  (xa08 + lineHeight / 2, yb14, xa09 - 1, yb15 - 1, frameColor, 5);
  osd->DrawText(xa00, yb10, cString::sprintf("%s-%s", "VDR", VDRVERSION), Theme.Color(clrMenuFrameFg), frameColor, font, xa02 - xa00, yb11 - yb10, taTop | taRight | taBorder);
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
  if (initial || !*lastDate || strcmp(s, lastDate)) {
     const cFont *font = cFont::GetFont(fontOsd);
     tColor ColorFg = Theme.Color(clrDateFg);
     tColor ColorBg = Theme.Color(clrDateBg);
     lastDate = s;
     const char *t = strrchr(s, ' ');
     osd->DrawText(xa00, yb01 - lineHeight, t, ColorFg, ColorBg, font, xa02 - xa00, lineHeight, taBottom | taRight | taBorder);
     s.Truncate(t - s);
     osd->DrawText(xa00, yb00, s, ColorFg, ColorBg, font, xa02 - xa00, yb01 - yb00 - lineHeight, taTop | taRight | taBorder);
     }
}

void cLCARSNGDisplayMenu::DrawDisk(void)
{
  if (yb02) {
     if (cVideoDiskUsage::HasChanged(lastDiskUsageState) || initial) { // must call HasChanged() first, or it shows an outdated value in the 'initial' case!
        const cFont *font = cFont::GetFont(fontOsd);
        int DiskUsage = cVideoDiskUsage::UsedPercent();
        bool DiskAlert = DiskUsage > DISKUSAGEALERTLIMIT;
        int freemb = FreeMB(currentTitle, initial);
        int minutes = 0;
        {
        LOCK_RECORDINGS_READ;
        double MBperMinute = Recordings->MBperMinute();
        minutes = int(double(freemb) / (MBperMinute > 0 ? MBperMinute : MB_PER_MINUTE));
        }
        tColor ColorFg = DiskAlert ? Theme.Color(clrAlertFg) : Theme.Color(clrMenuFrameFg);
        tColor ColorBg = DiskAlert ? Theme.Color(clrAlertBg) : frameColor;
        if (initial || DiskAlert != lastDiskAlert)
           osd->DrawText(xa00, yb02, tr("DISK"), ColorFg, ColorBg, tinyFont, xa02 - xa00, yb03 - yb02, taTop | taLeft | taBorder);
        osd->DrawText(xa01, yb02, cString::sprintf("%02d%s", DiskUsage, "%"), ColorFg, ColorBg, font, xa02 - xa01, lineHeight, taBottom | taRight | taBorder);
        osd->DrawText(xa00, yb03 - lineHeight, freemb ? cString::sprintf("%02d:%02d", minutes / 60, minutes % 60) : cString::sprintf("%02d:%02d", cVideoDiskUsage::FreeMinutes() / 60, cVideoDiskUsage::FreeMinutes() % 60), ColorFg, ColorBg, font, xa02 - xa00, 0, taBottom | taRight | taBorder);
        lastDiskAlert = DiskAlert;
        }
     }
}

void cLCARSNGDisplayMenu::DrawLoad(void)
{
  if (yb04) {
     tColor ColorFg = Theme.Color(clrMenuFrameFg);
     tColor ColorBg = frameColor;
     if (initial)
        osd->DrawText(xa00, yb04, tr("LOAD"), ColorFg, ColorBg, tinyFont, xa02 - xa00, yb05 - yb04, taTop | taLeft | taBorder);
     double SystemLoad;
     if (getloadavg(&SystemLoad, 1) > 0) {
        if (initial || SystemLoad != lastSystemLoad) {
           osd->DrawText(xa00, yb05 - lineHeight, cString::sprintf("%.1f", SystemLoad), ColorFg, ColorBg, cFont::GetFont(fontOsd), xa02 - xa00, lineHeight, taBottom | taRight | taBorder);
           lastSystemLoad = SystemLoad;
           }
        }
     }
}

void cLCARSNGDisplayMenu::DrawNumRecordingsInPath(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  int NumRecordingsInPath = 0;
  {
#if APIVERSNUM > 20300
  LOCK_RECORDINGS_READ;
#endif
  NumRecordingsInPath = Recordings->GetNumRecordingsInPath(cMenuRecordings::GetActualPath());
  }
  if (NumRecordingsInPath > 0)
     osd->DrawText(xm04, ys00, cString::sprintf("%i", NumRecordingsInPath), Theme.Color(clrMenuFrameFg), frameColor, font, xm08 - xm04 - 1, lineHeight, taBottom | taRight | taBorder);
}

void cLCARSNGDisplayMenu::DrawCountRecordings(void)
{
  if (yb06) {
     const cFont *font = cFont::GetFont(fontOsd);
     tColor ColorFg = Theme.Color(clrMenuFrameFg);
     tColor ColorBg = frameColor;
     int CountRecordings = 0;
     {
#if APIVERSNUM > 20300
     LOCK_RECORDINGS_READ;
#endif
     CountRecordings = Recordings->Count();
     }
     osd->DrawText(xa00, yb06, tr("RECORDINGS"), ColorFg, ColorBg, tinyFont, xa02 - xa00, yb07 - yb06, taTop | taLeft | taBorder);
     osd->DrawText(xa00, yb07 - lineHeight, cString::sprintf("%i", CountRecordings), Theme.Color(clrMenuFrameFg), frameColor, font, xa02 - xa00, lineHeight, taBottom | taRight | taBorder);
     }
}

void cLCARSNGDisplayMenu::DrawCountTimers(void)
{
  if (yb08) {
     const cFont *font = cFont::GetFont(fontOsd);
     tColor ColorFg = Theme.Color(clrMenuFrameFg);
     tColor ColorBg = frameColor;
     int CountTimers = 0;
#if APIVERSNUM > 20300
     LOCK_TIMERS_READ;
     for (const cTimer *Timer = Timers->First(); Timer; Timer = Timers->Next(Timer)) {
#else
     for (cTimer *Timer = Timers.First(); Timer; Timer = Timers.Next(Timer)) {
#endif
        if (Timer->HasFlags(tfActive))
           CountTimers++;
        }
     osd->DrawText(xa00, yb08, tr("TIMER"), ColorFg, ColorBg, tinyFont, xa02 - xa00, yb081 - yb08, taTop | taLeft | taBorder);
     osd->DrawText(xa00, yb081 - lineHeight, itoa(CountTimers), Theme.Color(clrMenuFrameFg), frameColor, font, xa02 - xa00, lineHeight, taBottom | taRight | taBorder);
     }
}

void cLCARSNGDisplayMenu::DrawMainBracket(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  tColor Color = Theme.Color(clrMenuMainBracket);
  int y0, y1, y2, y3;
  if (MenuCategory() == mcChannel) {
     y0 = yt00 + lineHeight *2;
     y1 = y0 + lineHeight / 2;
     y2 = y1 + lineHeight / 2;
     y3 = y2 + Gap;
     }
  else {
     y0 = ym00; //yc08
     y1 = ym01; //ym00 + lineHeight / 2
     y2 = ym02; //ym01 + lineHeight / 2
     y3 = ym03; //ym02 + Gap
     }
  if (MenuCategory() != mcSchedule && MenuCategory() != mcScheduleNow && MenuCategory() != mcScheduleNext && MenuCategory() != mcEvent && MenuCategory() != mcRecording && MenuCategory() != mcRecordingInfo && MenuCategory() != mcRecordingEdit && MenuCategory() != mcTimer && MenuCategory() != mcTimerEdit && MenuCategory() != mcCommand) {
     osd->DrawRectangle(xm00, y0, xm01 - 1, y1 - 1, Color);
     osd->DrawRectangle(xm02, y0, xm07 - 1, y1 - 1, Color);
     osd->DrawEllipse  (xm07, y0, xm08 - 1, y2 - 1, Color, 1);
     osd->DrawEllipse  (xm06, y1, xm07 - 1, y2 - 1, Color, -1);
     osd->DrawRectangle(xm07, y3, xm08 - 1, ym04 - 1, Color);
     osd->DrawEllipse  (xm06, ym05, xm07 - 1, ym06 - 1, Color, -4);
     osd->DrawEllipse  (xm07, ym05, xm08 - 1, ym07 - 1, Color, 4);
     osd->DrawRectangle(xm02, ym06, xm07 - 1, ym07 - 1, Color);
     osd->DrawRectangle(xm00, ym06, xm01 - 1, ym07 - 1, Color);
     }
  if (MenuCategory() == mcSetup)
     osd->DrawText(xm02, ys00, tr("Setup"), Theme.Color(clrMenuFrameFg), frameColor, font, xm04 - xm02 - Gap, lineHeight, taBottom | taLeft | taBorder);
//  if (MenuCategory() == mcCommand)
//     osd->DrawText(xm02, ys00, tr("Commands"), Theme.Color(clrMenuFrameFg), frameColor, font, xm04 - xm02 - Gap, lineHeight, taBottom | taLeft | taBorder);
  if (MenuCategory() == mcChannel)
     osd->DrawText(xm02, yt00, tr("Channels"), Theme.Color(clrMenuFrameFg), frameColor, font, xm04 - xm02 - Gap, lineHeight, taBottom | taLeft | taBorder);
  if (MenuCategory() != mcMain && MenuCategory() != mcSchedule && MenuCategory() != mcScheduleNow && MenuCategory() != mcScheduleNext && MenuCategory() != mcEvent && MenuCategory() != mcRecording && MenuCategory() != mcRecordingInfo && MenuCategory() != mcRecordingEdit && MenuCategory() != mcTimer && MenuCategory() != mcTimerEdit && MenuCategory() != mcCommand) {
     osd->DrawRectangle(xm04 - Gap, y0, xm04, ym01 - 1, clrTransparent);
     osd->DrawRectangle(xm04 - Gap, ym06, xm04, ym07 - 1, clrTransparent);
     }
}

void cLCARSNGDisplayMenu::DrawStatusElbows(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  osd->DrawText     (xs00, ys00, tr("TIMERS"), Theme.Color(clrMenuFrameFg), frameColor, font, xs01 - xs00, lineHeight, taBottom | taLeft | taBorder);
  osd->DrawRectangle(xs02, ys00, xs03 - 1, ys01 - 1, frameColor);
  osd->DrawEllipse  (xs03, ys00, xs05 - 1, ys01 - 1, frameColor, 1);
  osd->DrawEllipse  (xs03, ys01, xs04 - 1, ys02 - 1, frameColor, -1);
  osd->DrawRectangle(xs04, ys01, xs05 - 1, ys03 - 1, frameColor);
  osd->DrawRectangle(xs04, ys04, xs05 - 1, ys05 - 1, frameColor);
  osd->DrawText     (xs10, ys00, tr("DEVICES"), Theme.Color(clrMenuFrameFg), frameColor, font, xs11 - xs10, lineHeight, taBottom | taRight | taBorder);
  osd->DrawRectangle(xs08, ys00, xs09 - 1, ys01 - 1, frameColor);
  osd->DrawEllipse  (xs06, ys00, xs08 - 1, ys01 - 1, frameColor, 2);
  osd->DrawEllipse  (xs07, ys01, xs08 - 1, ys02 - 1, frameColor, -2);
  osd->DrawRectangle(xs06, ys01, xs07 - 1, ys03 - 1, frameColor);
  osd->DrawRectangle(xs06, ys04, xs07 - 1, ys05 - 1, frameColor);
  osd->DrawRectangle(xs12, ys00, xs13 - 1, ys01 - 1, frameColor);
}

void cLCARSNGDisplayMenu::DrawFrameDisplay(void)
{
//  if (MenuCategory() != mcChannel) {
     DrawDate();
     DrawDisk();
     DrawLoad();
     if (initial) {
        if (yb06)
           osd->DrawRectangle(xa00, yb06, xa02 - 1, yb07 - 1, frameColor);
        if (yb08)
           osd->DrawRectangle(xa00, yb08, xa02 - 1, yb081 - 1, frameColor);
        if (yb082) {
//           const cFont *font = cFont::GetFont(fontOsd);
           osd->DrawRectangle(xa00, yb082, xa02 - 1, yb09 - 1, frameColor);
//           osd->DrawText(xa00, yb09 - lineHeight - Gap, "LCARSNG", Theme.Color(clrMenuFrameFg), frameColor, font, xa02 - xa00, lineHeight, taBottom | taRight | taBorder);
           }
        }
     DrawCountRecordings();
     DrawCountTimers();
     if (MenuCategory() == mcRecording)
        DrawNumRecordingsInPath();
//     }
}

void cLCARSNGDisplayMenu::DrawScrollbar(int Total, int Offset, int Shown, bool CanScrollUp, bool CanScrollDown)
{
  int x0, x1, tt, tb;
  tColor ClearColor;
  if (MenuCategory() == mcMain || MenuCategory() == mcSetup) { //|| MenuCategory() == mcCommand) {
     x0 = xm07;
     x1 = xm08;
     tt = ym03;
     tb = ym04;
     ClearColor = Theme.Color(clrMenuMainBracket);
     }
  else if (MenuCategory() == mcChannel) {
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
     if (MenuCategory() == mcSchedule || MenuCategory() == mcScheduleNow || MenuCategory() == mcScheduleNext || MenuCategory() == mcEvent || MenuCategory() == mcRecording || MenuCategory() == mcRecordingInfo || MenuCategory() == mcRecordingEdit || MenuCategory() == mcTimer || MenuCategory() == mcTimerEdit || MenuCategory() == mcCommand) {
        tt = yb00;
        tb = yb07 + lineHeight + Gap;
        }
     else {
        tt = yc00;
        tb = yc11;
        if (CanScrollUp)
           osd->DrawBitmap(xa02 - bmArrowUp.Width() - d, tt + d, bmArrowUp, Theme.Color(clrMenuScrollbarArrow), frameColor);
        else
           osd->DrawRectangle(xa02 - bmArrowUp.Width() - d, tt + d, xa02 - d - 1, tt + d + bmArrowUp.Height() - 1, frameColor);
        if (CanScrollDown)
           osd->DrawBitmap(xa02 - bmArrowDown.Width() - d, tb - d - bmArrowDown.Height(), bmArrowDown, Theme.Color(clrMenuScrollbarArrow), frameColor);
        else
           osd->DrawRectangle(xa02 - bmArrowDown.Width() - d, tb - d - bmArrowDown.Height(), xa02 - d - 1, tb - d - 1, frameColor);
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
//  else if (MenuCategory() != mcMain && MenuCategory() != mcSetup && MenuCategory() != mcCommand && MenuCategory() != mcChannel)
  else if (MenuCategory() != mcMain && MenuCategory() != mcSetup && MenuCategory() != mcChannel)
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
     Date = cString::sprintf("-%s", *TimeString(Timer->StopTime()));
  else {
     time_t Now = time(NULL);
     cString Today = WeekDayName(Now);
     cString Time = TimeString(Timer->StartTime());
     cString Day = WeekDayName(Timer->StartTime());
     if (Timer->StartTime() > Now + 6 * SECSINDAY)
        Date = DayDateTime(Timer->StartTime());
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
     osd->DrawText(xs02, ys00, itoa(NumTimers), Theme.Color(clrMenuFrameFg), frameColor, font, xs03 - xs02, ys01 - ys00, taBottom | taLeft | taBorder);
     osd->DrawText(xs08, ys00, itoa(NumDevices), Theme.Color(clrMenuFrameFg), frameColor, font, xs09 - xs08, ys01 - ys00, taBottom | taRight | taBorder);
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
        tColor ColorBg = Theme.Color(clrChannelFrameBg);
        osd->DrawRectangle(xs12, y, xs12 + lineHeight / 2 - 1, y + lineHeight - 1, ColorBg);
        osd->DrawEllipse  (xs12 + lineHeight / 2, y, xs13 - 1, y + lineHeight - 1, ColorBg, 5);
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
           osd->DrawBitmap((xs12 + xs13 - bm->Width()) / 2, y + (lineHeight - bm->Height()) / 2, *bm, Theme.Color(clrChannelFrameFg), ColorBg);
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
  if (initial) {
     DrawMainFrameUpper(Theme.Color(clrChannelFrameBg));
     osd->DrawText(xd00, yd00, tr("LIVE"), Theme.Color(clrChannelFrameBg), Theme.Color(clrBackground), tallFont, xd07 - xd00, yt02 - yd00, taTop | taRight | taBorder);
     }
  if (!Channel)
     return;
  if (initial || Channel != lastChannel || strcmp(Channel->Name(), lastChannelName)) {
     osd->DrawText(xa00, yt04, itoa(Channel->Number()), Theme.Color(clrChannelFrameFg), Theme.Color(clrChannelFrameBg), tallFont, xa02 - xa00, yt06 - yt04, taTop | taRight | taBorder);
     osd->DrawText(xa03, yt04, Channel->Name(), Theme.Color(clrChannelName), Theme.Color(clrBackground), tallFont, xd07 - xa03, yt06 - yt04, taTop | taLeft);
     int x = xa00 + (yc03 - yc02); // compensate for the arc
//     osd->DrawText(x, yc00, cSource::ToString(Channel->Source()), Theme.Color(clrChannelFrameFg), Theme.Color(clrChannelFrameBg), cFont::GetFont(fontOsd), xa02 - x, yc03 - yc00, taTop | taRight | taBorder);
     osd->DrawText(x, yc00, cSource::ToString(Channel->Source()), Theme.Color(clrChannelFrameFg), Theme.Color(clrChannelFrameBg), cFont::GetFont(fontOsd), xa02 - x, 2 * lineHeight, taTop | taRight | taBorder);
     lastChannel = Channel;
     lastChannelName = Channel->Name();
     DrawSeen(0, 0);
     }
  // The current programme:
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
  if (initial) {
     DrawMainFrameUpper(Theme.Color(clrReplayFrameBg));
     osd->DrawText(xd00, yd00, tr("PLAY"), Theme.Color(clrReplayFrameBg), Theme.Color(clrBackground), tallFont, xd07 - xd00, yt02 - yd00, taTop | taRight | taBorder);
     }
  // The current progress:
  int Current = 0;
  int Total = 0;
  if (Control->GetIndex(Current, Total))
     DrawSeen(Current, Total);
  // The current programme:
  if (const cRecording *Recording = Control->GetRecording()) {
     if (initial || Recording != lastRecording) {
        const cFont *font = cFont::GetFont(fontOsd);
        if (const cRecordingInfo *Info = Recording->Info()) {
           osd->DrawText(xa03, yt04, Info->ChannelName(), Theme.Color(clrChannelName), Theme.Color(clrBackground), tallFont, xd07 - xa03, yt06 - yt04, taTop | taLeft);
           DrawInfo(Info->GetEvent(), false);
           }
        else
           osd->DrawText(xa03, yt04, Recording->Name(), Theme.Color(clrEventTitle), Theme.Color(clrBackground), font, xd07 - xa03, 0, taTop | taLeft);
        osd->DrawText(xa00, yt07, ShortDateString(Recording->Start()), Theme.Color(clrReplayFrameFg), Theme.Color(clrReplayFrameBg), font, xa02 - xa00, 0, taTop | taRight | taBorder);
        osd->DrawText(xa00, yt07 + lineHeight, TimeString(Recording->Start()), Theme.Color(clrReplayFrameFg), Theme.Color(clrReplayFrameBg), font, xa02 - xa00, 0, taBottom | taRight | taBorder);
        lastRecording = Recording;
        }
     }
  else {
     cString Header = Control->GetHeader();
     if (!*lastHeader || strcmp(Header, lastHeader)) {
        osd->DrawText(xa03, yt04, Header, Theme.Color(clrMenuText), Theme.Color(clrBackground), tallFont, xd07 - xa03, 0, taTop | taLeft);
        lastHeader = Header;
        }
     }
}

void cLCARSNGDisplayMenu::DrawInfo(const cEvent *Event, bool WithTime)
{
  if (Event) {
     const cFont *font = cFont::GetFont(fontOsd);
     int y = yt07;
     osd->DrawText(xa03, y, Event->Title(), Theme.Color(clrEventTitle), Theme.Color(clrBackground), font, xd07 - xa03 - lineHeight, lineHeight, taBottom | taLeft);
     y += lineHeight;
     osd->DrawText(xa03, y, Event->ShortText(), Theme.Color(clrEventShortText), Theme.Color(clrBackground), cFont::GetFont(fontSml), xd07 - xa03 - lineHeight, lineHeight, taTop | taLeft);
     if (WithTime) {
        osd->DrawText(xa00, y - lineHeight, Event->GetTimeString(), Theme.Color(clrChannelFrameFg), Theme.Color(clrChannelFrameBg), font, xa02 - xa00, lineHeight, taTop | taRight | taBorder);
        osd->DrawText(xa00, y, cString::sprintf("-%s", *Event->GetEndTimeString()), Theme.Color(clrChannelFrameFg), Theme.Color(clrChannelFrameBg), font, xa02 - xa00, lineHeight, taBottom | taRight | taBorder);
        }
     }
}

void cLCARSNGDisplayMenu::DrawSeen(int Current, int Total)
{
// Fortschrittsbalken
  int Seen = (Total > 0) ? min(xm03 - xm02, int((xm03 - xm02) * double(Current) / Total)) : 0;
  if (initial || Seen != lastSeen) {
     int y0 = yc04 - ShowSeenExtent;
     int y1 = yc04 + lineHeight / 2 - Gap / 2;
     osd->DrawRectangle(xm02, y0, xm02 + Seen - 1, y1 - 1, Theme.Color(clrSeen));
     osd->DrawRectangle(xm02 + Seen, y0, xm03 - 1, y1 - 1, Theme.Color(clrBackground));
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
     case mcSetup:
//     case mcCommand:
        return (ym04 - ym03) / lineHeight;
        break;
     case mcChannel:
        return (ym04 - yt04 - lineHeight) / lineHeight;
        break;
     case mcCommand:
     case mcSchedule:
     case mcScheduleNow:
     case mcScheduleNext:
     case mcEvent:
     case mcRecording:
     case mcRecordingInfo:
     case mcRecordingEdit:
     case mcTimer:
     case mcTimerEdit:
        return (ym07 - ym00) / lineHeight;
        break;
     default:
        return (yb13 - yt02) / lineHeight;
     }
}

void cLCARSNGDisplayMenu::Clear(void)
{
  textScroller.Reset();
  osd->DrawRectangle(xi00, yi00, xi03 - 1, yi01 - 1, Theme.Color(clrBackground));
}

void cLCARSNGDisplayMenu::SetTitle(const char *Title)
{
  const cFont *font = cFont::GetFont(fontOsd);
  initial = true;
  currentTitle = NULL;
  switch (MenuCategory()) {
     case mcMain:
     case mcSetup:
//     case mcCommand:
     case mcChannel:
        break;
     case mcRecording:
        currentTitle = Title;
     case mcRecordingInfo:
     case mcRecordingEdit:
     case mcCommand:
     case mcTimerEdit:
     case mcSchedule:
     case mcScheduleNow:
     case mcScheduleNext:
     case mcEvent:
        osd->DrawRectangle(xs00 - Gap, ys00, xs00, ys01 - 1, Theme.Color(clrBackground));
        osd->DrawText(xs00, ys00, Title, Theme.Color(clrMenuFrameFg), frameColor, font, xs11 - xs00 - 1, lineHeight, taBottom | taRight);
        osd->DrawRectangle(xs12, ys00, xs13 - 1, ys01 - 1, frameColor);
        break;
     case mcTimer: {
        osd->DrawText(xs00, ys00, Title, Theme.Color(clrMenuFrameFg), frameColor, font, xs11 - xs00, lineHeight, taBottom | taRight | taBorder);
        osd->DrawRectangle(xs12, ys00, xs13 - 1, ys01 - 1, frameColor);
        }
        break;
     default:
        int w = min(font->Width(Title), xa07 - xa06 - Gap);
        osd->DrawRectangle(xa06, yt00, xa07 - w - Gap - 1, yt01 - 1, frameColor);
        osd->DrawText(xa07 - w - Gap, yt00, Title, Theme.Color(clrMenuTitle), Theme.Color(clrBackground), font, w + Gap, yt01 - yt00, taRight);
     }
}

void cLCARSNGDisplayMenu::SetButtons(const char *Red, const char *Green, const char *Yellow, const char *Blue)
{
  const char *lutText[] = { Red, Green, Yellow, Blue };
  tColor lutFg[] = { clrButtonRedFg, clrButtonGreenFg, clrButtonYellowFg, clrButtonBlueFg };
  tColor lutBg[] = { clrButtonRedBg, clrButtonGreenBg, clrButtonYellowBg, clrButtonBlueBg };
  int x = 0;
  int y = 0;
  if (MenuCategory() == mcChannel) {
     x = xa09 - xm05;
     y = yb15 - yc04;
     }
  const cFont *font = cFont::GetFont(fontSml);
  if (MenuCategory() == mcMain || MenuCategory() == mcSetup || MenuCategory() == mcCommand || MenuCategory() == mcChannel || MenuCategory() == mcSchedule || MenuCategory() == mcScheduleNow || MenuCategory() == mcScheduleNext || MenuCategory() == mcEvent || MenuCategory() == mcRecording || MenuCategory() == mcRecordingInfo || MenuCategory() == mcRecordingEdit || MenuCategory() == mcTimer || MenuCategory() == mcTimerEdit) {
     DrawMainButton(lutText[Setup.ColorKey0], xd00 + x, xd01 + x, xd02 + x, xd03 + x, yd02 + y, yd03 + y, Theme.Color(lutFg[Setup.ColorKey0]), Theme.Color(lutBg[Setup.ColorKey0]), font);
     DrawMainButton(lutText[Setup.ColorKey1], xd04 + x, xd05 + x, xd06 + x, xd07 + x, yd02 + y, yd03 + y, Theme.Color(lutFg[Setup.ColorKey1]), Theme.Color(lutBg[Setup.ColorKey1]), font);
     DrawMainButton(lutText[Setup.ColorKey2], xd00 + x, xd01 + x, xd02 + x, xd03 + x, yd04 + y, yd05 + y, Theme.Color(lutFg[Setup.ColorKey2]), Theme.Color(lutBg[Setup.ColorKey2]), font);
     DrawMainButton(lutText[Setup.ColorKey3], xd04 + x, xd05 + x, xd06 + x, xd07 + x, yd04 + y, yd05 + y, Theme.Color(lutFg[Setup.ColorKey3]), Theme.Color(lutBg[Setup.ColorKey3]), font);
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
     osd->SaveRegion(xb00, yb14, xb15 - 1, yb15 - 1);
     osd->DrawText(xb00, yb14, Text, Theme.Color(clrMessageStatusFg + 2 * Type), Theme.Color(clrMessageStatusBg + 2 * Type), cFont::GetFont(fontSml), xb15 - xb00, yb15 - yb14, taCenter);
     }
  else
     osd->RestoreRegion();
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
     ColorBg = Theme.Color(clrBackground);
//     if (currentIndex == Index)
        osd->DrawRectangle(xi00, y, xi03 - 1, y + lineHeight - 1, Theme.Color(clrBackground));
     }
  const cFont *font = cFont::GetFont(fontOsd);
  for (int i = 0; i < MaxTabs; i++) {
      const char *s = GetTabbedText(Text, i);
      if (s) {
         int xt = xi00 + TextSpacing + Tab(i);
         osd->DrawText(xt, y, s, ColorFg, ColorBg, font, xi01 - xt);
         }
      if (!Tab(i + 1))
         break;
      }
  SetEditableWidth(xi02 - xi00 - TextSpacing - Tab(1));
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
  snprintf(t, sizeof(t), "%s  %s - %s", *Event->GetDateString(), *Event->GetTimeString(), *Event->GetEndTimeString());
  ts.Set(osd, xl, y, xi01 - xl, yi01 - y, t, font, Theme.Color(clrEventTime), Theme.Color(clrBackground));
  if (Event->Vps() && Event->Vps() != Event->StartTime()) {
     cString buffer = cString::sprintf(" VPS: %s ", *Event->GetVpsString());
     const cFont *font = cFont::GetFont(fontSml);
     int w = font->Width(buffer);
     osd->DrawText(xi01 - w, y, buffer, Theme.Color(clrMenuFrameFg), frameColor, font, w);
     int yb = y + font->Height();
     osd->DrawRectangle(xi02, y, xi02 + lineHeight / 2 - 1, yb - 1, frameColor);
     osd->DrawEllipse  (xi02 + lineHeight / 2, y, xi03 - 1, yb - 1, frameColor, 5);
     }
  y += ts.Height();
  if (Event->ParentalRating()) {
     cString buffer = cString::sprintf(" %s ", *Event->GetParentalRatingString());
     const cFont *font = cFont::GetFont(fontSml);
     int w = font->Width(buffer);
     osd->DrawText(xi01 - w, y, buffer, Theme.Color(clrMenuFrameFg), frameColor, font, w);
     int yb = y + font->Height();
     osd->DrawRectangle(xi02, y, xi02 + lineHeight / 2 - 1, yb - 1, frameColor);
     osd->DrawEllipse  (xi02 + lineHeight / 2, y, xi03 - 1, yb - 1, frameColor, 5);
     }
  y += font->Height();
  ts.Set(osd, xl, y, xi01 - xl, yi01 - y, Event->Title(), font, Theme.Color(clrEventTitle), Theme.Color(clrBackground));
  y += ts.Height();
  if (!isempty(Event->ShortText())) {
     const cFont *font = cFont::GetFont(fontSml);
     ts.Set(osd, xl, y, xi01 - xl, yi01 - y, Event->ShortText(), font, Theme.Color(clrEventShortText), Theme.Color(clrBackground));
     y += ts.Height();
     }
  y += font->Height();
  if (!isempty(Event->Description())) {
     int yt = y;
     int yb = yi01;
     textScroller.Set(osd, xl, yt, xi01 - xl, yb - yt, Event->Description(), font, Theme.Color(clrEventDescription), Theme.Color(clrBackground));
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
  cString t = cString::sprintf("%s  %s  %s", *DateString(Recording->Start()), *TimeString(Recording->Start()), Info->ChannelName() ? Info->ChannelName() : "");
  ts.Set(osd, xl, y, xi01 - xl, yi01 - y, t, font, Theme.Color(clrEventTime), Theme.Color(clrBackground));
  y += ts.Height();
  if (Info->GetEvent()->ParentalRating()) {
     cString buffer = cString::sprintf(" %s ", *Info->GetEvent()->GetParentalRatingString());
     const cFont *font = cFont::GetFont(fontSml);
     int w = font->Width(buffer);
     osd->DrawText(xi01 - w, y, buffer, Theme.Color(clrMenuFrameFg), frameColor, font, w);
     int yb = y + font->Height();
     osd->DrawRectangle(xi02, y, xi02 + lineHeight / 2 - 1, yb - 1, frameColor);
     osd->DrawEllipse  (xi02 + lineHeight / 2, y, xi03 - 1, yb - 1, frameColor, 5);
     }
  y += font->Height();
  const char *Title = Info->Title();
  if (isempty(Title))
     Title = Recording->Name();
  ts.Set(osd, xl, y, xi01 - xl, yi01 - y, Title, font, Theme.Color(clrEventTitle), Theme.Color(clrBackground));
  y += ts.Height();
  if (!isempty(Info->ShortText())) {
     const cFont *font = cFont::GetFont(fontSml);
     ts.Set(osd, xl, y, xi01 - xl, yi01 - y, Info->ShortText(), font, Theme.Color(clrEventShortText), Theme.Color(clrBackground));
     y += ts.Height();
     }
  y += font->Height();
  if (!isempty(Info->Description())) {
     int yt = y;
     int yb = yi01;
     textScroller.Set(osd, xl, yt, xi01 - xl, yb - yt, Info->Description(), font, Theme.Color(clrEventDescription), Theme.Color(clrBackground));
     DrawTextScrollbar();
     }
}

void cLCARSNGDisplayMenu::SetText(const char *Text, bool FixedFont)
{
  textScroller.Set(osd, xi00, yi00, GetTextAreaWidth(), yi01 - yi00, Text, GetTextAreaFont(FixedFont), Theme.Color(clrMenuText), Theme.Color(clrBackground));
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
  DrawFrameDisplay();
  switch (MenuCategory()) {
     case mcMain:
     case mcSetup:
//     case mcCommand:
//     case mcChannel:
        DrawTimers();
        DrawDevices();
        DrawLiveIndicator();
        DrawSignals();
     case mcCommand:
     case mcSchedule:
     case mcScheduleNow:
     case mcScheduleNext:
     case mcEvent:
     case mcRecording:
     case mcRecordingInfo:
     case mcRecordingEdit:
     case mcTimer:
     case mcTimerEdit:
        if (!Device->Replaying() || Device->Transferring()) {
#if APIVERSNUM > 20300
           LOCK_CHANNELS_READ;
           const cChannel *Channel = Channels->GetByNumber(cDevice::PrimaryDevice()->CurrentChannel());
#else
           const cChannel *Channel = Channels.GetByNumber(cDevice::PrimaryDevice()->CurrentChannel());
#endif
           DrawLive(Channel);
           }
        else if (cControl *Control = cControl::Control(true))
           DrawPlay(Control);
        if (initial) {
           availableRect = cDevice::PrimaryDevice()->CanScaleVideo(videoWindowRect);
           osd->Flush();
           cDevice::PrimaryDevice()->ScaleVideo(availableRect);
           }
        else
           osd->Flush();
        break;
     default:
        availableRect = cDevice::PrimaryDevice()->CanScaleVideo(cRect::Null);
        osd->Flush();
        cDevice::PrimaryDevice()->ScaleVideo(availableRect);
     }
  initial = false;
}
