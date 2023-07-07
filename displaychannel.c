#include <vdr/plugin.h>
#ifdef USE_WAREAGLEICON
#include <vdr/iconpatch.h>
#endif
#include "config.h"
#include "lcarsng.h"
#include "displaychannel.h"
#include "status.h"

cBitmap cLCARSNGDisplayChannel::bmTeletext(teletext_xpm);
cBitmap cLCARSNGDisplayChannel::bmRadio(radio_xpm);
cBitmap cLCARSNGDisplayChannel::bmAudio(audio_xpm);
cBitmap cLCARSNGDisplayChannel::bmDolbyDigital(dolbydigital_xpm);
cBitmap cLCARSNGDisplayChannel::bmEncrypted(encrypted_xpm);
cBitmap cLCARSNGDisplayChannel::bmRecording(recording_xpm);

// --- cLCARSNGDisplayChannel ----------------------------------------------

cLCARSNGDisplayChannel::cLCARSNGDisplayChannel(bool WithInfo) : cThread("LCARS DisplChan")
{
  tallFont = cFont::CreateFont(Setup.FontOsd, Setup.FontOsdSize * 1.8);
  osdFont = cFont::GetFont(fontOsd);
  lineHeight = osdFont->Height();
  tinyFont = CreateTinyFont(lineHeight);
  smlFont = cFont::GetFont(fontSml);
  smlLineHeight = smlFont->Height();
  textBorder = lineHeight * TEXT_ALIGN_BORDER / 100;
  zoom = 0;
  initial = true;
  present = NULL;
  following = NULL;
  lastSeen = -1;
  lastCurrentPosition = -1;
  lastDeviceNumber = -1;
  lastCamSlot = NULL;
  lastSignalStrength = -1;
  lastSignalQuality = -1;
  lastSignalDisplay = 0;
  memset(&lastTrackId, 0, sizeof(lastTrackId));
  withInfo = WithInfo;
  frameColorBg = Theme.Color(clrChannelFrameBg);
  frameColorFg = Theme.Color(clrChannelFrameFg);
  frameColorBr = (Theme.Color(clrChannelFrameBr) == CLR_BLACK) ? frameColorBg : Theme.Color(clrChannelFrameBr);
  textColorBg  = Theme.Color(clrChannelTextBg);
  iconHeight = bmTeletext.Height();
  message = false;
  lastOn = false;
  On = false;
  Margin = Config.Margin;
  lastVolume = statusMonitor->GetVolume();
  lastVolumeTime = time(NULL);
  oldResolution = "";

#ifdef USE_ZAPCOCKPIT
  oldZapcockpitUseInfo = Setup.ZapcockpitUseInfo;
  if (Config.displInfoChannel < 2)
     Setup.ZapcockpitUseInfo = 0;
  else
     Setup.ZapcockpitUseInfo = 1;
#endif // USE_ZAPCOCKPIT

  float hrel = cOsd::OsdHeight() / lineHeight;
  if (hrel < 18) {
     dsyslog ("skinlcarsng: Zoomfactor = 3");
     zoom = 3;
     }
  else if (hrel < 20) {
     dsyslog ("skinlcarsng: Zoomfactor = 2");
     zoom = 2;
     }
  else if (hrel < 25) {
     dsyslog ("skinlcarsng: Zoomfactor = 1");
     zoom = 1;
     }

  // OSD 
  xo00 = 0;
  xo01 = cOsd::OsdWidth();
  yo00 = 0;
  yo01 = cOsd::OsdHeight();

  int d = 5 * lineHeight;
  int d1 = 3 * lineHeight;

  xc00 = xo00;                                // OSD left
  xc01 = xc00 + 2 * lineHeight;
  xc02 = xc00 + d;
  xc03 = xc02 + lineHeight;
  xc04 = xc03 + Gap;
  xc05 = xc02 + ((zoom > 1) ? 0.5 : 1) * d;
  xc06 = xc05 + Gap;
  xc24 = xo01;                                // OSD right
  xc23 = xc24 - lineHeight - 2 * Margin;
  xc22 = xc23 - Gap;
  xc21 = xc22 - ((zoom > 1) ? 1.5 : 2) * d;
  xc20 = xc21 - Gap;
  const cFont *font = (zoom) ? smlFont : osdFont;
  int w = font->Width("HD720p") + 2 * textBorder + 2 * Gap;
  xc19 = xc22 - 2 * Margin - (w + 5 * (bmTeletext.Width() + Gap));
  xc18 = xc19 - Gap;
  xc11 = (xc24 + xc00) / 2;
  xc10 = xc11 - lineHeight - 2 * Margin;
  xc09 = xc10 - Gap;
  xc08 = xc09 - d1;
  xc07 = xc08 - Gap;
  xc12 = xc11 + Gap;
  xc13 = xc12 + lineHeight + 2 * Margin;
  xc14 = xc13 + Gap;
  xc15 = xc14 + d1;
  xc16 = xc15 + Gap;
  xc17 = xc15 + lineHeight;

  int h = max(lineHeight, iconHeight);

  // Calculate the channel info height
  int y00 = 0;                                                                    // Top
  int y01 = y00 + h + 2 * Margin;
  int y02 = y01 + lineHeight / 2;
//  int y03 = y01 + lineHeight;
//  int y04 = y00 + 2 * lineHeight;
  int y05 = y02 + lineHeight + 2 * Margin;
  int y06 = y05 + lineHeight;
  int y07 = y06 + Gap;
  int y08 = y07 + lineHeight + 2 * Margin;
  int y09 = y08 + lineHeight;
  int y10 = y09 + Gap;
//  int y11 = y10 + lineHeight + 2 * Margin;
//  int y12 = y11 + lineHeight;
  int y16 = y10 + h + (Config.swapTimers ? 2.5 : 1.5)  * lineHeight + 2 * Margin; // Bottom

  // Channel info in the OSD
  yc00 = (Setup.ChannelInfoPos) ? 0 : yo01 - (withInfo ? y16 : y06);              // Top
  yc01 = yc00 + h + 2 * Margin;
  yc02 = yc01 + lineHeight / 2;
  yc03 = yc01 + lineHeight;
  yc04 = yc00 + 2 * lineHeight;
  yc05 = yc02 + lineHeight + 2 * Margin;
  yc06 = yc05 + lineHeight;
  yc07 = yc06 + Gap;
  yc08 = yc07 + lineHeight + 2 * Margin;
  yc09 = yc08 + lineHeight;
  yc10 = yc09 + Gap;
  yc11 = yc10 + lineHeight + 2 * Margin;
  yc12 = yc11 + lineHeight;
  yc16 = yc10 + h + (Config.swapTimers ? 2.5 : 1.5)  * lineHeight + 2 * Margin;   // Bottom
  yc15 = yc16 - h - 2 * Margin;
  yc14 = yc15 - lineHeight;
  yc13 = yc16 - 2 * lineHeight;

  // message and volume box
  xv00 = (withInfo) ? xc06 : xc00;
  xv01 = (withInfo) ? xc20 : xc24;
  yv00 = yc00;
  yv01 = yc01;

  leftIcons = xc22;
  xs = 0;

//  dsyslog ("%s %s %d Lineheight %i\n", __FILE__, __func__,  __LINE__, lineHeight);
//  dsyslog ("%s %s %d Iconheight %i\n", __FILE__, __func__,  __LINE__, iconHeight);
//  dsyslog ("%s %s %d Ellipse AU %ix%i\n", __FILE__, __func__,  __LINE__, (xc01 - 1 - xc00), (yc16 - 1 - yc13));
//  dsyslog ("%s %s %d Ellipse AO %ix%i\n", __FILE__, __func__,  __LINE__, (xc01 - 1 - xc00), (yc04 - 1 - yc00));

  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop(), xc00, yo00, xc24 - 1, yo01 - 1);

  animatedInfo.x0 = (zoom > 1) ? xc14 : (zoom) ? xc15 : xc17; // info window left
  animatedInfo.x1 = xc24;                                     // info window right
  animatedInfo.y0 = lineHeight;
  animatedInfo.y1 = yc00 - Gap;
  animatedInfo.d = d;
  animatedInfo.zoom = zoom;
  animatedInfo.textColorBg = textColorBg;
  animatedInfo.shortTextColorFg = Theme.Color(clrEventShortText);
  animatedInfo.frameColorBr = frameColorBr;
  animatedInfo.frameColorBg = frameColorBg;

  int y1 = withInfo ? yc16 : yc06;
  if (withInfo) {
     // Rectangles:
     osd->DrawRectangle(xc00, yc00, xc24 - 1, y1 - 1, Theme.Color(clrBackground)); // Main background
     DrawRectangleOutline(osd, xc00, yc07, xc02 - 1, yc09 - 1, frameColorBr, frameColorBg, 15); // Left middle
     DrawRectangleOutline(osd, xc06, yc00, xc20 - 1, yc00 + (lineHeight + 2 * Margin - Gap) / 2 - 1, frameColorBr, frameColorBg, 15); // Top middle
     // Upper Elbow:
     DrawRectangleOutline(osd, xc00, yc00, xc02 - 1, yc01 - 1, frameColorBr, frameColorBg, 3);
     DrawRectangleOutline(osd, xc00, yc01, xc02 - 1, yc06 - 1, frameColorBr, frameColorBg, 13);
     DrawRectangleOutline(osd, xc02, yc00, xc05 - 1, yc01 - 1, frameColorBr, frameColorBg, 14);
     osd->DrawRectangle(xc00, yc00, xc01 - 1, yc04 - 1, clrTransparent);
     DrawEllipseOutline(osd, xc00, yc00, xc01 - 1, yc04 - 1, frameColorBr, frameColorBg, 2);
     DrawEllipseOutline(osd, xc02, yc01, xc03 - 1, yc03 - 1, frameColorBr, frameColorBg, -2);
     // Lower Elbow:
     DrawRectangleOutline(osd, xc00, yc10, xc02 - 1, yc15 - 1, frameColorBr, frameColorBg, 7);
     DrawRectangleOutline(osd, xc00, yc15, xc02 - 1, yc16 - 1, frameColorBr, frameColorBg, 8);
     DrawRectangleOutline(osd, xc02, yc15, xc05 - 1, yc16 - 1, frameColorBr, frameColorBg, 14);
     osd->DrawRectangle(xc00, yc13, xc01 - 1, yc16 - 1, clrTransparent);
     DrawEllipseOutline(osd, xc00, yc13, xc01 - 1, yc16 - 1, frameColorBr, frameColorBg, 3);
     DrawEllipseOutline(osd, xc02, yc14, xc03 - 1, yc15 - 1, frameColorBr, frameColorBg, -3);
     // Status area:
     DrawRectangleOutline(osd, xc14, yc15, xc18 - 1, yc16 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xc23, yc15, xc23 + lineHeight / 2 + Margin - 1, yc16 - 1, frameColorBr, frameColorBg, 11);
     osd->DrawRectangle(xc23 + lineHeight / 2 + Margin, yc15 + lineHeight / 2, xc24 - 1, yc16 - 1, clrTransparent);
     DrawEllipseOutline(osd, xc23 + lineHeight / 2 + Margin, yc15, xc24 - 1, yc16 - 1, frameColorBr,frameColorBg,  5);
     // Status area tail middle:
     if (!Config.swapTimers) {
        // Middle left middle
        DrawRectangleOutline(osd, xc11 - lineHeight / 2, yc08 - Margin, xc11 - 1, yc09 - 1, frameColorBr, frameColorBg, 13);
        DrawRectangleOutline(osd, xc10, yc07, xc11 - lineHeight / 2 - 1, yc08 - 1, frameColorBr, frameColorBg, 9);
        osd->DrawRectangle(xc10 + Margin, yc07, xc11 - lineHeight / 2 - 1, yc07 + lineHeight / 2 - 1, Theme.Color(clrBackground));
        DrawEllipseOutline(osd, xc10 + Margin, yc07, xc11 - 1, yc08 - Margin - 1, frameColorBr, frameColorBg, 1);
        DrawEllipseOutline(osd, xc10 + Margin, yc08 - 1, xc11 - lineHeight / 2 - 1, yc08 + lineHeight / 2 - 1, frameColorBr, frameColorBg, -1);
        // Middle left bottom
        DrawRectangleOutline(osd, xc11 - lineHeight / 2, yc10, xc11 - 1, yc15 + Margin - 1, frameColorBr, frameColorBg, 7);
        DrawRectangleOutline(osd, xc10, yc15, xc11 - lineHeight / 2 - 1, yc16 - 1, frameColorBr, frameColorBg, 3);
        osd->DrawRectangle(xc10 + Margin, yc15 + lineHeight / 2, xc11 - lineHeight / 2 - 1, yc16 - 1, Theme.Color(clrBackground));
        DrawEllipseOutline(osd, xc10 + Margin, yc15 + Margin, xc11 - 1, yc16 - 1, frameColorBr, frameColorBg, 4);
        DrawEllipseOutline(osd, xc10, yc15 - lineHeight / 2, xc11 - lineHeight / 2 - 1, yc15, frameColorBr, frameColorBg, -4);
        //Middle right top
        DrawRectangleOutline(osd, xc12, yc02 + lineHeight + Margin, xc12 + lineHeight / 2 - 1, yc06 - 1, frameColorBr, frameColorBg, 13);
        DrawRectangleOutline(osd, xc13 - lineHeight / 2, yc02, xc13 - 1, yc05 - 1, frameColorBr, frameColorBg, 12);
        osd->DrawRectangle(xc13 - lineHeight / 2, yc02, xc13 - Margin - 1, yc02 + lineHeight / 2 - 1, Theme.Color(clrBackground));
        DrawEllipseOutline(osd, xc12, yc02, xc13 - Margin - 1, yc05 - Margin - 1, frameColorBr, frameColorBg, 2);
        DrawEllipseOutline(osd, xc12 + lineHeight / 2, yc05 - 1, xc13 - Margin - 1, yc05 + lineHeight / 2 - 1, frameColorBr, frameColorBg, -2);
        // Middle right middle
        DrawRectangleOutline(osd, xc12, yc07, xc12 + lineHeight / 2 - 1, yc09 - 1, frameColorBr, frameColorBg, 15);
        DrawRectangleOutline(osd, xc13 - lineHeight / 2, yc07, xc13 - 1, yc08 - 1, frameColorBr, frameColorBg, 12);
        osd->DrawRectangle(xc13 - lineHeight / 2, yc07, xc13 - Margin - 1, yc07 + lineHeight / 2 - 1, Theme.Color(clrBackground));
        DrawEllipseOutline(osd, xc12, yc07, xc13 - Margin - 1, yc08 - Margin - 1, frameColorBr, frameColorBg, 2);
        DrawEllipseOutline(osd, xc12 + lineHeight / 2, yc08 - 1, xc13 - Margin - 1, yc08 + lineHeight / 2 - 1, frameColorBr, frameColorBg, -2);
        osd->DrawRectangle(xc12 + Margin, yc07 + Margin, xc12 + lineHeight / 2 - 1 - Margin, yc09 - 1 - Margin, frameColorBg);
        // Middle right bottom
        DrawRectangleOutline(osd, xc12, yc10, xc12 + lineHeight / 2 - 1, yc15 + Margin - 1, frameColorBr, frameColorBg, 7);
        DrawRectangleOutline(osd, xc12 + lineHeight / 2, yc15, xc13 - 1, yc16 - 1, frameColorBr, frameColorBg, 6);
        osd->DrawRectangle(xc12 + lineHeight / 2, yc15 + lineHeight / 2, xc13 - Margin - 1, yc16 - 1, Theme.Color(clrBackground));
        DrawEllipseOutline(osd, xc12, yc15 + Margin, xc13 - Margin - 1, yc16 - 1, frameColorBr, frameColorBg, 3);
        DrawEllipseOutline(osd, xc12 + lineHeight / 2, yc15 - lineHeight / 2, xc13 - Margin - 1, yc15 - 1, frameColorBr, frameColorBg, -3);
        // Middle
        DrawRectangleOutline(osd, xc14, yc02, xc15 - 1, yc05 - 1, frameColorBr, frameColorBg, 15); // "Event time 1"
        DrawRectangleOutline(osd, xc14, yc07, xc15 - 1, yc08 - 1, frameColorBr, frameColorBg, 15); // "Event time 2"
        DrawRectangleOutline(osd, xc08, yc07, xc09 - 1, yc08 - 1, frameColorBr, frameColorBg, 15); // "Timer"
        osd->DrawText(xc08 + Margin, yc07 + Margin, "Timer", frameColorFg, frameColorBg, osdFont, xc09 - xc08 - 1 - 2 * Margin, yc08 - yc07 - 1 - 2 * Margin, taLeft | taBorder);
        // Top Right:
        DrawRectangleOutline(osd, xc23, yc00, xc23 + lineHeight / 2 + Margin - 1, yc01 - 1, frameColorBr, frameColorBg, 11);
        osd->DrawRectangle(xc23 + lineHeight / 2 + Margin, yc00, xc24 - 1, yc00 + lineHeight / 2 - 1, clrTransparent);
        DrawEllipseOutline(osd, xc23 + lineHeight / 2 + Margin, yc00, xc24 - 1, yc01 - 1, frameColorBr, frameColorBg, 5);
        }
     else {
        // Middle right top, Timer title
        int ytt1 = yc02 + lineHeight / 3;
        int ytt2 = yc05 + lineHeight / 3;
        DrawEllipseOutline(osd, xc12, ytt1, xc13 - lineHeight / 2 - 1, ytt2 - 1, frameColorBr, frameColorBg, 7);
        DrawRectangleOutline(osd, xc12 + lineHeight / 2, ytt1, xc13 - 1, ytt2 - 1, frameColorBr, frameColorBg, 14);
        DrawRectangleOutline(osd, xc14, ytt1, xc15 - 1, ytt2 - 1, frameColorBr, frameColorBg, 15);
        DrawRectangleOutline(osd, xc15 + Gap, ytt1, xc22 - 1, ytt2 - 1, frameColorBr, frameColorBg, 15);
        DrawRectangleOutline(osd, xc23, ytt1, xc23 + lineHeight / 2 - 1, ytt2 - 1, frameColorBr, frameColorBg, 11);
        DrawEllipseOutline(osd, xc23 + lineHeight / 2, ytt1, xc24 - 1, ytt2 - 1, frameColorBr, frameColorBg, 5);
        osd->DrawText(xc14 + Margin, ytt1 + Margin, "Timer", frameColorFg, frameColorBg, osdFont, xc15 - xc14 - 1 - 2 * Margin, ytt2 - ytt1 - 1 - 2 * Margin, taRight | taBorder);
        // Bottom middle
        DrawRectangleOutline(osd, xc10, yc15, xc13 - 1, yc16 - 1, frameColorBr, frameColorBg, 15);
        // Top Right:
        DrawRectangleOutline(osd, xc23, yc00, xc23 + lineHeight / 2 + Margin - 1, yc01 - 1, Theme.Color(clrDateBg), Theme.Color(clrDateBg), 11);
        osd->DrawRectangle(xc23 + lineHeight / 2 + Margin, yc00, xc24 - 1, yc00 + lineHeight / 2 - 1, clrTransparent);
        DrawEllipseOutline(osd, xc23 + lineHeight / 2 + Margin, yc00, xc24 - 1, yc01 - 1, Theme.Color(clrDateBg), Theme.Color(clrDateBg), 5);
        }
     }
  else {
     // Rectangles:
     osd->DrawRectangle(xc00, yc02, xc24 - 1, y1 - 1, Theme.Color(clrBackground));
     DrawRectangleOutline(osd, xc00, yc02, xc02 - 1, yc06 - 1, frameColorBr, frameColorBg, 15);
     }
}

cLCARSNGDisplayChannel::~cLCARSNGDisplayChannel()
{
  Cancel(3);
  delete drawDescription;
  delete messageBox;
  delete volumeBox;
  delete tallFont;
  delete tinyFont;
  delete osd;
#ifdef USE_ZAPCOCKPIT
  Setup.ZapcockpitUseInfo = oldZapcockpitUseInfo;
#endif // USE_ZAPCOCKPIT
}

void cLCARSNGDisplayChannel::DrawDate(void)
{
  cString s = DayDateTime();
  if (initial || !*lastDate || strcmp(s, lastDate)) {
     osd->DrawText(xc21, yc00, s, Theme.Color(clrDateFg), Theme.Color(clrDateBg), osdFont, xc22 - xc21 - 1, yc01 - yc00, taCenter | taRight | taBorder);
     lastDate = s;
     }
}

void cLCARSNGDisplayChannel::DrawTrack(void)
{
  cDevice *Device = cDevice::PrimaryDevice();
  const tTrackId *Track = Device->GetTrack(Device->GetCurrentAudioTrack());
  if (Track ? strcmp(lastTrackId.description, Track->description) : *lastTrackId.description) {
     osd->DrawText(xc14 + Margin, yc15 + Margin, Track ? Track->description : "", Theme.Color(clrTrackName), frameColorBg, (zoom) ? smlFont : osdFont, xc18 - xc14 - 1 - textBorder - 2 * Margin, yc16 - yc15 - 2 * Margin, taCenter | taRight | taBorder);
     strn0cpy(lastTrackId.description, Track ? Track->description : "", sizeof(lastTrackId.description));
     }
}

int cLCARSNGDisplayChannel::GetLiveBuffer(void) {
  static cPlugin *pPermashift = cPluginManager::GetPlugin("permashift");
  if (pPermashift) {
     int buffer = 0;
     if (pPermashift->Service("Permashift-GetUsedBufferSecs-v1", &buffer)) {
        return buffer;
     }
  }
  return -1;
}

void cLCARSNGDisplayChannel::DrawSeen(int Current, int Total)
{
  if (lastCurrentPosition >= 0)
     return; // to not interfere with SetPositioner()

  int Seen = (Total > 0) ? min(xc20 - xc06, int((xc20 - xc06) * double(Current) / Total)) : 0;
  if (initial || Seen != lastSeen) {
     int x = (Config.swapTimers) ? 0 : 2;
     int y0 = yc00 + (lineHeight + 2 * Margin) / 2;
     int y1 = y0 + lineHeight / 3;
     // progress bar
     if (Config.swapTimers) { // Don't draw the background border, but clear the background
        osd->DrawRectangle(xc06, y0, xc20 - 1, y1 - 1, Theme.Color(clrBackground));
        osd->DrawRectangle(xc06, y0, std::min(xc06 + Seen, xc20 - 1), y1 - 1, Theme.Color(clrSeen));
        }
     else {
        osd->DrawRectangle(xc06, y0, xc20 - 1, y1 - 1, Theme.Color(clrSeen));
        osd->DrawRectangle(xc06 + x + Seen, y0 + 2, xc20 - 1 - x, y1 - 3, frameColorBg);
        }
     // timeshift buffer
     int Buffer = GetLiveBuffer();
     if (Buffer > 0) {
        int Timeshift = std::max(0, int((xc20 - xc06) * double(Current - Buffer) / Total));
        int x1 = std::min(xc20 - 1 - x, xc06 + x + Seen);
        osd->DrawRectangle(xc06 + x + Timeshift, y0 + 2, x1, y1 - 3, Theme.Color(clrChannelSymbolRecBg));
        }
     // display time remaining
     cString time = ((Current / 60.0) > 0.1) ? cString::sprintf("-%d", max((int)ceil((Total - Current) / 60.0), 0)) : "";
     if (Config.swapTimers)
        osd->DrawRectangle(xc01 + Margin, yc08, xc02 - Margin - 1, yc09 - 1 - Margin, frameColorBg); //Backgroung time remaining
     else
        osd->DrawRectangle(xc14 + Margin, yc05, xc15 - Margin - 1, yc06 - 1, Theme.Color(clrBackground)); // Background time remaining
     if (!isempty(time)) {
        int w = smlFont->Width(time) + 2 * textBorder;
        int x1 = Config.swapTimers ? xc01 : xc14;
        int x2 = Config.swapTimers ? xc02 : xc15;
        int y2 = Config.swapTimers ? yc08 : yc05;
        tColor background = Config.swapTimers ? frameColorBg : textColorBg;
        osd->DrawText(x1 + Margin + (x2 - x1 - 1 - 2 * Margin - w), y2 + Gap / 2, time, Theme.Color(clrEventShortText), background, smlFont, w, smlLineHeight, taRight | taBorder); // time remaining
        }
     lastSeen = Seen;
     }
}

void cLCARSNGDisplayChannel::DrawDevice(void)
{
  const cDevice *Device = cDevice::ActualDevice();
  if (DrawDeviceData(osd, Device, xc06, yc15, xc09, yc16, xs, tinyFont, lastDeviceType, lastCamSlot, Device->DeviceNumber() != lastDeviceNumber)) {
     lastDeviceNumber = Device->DeviceNumber();
     // Make sure signal meters are redrawn:
     lastSignalStrength = -1;
     lastSignalQuality = -1;
     lastSignalDisplay = 0;
     }
}

void cLCARSNGDisplayChannel::DrawSignal(void)
{
  time_t Now = time(NULL);
  if (Now != lastSignalDisplay) {
     DrawDeviceSignal(osd, cDevice::ActualDevice(), xs + lineHeight / 2, yc15, xc09, yc16, lastSignalStrength, lastSignalQuality, initial);
     lastSignalDisplay = Now;
     }
}

void cLCARSNGDisplayChannel::DrawScreenResolution(void)
{
  cString resolution = GetScreenResolutionIcon();
  if (!(strcmp(resolution, oldResolution) == 0)) {
     if (strcmp(resolution, "") == 0) {
        osd->DrawRectangle(xc19 + Margin, yc15 + Margin, leftIcons, yc16 - Margin, frameColorBg);
        }
     const cFont *font = (zoom) ? smlFont : osdFont;
     int w = font->Width(*resolution) + 2 * textBorder;
     int x = leftIcons - w - SymbolSpacing;
     int h = ((zoom) && (iconHeight < lineHeight)) ? (lineHeight - iconHeight) / 2 : 0;
     osd->DrawText(x, yc15 + h + Margin, cString::sprintf("%s", *resolution), Theme.Color(clrChannelSymbolOn), frameColorBr, font, w, yc16 - yc15 - 2 * Margin - 2 * h, taCenter | taRight | taBorder);
     oldResolution = resolution;
     }
}

void cLCARSNGDisplayChannel::DrawBlinkingRec(void)
{
  if (message)
     return;

  bool rec = cRecordControls::Active();

  if (rec) {
     if (!Running()) {
        Start();
        On = true;
        }
     }
  else {
     On = false;
     }
  if (initial || On != lastOn) {
     int x = xc22;
     x -= bmRecording.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc15 + (yc16 - yc15 - bmRecording.Height()) / 2, bmRecording, Theme.Color(rec ? On ? clrChannelSymbolRecFg : clrChannelSymbolOff : clrChannelSymbolOff), rec ? On ? Theme.Color(clrChannelSymbolRecBg) : frameColorBr : frameColorBr);
     lastOn = On;
     }
}
 
void cLCARSNGDisplayChannel::DrawEventRec(const cEvent *Present, const cEvent *Following)
{
  for (int i = 0; i < 2; i++) {
      const cEvent *e = !i ? Present : Following;
      tColor recColor = !i ? Theme.Color(clrChannelSymbolRecBg) : Theme.Color(clrButtonYellowBg);
      int y0 = !i ? (Config.swapTimers ? yc07 : yc02) : (Config.swapTimers ? yc10 : yc07);
      int y1 = !i ? (Config.swapTimers ? yc08 : yc05) : (Config.swapTimers ? yc11 : yc08);
      int x0 = Config.swapTimers ? xc02 : xc15;
      int x1 = Config.swapTimers ? xc03 : xc17;
      if (e) {
         LOCK_TIMERS_READ;
         eTimerMatch TimerMatch = tmNone;
         const cTimer *Timer = Timers->GetMatch(e, &TimerMatch);
         if (Timer && Timer->HasFlags(tfActive) && TimerMatch == tmFull)
            osd->DrawEllipse(x0 + Gap, y0 + Margin + Gap, x1 - Gap - 1, y1 - Margin - Gap - 1, recColor, 0);
         else
            osd->DrawRectangle(x0, y0, x1 - 1, y1 - 1, Theme.Color(clrBackground));
         }
      }
}

void cLCARSNGDisplayChannel::DrawTimer(void)
{
  int CountTimers = 0;
  LOCK_TIMERS_READ;
  for (const cTimer *Timer = Timers->First(); Timer; Timer = Timers->Next(Timer)) {
     if (Timer->HasFlags(tfActive))
        CountTimers++;
     }
  if (CountTimers == 0)
     return;

  cSortedTimers SortedTimers(Timers);
  int x0 = (Config.swapTimers) ? xc16 : xc04;
  int x1 = x0 + smlFont->Width("Mo. 00.00. 00:00") + 2 * textBorder;
  int x2 = x1 + Gap;
  int x3 = (Config.swapTimers) ? xc22 : xc07;
  int i = 0;
  int j = 0;
  int yrel = (lineHeight - smlLineHeight) / 2;
  int maxTimers = (Config.swapTimers ? 4 : 3);
  cString rec = (Config.swapTimers) ? "" : "Rec: ";

  while (i < min(CountTimers, maxTimers)) {
     int y = 0;
     if (Config.swapTimers)
        y = yc05 + lineHeight / 3 + Gap + yrel + i * (lineHeight + Gap) + Margin;
     else
        y = (i == 0) ? yc07 + Margin + yrel
                     : (i == 1) ? yc09 - Margin - smlLineHeight - yrel
                                : yc10 + Margin;
     if (const cTimer *Timer = SortedTimers[i + j]) {
        time_t Now = time(NULL);
        if (!(Timer->HasFlags(tfActive)) || (Timer->StopTime() < Now))
           j++;
        else {
           cString Date;
           bool isRemote = false;
           bool isRecording = false;
#if APIVERSNUM > 20300
           // The remote timer indicator:
           if (Timer->Remote())
              isRemote = true;
#endif
           // The timer recording indicator:
           if (Timer->Recording()) {
              isRecording = true;
              Date = cString::sprintf("- %s", *TimeString(Timer->StopTime()));
              }
           else
              Date = DayDateTime(Timer->StartTime());
           const cChannel *Channel = Timer->Channel();
           const cEvent *Event = Timer->Event();
           if (Channel && Event) {
              const char *File = Event->Title();
              if (isRecording) {
                 const char *recName  = Timer->File();
                 if (recName && *recName == '@')
                    File = recName;
                 }
              tColor timerColor = Theme.Color(clrEventShortText);
#ifdef SWITCHONLYPATCH
              if (Timer->HasFlags(tfSwitchOnly)) timerColor = Theme.Color(clrSwitchTimer);
#endif
              if (Config.swapTimers) { // color the timer channel number button
                 tColor recColorBg = frameColorBg;
                 tColor recColorBr = frameColorBr;
                 if (isRecording) {
                    recColorBg = Theme.Color(clrChannelSymbolRecBg);
                    recColorBr = Theme.Color(clrChannelSymbolRecBg);
                 }
                 DrawEllipseOutline(osd, xc12, y - yrel, xc12 + smlLineHeight / 2 - 1, y + smlLineHeight + yrel - 1, recColorBr, recColorBg, 7);
                 DrawRectangleOutline(osd, xc12 + smlLineHeight / 2, y - yrel, xc13 - 1,  y + smlLineHeight + yrel - 1, recColorBr, recColorBg, 14);
                 DrawRectangleOutline(osd, xc14, y - yrel, xc15 - 1,  y + smlLineHeight + yrel - 1, recColorBr, recColorBg, 15);
                 osd->DrawText(xc14 + Gap, y, cString::sprintf("%d", Channel->Number()), frameColorFg, recColorBg, smlFont, xc15 - xc14 - 2 * Gap - 1, smlLineHeight, taRight | taBorder);
                 }
              else {
                 osd->DrawText(xc01, y, cString::sprintf("%d", Channel->Number()), frameColorFg, frameColorBg, smlFont, xc02 - xc01 - Gap - 1, smlLineHeight, taRight | taBorder);
                 }

              if (isRecording) {
                 if (isRemote) {
#ifdef USE_WAREAGLEICON
                    const char *icon = Icons::MovingRecording();
                    osd->DrawText(x0, y, cString::sprintf("%s%s %s", *rec, icon, *Date), Theme.Color(clrChannelSymbolRecBg), textColorBg, smlFont, x1 - x0 - 1, smlLineHeight, taRight | taBorder);
#else
                    osd->DrawText(x0, y, cString::sprintf("%s%s", *rec, *Date), Theme.Color(clrChannelSymbolRecBg), textColorBg, smlFont, x1 - x0 - 1, smlLineHeight, taRight | taBorder);
#endif
                    }
                 else {
                    const cDevice *Device = NULL;
                    if (cRecordControl *RecordControl = cRecordControls::GetRecordControl(Timer))
                       Device = RecordControl->Device();
                    cString Number = Device ? itoa(Device->DeviceNumber() + 1) : "?";
                    osd->DrawText(x0, y, cString::sprintf("%s#%s %s", *rec, *Number, *Date), Theme.Color(clrChannelSymbolRecBg), textColorBg, smlFont, x1 - x0 - 1, smlLineHeight, taRight | taBorder);
                    }
                 }
              else
                    osd->DrawText(x0, y, cString::sprintf("%s", *Date), timerColor, textColorBg, smlFont, x1 - x0 - 1, smlLineHeight, taRight | taBorder);

              int w = smlFont->Width(File) + 2 * textBorder; // smlFont width to short
              osd->DrawRectangle(x2, y, x3 - 1, y + smlLineHeight, Theme.Color(clrBackground));
              osd->DrawText(x2, y, cString::sprintf("%s", File), timerColor, textColorBg, smlFont, min(w, x3 - x1 - Gap - 1), smlLineHeight, taLeft | taBorder);
              }
           i++;
           }
        }
     }
}

void cLCARSNGDisplayChannel::SetChannel(const cChannel *Channel, int Number)
{
  DELETENULL(drawDescription);
  int x = xc22;
  if (withInfo) {
     DrawRectangleOutline(osd, xc19, yc15, xc22 - 1, yc16 - 1, frameColorBr, frameColorBg, 15);
     if (Channel && !Channel->GroupSep()) {
        x -= bmRecording.Width() + SymbolSpacing;
        x -= bmEncrypted.Width() + SymbolSpacing;
        osd->DrawBitmap(x, yc15 + (yc16 - yc15 - bmEncrypted.Height()) / 2, bmEncrypted, Theme.Color(Channel->Ca() ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
        x -= bmDolbyDigital.Width() + SymbolSpacing;
        osd->DrawBitmap(x, yc15 + (yc16 - yc15 - bmDolbyDigital.Height()) / 2, bmDolbyDigital, Theme.Color(Channel->Dpid(0) ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
        x -= bmAudio.Width() + SymbolSpacing;
        osd->DrawBitmap(x, yc15 + (yc16 - yc15 - bmAudio.Height()) / 2, bmAudio, Theme.Color(Channel->Apid(1) ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
        if (Channel->Vpid()) {
           x -= bmTeletext.Width() + SymbolSpacing;
           osd->DrawBitmap(x, yc15 + (yc16 - yc15 - bmTeletext.Height()) / 2, bmTeletext, Theme.Color(Channel->Tpid() ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
           }
        else if (Channel->Apid(0)) {
           x -= bmRadio.Width() + SymbolSpacing;
           osd->DrawBitmap(x, yc15 + (yc16 - yc15 - bmRadio.Height()) / 2, bmRadio, Theme.Color(clrChannelSymbolOn), frameColorBr);
           }
        initial = true; // make shure DrawBlinkingRec() refreshs recording icon
        }
     }
  leftIcons = x;

  cString ChNumber("");
  cString ChName("");
  if (Channel) {
     ChName = Channel->Name();
     if (!Channel->GroupSep())
        ChNumber = cString::sprintf("%d%s", Channel->Number(), Number ? "-" : "");
     }
  else if (Number)
     ChNumber = cString::sprintf("%d-", Number);
  else
     ChName = ChannelString(NULL, 0);

  int xR = (withInfo) ? xc09 : xc22;
  int w = tallFont->Width(ChName);
  // must be (xc00 + 3 * Margin) to not interact with the ellipse
  osd->DrawText(xc00 + Gap + Margin, yc02 + Margin, ChNumber, frameColorFg, frameColorBg, tallFont, xc02 - xc00 - Gap - 2 * Margin, yc06 - yc02 - 2 * Margin, taCenter | taRight | taBorder);
  osd->DrawRectangle(xc03, yc02, xR - 1, yc06 - 1, Theme.Color(clrBackground));
  osd->DrawText(xc03, yc02 + Margin, ChName, Theme.Color(clrChannelName), textColorBg, tallFont, min(w, xR - xc03 - 1), yc06 - yc02 - 2 * Margin, taCenter | taLeft);
  lastSignalDisplay = 0;

  if (withInfo) {
     if (Channel) {
        osd->DrawText(xc02, yc15 + Margin, cSource::ToString(Channel->Source()), frameColorFg, frameColorBg, osdFont, xc05 - xc02 - Gap, yc16 - yc15 - 2 * Margin, taRight | taBorder);
        }
     DrawDevice();
     }
}

void cLCARSNGDisplayChannel::SetEvents(const cEvent *Present, const cEvent *Following)
{
  DELETENULL(drawDescription);
  if (!withInfo)
     return;

  if (present != Present)
     lastSeen = -1;
  present = Present;
  following = Following;
  for (int i = 0; i < 2; i++) {
      const cEvent *e = !i ? Present : Following;
      int x1 = Config.swapTimers ? xc03 : xc17; // text left
      int x2 = Config.swapTimers ? xc09 : xc22; // text right
      int x3 = Config.swapTimers ? xc01 : xc14; // time left
      int x4 = Config.swapTimers ? xc02 : xc15; // time right
      int y0 = !i ? (Config.swapTimers ? yc07 : yc02) : (Config.swapTimers ? yc10 : yc07);
      int y1 = !i ? (Config.swapTimers ? yc08 : yc05) : (Config.swapTimers ? yc11 : yc08);
      int y2 = !i ? (Config.swapTimers ? yc09 : yc06) : (Config.swapTimers ? yc12 : yc09);
      if (e) {
         osd->DrawRectangle(x1, y0, x2 - 1, y2 - 1, Theme.Color(clrBackground));
//       draw Time:
         osd->DrawText(x3 + Margin, y0  + Margin, e->GetTimeString(), frameColorFg, frameColorBg, osdFont, x4 - x3 - 1 - 2 * Margin, y1 - y0 - 1 - 2 * Margin, taRight | taBorder);
//       draw Title:
         if (!isempty(e->Title())) {
            int w = osdFont->Width(e->Title()) + 2 * textBorder;
            osd->DrawText(x1, y0 + Margin, e->Title(), Theme.Color(clrEventTitle), textColorBg, osdFont, min(w, x2 - x1), y1 - y0 - 1 - 2 * Margin, taBorder);
            }
//       draw ShortText:
         if (!isempty(e->ShortText())) {
            int w = smlFont->Width(e->ShortText()) + 2 * textBorder;
            osd->DrawText(x1, y1 + Gap / 2, e->ShortText(), Theme.Color(clrEventShortText), textColorBg, smlFont, min(w, x2 - x1), smlLineHeight, taBorder);
            }
         }
      else {
         osd->DrawRectangle(x3 + Margin, y0 + Margin, x4 - 1 - Margin, y1 - 1 - Margin, frameColorBg);
         osd->DrawRectangle(x1, y0, x2 - 1, y2 - 1, Theme.Color(clrBackground));
         }
      }
   if (lastSeen == -1)
      animatedInfo.Event = Present;
}

void cLCARSNGDisplayChannel::SetMessage(eMessageType Type, const char *Text)
{
  if (Text) {
     DELETENULL(drawDescription);
     DELETENULL(volumeBox);
     message = true;
     if (!messageBox)
        messageBox = new cLCARSNGMessageBox(osd, cRect(xv00, yv00, xv01 - xv00, yv01 - yv00), !withInfo);
     messageBox->SetMessage(Type, Text);
     }
  else {
     DELETENULL(messageBox);
     message = false;
     }
}

void cLCARSNGDisplayChannel::DrawVolume(void)
{
   if (!message && withInfo) {
      int volume = statusMonitor->GetVolume();
      if (volume != lastVolume) {
         if (!volumeBox)
            volumeBox = new cLCARSNGVolumeBox(osd, cRect(xv00, yv00, xv01 - xv00, yv01 - yv00), !withInfo);
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

#if APIVERSNUM > 20101
void cLCARSNGDisplayChannel::SetPositioner(const cPositioner *Positioner)
{
  if (Positioner) {
     int y0 = yc01 - (yc01 - yc00) / 2 + Gap / 2;
     int y1 = yc01 + ShowSeenExtent;
     DrawDevicePosition(osd, Positioner, xc06, y0, xc20, y1, lastCurrentPosition);
     }
  else {
     lastCurrentPosition = -1;
     initial = true; // make shure DrawSeen() refreshs progress bar
     }
  return;
}
#endif

void cLCARSNGDisplayChannel::SetInfo(bool showInfo) {
  if (!Config.displInfoChannel || Setup.ChannelInfoPos)
     return;

  if (showInfo && !message && !drawDescription) {
     drawDescription = new cDrawChannelDescription(osd, animatedInfo);
     Start();
  } else {
     DELETENULL(drawDescription);
  }
}

#ifdef USE_ZAPCOCKPIT
void cLCARSNGDisplayChannel::SetViewType(eDisplaychannelView ViewType) {
  viewTypeLast = this->viewType;
  this->viewType = viewType;
}

int cLCARSNGDisplayChannel::MaxItems(void) {
/*  initList = true;
  if (viewType == dcChannelList && channelList)
     return channelList->NumItems();
  else if (viewType == dcGroupsList && groupList)
     return groupList->NumItems();
  else if (viewType == dcGroupsChannelList && groupChannelList)
     return groupChannelList->NumItems();*/
  return 0;
}

bool cLCARSNGDisplayChannel::KeyRightOpensChannellist(void) {
//  return view->KeyRightOpensChannellist();
  return true;
}

void cLCARSNGDisplayChannel::SetChannelInfo(const cChannel *Channel) {
  if (!(Config.displInfoChannel == 2))
     return;

  SetInfo(true);
}

void cLCARSNGDisplayChannel::SetChannelList(const cChannel *Channel, int Index, bool Current) {
/*  displayList = true;
  if (viewType == dcChannelList && channelList) {
     channelList->Set(channel, index, current);
  } else if (viewType == dcGroupsChannelList && groupChannelList) {
     groupChannelList->Set(channel, index, current);
  }*/
}

void cLCARSNGDisplayChannel::SetGroupList(const char *Group, int NumChannels, int Index, bool Current) {
//  view->SetGroupList(Group, NumChannels, Index, Current);
}

void cLCARSNGDisplayChannel::SetGroupChannelList(const cChannel *Channel, int Index, bool Current) {
}

void cLCARSNGDisplayChannel::ClearList(void) {
//  view->ClearList();
}

void cLCARSNGDisplayChannel::SetNumChannelHints(int Num) {
//  view->SetNumChannelHints(Num);
}

void cLCARSNGDisplayChannel::SetChannelHint(const cChannel *Channel) {
//  view->SetChannelHint(Channel);
}

#endif //USE_ZAPCOCKPIT

#ifdef DRAWGRID
void cLCARSNGDisplayChannel::DrawGrid(void)
{
  int bottom = yc16;
  int top = yc00;
  tColor gridColor = 0xffff7700;
  int offset = 0; // lineHeight;

// int xc00, xc01, xc02, xc03, xc04, xc05, xc06, xc07, xc08, xc09, xc10, xc11, xc12, xc13, xc14, xc15, xc17, xc18, xc19, xc20, xc21, xc22, xc23, xc24;
  int xc[27] = { xs, leftIcons, xc00, xc01, xc02, xc03, xc04, xc05, xc06,
                 xc07, xc08, xc09, xc10, xc11, xc12, xc13, xc14, xc15, xc16, xc17,
                 xc18, xc19, xc20, xc21, xc22, xc23, xc24};
  char strxc[28][6] = { "xs", "lIcon", "xc00", "xc01", "xc02", "xc03", "xc04", "xc05", "xc06",
                       "xc07", "xc08", "xc09", "xc10", "xc11", "xc12", "xc13", "xc14", "xc15", "xc16", "xc17",
                       "xc18", "xc19", "xc20", "xc21", "xc22", "xc23", "xc24", '\0'};

  for (int i = 0; strxc[i][0]; i++) {
     osd->DrawRectangle(xc[i], top, xc[i] + 1, bottom - 1, gridColor);
     osd->DrawText(xc[i], top + offset, cString(strxc[i]), gridColor, clrTransparent, tinyFont);

     offset = offset + lineHeight;

     if ((i % 3) == 0)
        offset = lineHeight;
  }

  offset = lineHeight;

// int yc00, yc01, yc02, yc03, yc04, yc05, yc06, yc07, yc08, yc09, yc10, yc13, yc14, yc15, yc16;
  int yc[17] = { yc00, yc01, yc02, yc03, yc04, yc05, yc06, yc07, yc08, yc09, yc10, yc11, yc12, yc13, yc14, yc15, yc16};
  char stryc[18][6] = { "yc00", "yc01", "yc02", "yc03", "yc04", "yc05", "yc06", "yc07", "yc08", "yc09", "yc10", "yc11", "yc12", "yc13", "yc14", "yc15", "yc16", '\0'};

  for (int i = 0; stryc[i][0]; i++) {
     osd->DrawRectangle(xc00, yc[i], xc24 - 1, yc[i] + 1, gridColor);
     osd->DrawText(xs + 1.5 * offset, yc[i], cString(stryc[i]), gridColor, clrTransparent, tinyFont);

     offset = offset + lineHeight;

     if ((i % 3) == 0)
        offset = lineHeight;
  }
}
#endif

void cLCARSNGDisplayChannel::Flush(void)
{
  if (withInfo) {
     if (!message) {
        DrawDate();
        DrawDevice();
        DrawSignal();
        DrawTimer();
        int Current = 0;
        int Total = 0;
        if (present) {
           time_t t = time(NULL);
           if (t > present->StartTime())
              Current = t - present->StartTime();
           Total = present->Duration();
           }
        DrawSeen(Current, Total);
        DrawTrack();
        DrawScreenResolution();
        DrawEventRec(present, following);
        DrawBlinkingRec();
        }
     }
  DrawVolume();
#ifdef DRAWGRID
  if (Config.displayGrid)
     DrawGrid();
#endif
  if (initial || !(Running()))
     osd->Flush();
  if (Config.displInfoChannel == 1 && !drawDescription)
     SetInfo(true);
  initial = false;
}

void cLCARSNGDisplayChannel::Action(void)
{
  int FrameTime = (int)(1000 / Config.framesPerSecond);

  uint64_t Start = cTimeMs::Now();

  while (Running()) {
     uint64_t Now = cTimeMs::Now();
     if (message || (int)(Now - Start) > 1000) {
        Start = Now;
        On = !On;
        DrawBlinkingRec();
        }
     if (Running() && drawDescription)
        drawDescription->Animate();
     if (Running())
        osd->Flush();
     if (!cRecordControls::Active() && !drawDescription)
        break;
     int Delta = cTimeMs::Now() - Now;
     if (Running() && (Delta < FrameTime))
        cCondWait::SleepMs(FrameTime - Delta);
     }
}

// --- cDrawDescription ----------------------------------------------------

cDrawChannelDescription::cDrawChannelDescription(cOsd *osd, AnimatedChannelInfo_t animatedInfo)
{
  this->osd = osd;
  aI = animatedInfo;
  Margin = Config.Margin;

  Draw();
}

cDrawChannelDescription::~cDrawChannelDescription()
{
  if (TextPixmap) TextPixmap->SetAlpha(0);
  if (BracketPixmap) BracketPixmap->SetAlpha(0);
  osd->Flush();
  osd->DestroyPixmap(TextPixmap);
  osd->DestroyPixmap(BracketPixmap);
}

void cDrawChannelDescription::DrawBracket(int height)
{
  int lineHeight = cFont::GetFont(fontOsd)->Height();
  int xy = lineHeight + 2 * Margin;
  int yh = (xy - Gap) / 2; // lineHeight / 2 + 2 * Margin;
  int x0 = 0;              // rectangle left
  int x6 = aI.x1 - aI.x0;  // rectangle right
  int x1 = x0 + xy;
  int x5 = x6 - xy;
  int x4 = x5 - Gap;
  int x3 = x4 - ((aI.zoom > 1) ? 1.5 : 2) * aI.d;
  int x2 = x3 - Gap; 
  int y0 = 0;              // rectangle top
  int y1 = height;         // rectangle bottom

  BracketPixmap = osd->CreatePixmap(2, cRect(aI.x0, aI.y1 - height, aI.x1 - aI.x0, height));
  if (!BracketPixmap) 
     return; 
  
  BracketPixmap->SetAlpha(255);

  BracketPixmap->Fill(Theme.Color(clrBackground));
  int rand = xy + Gap;
  DrawRectangle(BracketPixmap, rand, rand, x4, y1 - rand, clrTransparent);

  DrawRectangleOutline(BracketPixmap, x1, 0, x2, yh, aI.frameColorBr, aI.frameColorBg, 14);
  DrawRectangleOutline(BracketPixmap, x3, 0, x4, yh, aI.frameColorBr, aI.frameColorBg, 15);
  DrawRectangleOutline(BracketPixmap, x3, yh + Gap, x4, xy, aI.frameColorBr, aI.frameColorBg, 15);
  DrawRectangleOutline(BracketPixmap, 0, xy, x1, y1 - xy, aI.frameColorBr, aI.frameColorBg, 1);
  DrawRectangleOutline(BracketPixmap, x1, y1 - yh, x2, y1, aI.frameColorBr, aI.frameColorBg, 14);
  DrawRectangleOutline(BracketPixmap, x3, y1 - xy, x4, y1, aI.frameColorBr, aI.frameColorBg, 15);

  // Upper Elbow part 1:
  DrawRectangle(BracketPixmap, 0, 0, xy, xy, clrTransparent);
  DrawEllipseOutline(BracketPixmap, 0, 0, x1, xy, aI.frameColorBr, aI.frameColorBg, 2);
  // Lower Elbow part 1:
  DrawRectangle(BracketPixmap, 0, y1 - xy, xy, y1, clrTransparent);
  DrawEllipseOutline(BracketPixmap, 0, y1 - xy, x1, y1, aI.frameColorBr, aI.frameColorBg, 3);

  DrawRectangle(BracketPixmap, x1 - Margin, yh, x1, y1 - yh, aI.frameColorBr);

  // Upper Elbow part 2:
  DrawEllipseOutline(BracketPixmap, x1, yh, x1 + yh, xy, aI.frameColorBr, aI.frameColorBg, -2);
  // Lower Elbow part 2:
  DrawEllipseOutline(BracketPixmap, x1, y1 - xy, x1 + yh, y1 - yh, aI.frameColorBr, aI.frameColorBg, -3);

  // Top Right
  DrawRectangleOutline(BracketPixmap, x5, 0, x5 + xy / 2, xy, aI.frameColorBr, aI.frameColorBg, 11);
  DrawRectangle(BracketPixmap, x5 + xy / 2, 0, x6, xy / 2, clrTransparent);
  DrawEllipseOutline(BracketPixmap, x5 + xy / 2, 0, x6, xy, aI.frameColorBr, aI.frameColorBg, 5);
  // Bottom Right
  DrawRectangleOutline(BracketPixmap, x5, y1 - xy, x5 + xy / 2, y1, aI.frameColorBr, aI.frameColorBg, 11);
  DrawRectangle(BracketPixmap, x5 + xy / 2, y1 - xy / 2, x6, y1, clrTransparent);
  DrawEllipseOutline(BracketPixmap, x5 + xy / 2, y1 - xy, x6, y1, aI.frameColorBr, aI.frameColorBg, 5);
}

void cDrawChannelDescription::Draw(void)
{
  if (!(aI.Event && !isempty(aI.Event->Description())))
     return;

  const char *s = aI.Event->Description();             // text
  if (!s || isempty(s))
     return;

  const cFont *Font = cFont::GetFont(fontSml);
  const int smlLineHeight = Font->Height(s);

  int lineHeight = cFont::GetFont(fontOsd)->Height();
  int rand = lineHeight + 2 * Margin + Gap;            // Margin + Gap;
  int x0 = aI.x0 + rand;                               // text left
  int x1 = aI.x1 - rand;                               // text right

  int textwidth = x1 - x0;

  wrapper.Set(s, Font, textwidth - 2 * Gap);
  int l0 = wrapper.Lines();                            // textlines
  if (l0 == 0)
     return;

  int l1 = std::min(l0, Config.infoChanLines);         // diplayed lines

  int viewportheight = l1 * smlLineHeight;
  while ((aI.y1 - viewportheight - 2 * rand) <= aI.y0) {
     viewportheight = viewportheight - smlLineHeight;
     }

  DrawBracket(viewportheight + 2 * rand);

  int pixmapwidth = textwidth;
  int drawportheight = l0 * smlLineHeight;
  int y1 = aI.y1 - viewportheight - rand;              // text bottom

  if (TextPixmap = osd->CreatePixmap(3, cRect(x0, y1, pixmapwidth, viewportheight), cRect(0, 0, pixmapwidth, drawportheight))) {
     TextPixmap->Fill(aI.textColorBg);
     int y = 0;
     for (int i = 0; i < l0; i++) {
        TextPixmap->DrawText(cPoint(Gap, y), wrapper.GetLine(i), aI.shortTextColorFg, clrTransparent, Font, textwidth); // textline
        y += smlLineHeight;
        }
     }
  StartTime = cTimeMs::Now();
}

void cDrawChannelDescription::Animate(void)
{
  if (!TextPixmap)
     return;

//  int fadeinDelay = Config.waitTimeFadein;
  int scrollDelay = Config.waitTimeScroll;

  int maxY = std::max(0, TextPixmap->DrawPort().Height() - TextPixmap->ViewPort().Height());
  if (maxY == 0)
     return;

  uint64_t Now = cTimeMs::Now();
  if ((int)(Now - StartTime) < scrollDelay)
     return;

  // Scroll
  cPixmap::Lock();
  int drawPortY = TextPixmap->DrawPort().Y();
  if (std::abs(drawPortY) < maxY)
     drawPortY -= Config.scrollPixel;
  if (std::abs(drawPortY) >= maxY) {
     if (!dowait) {
        StartTime = cTimeMs::Now();
        dowait = true;
        }
     else {
        drawPortY = 0;
        StartTime = cTimeMs::Now();
        dowait = false;
        }
     }
  TextPixmap->SetDrawPortPoint(cPoint(0, drawPortY));
  cPixmap::Unlock();
}
