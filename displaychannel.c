#include <vdr/plugin.h>
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
  textBorder = lineHeight * TEXT_ALIGN_BORDER / 100;
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

  int d = 5 * lineHeight;
  int d1 = 3 * lineHeight;

  xc00 = 0;                                        // Left
  xc01 = xc00 + 2 * lineHeight;
  xc02 = xc00 + d;
  xc03 = xc02 + lineHeight;
  xc04 = xc03 + Gap;
  xc05 = xc02 + d;
  xc06 = xc05 + Gap;
  xc23 = cOsd::OsdWidth();                         // Bottom
  xc22 = xc23 - lineHeight;
  xc21 = xc22 - Gap;
  xc20 = xc21 - 2 * d;
  xc19 = xc20 - Gap;
  xc18 = xc19 - 2.5 * lineHeight;
  xc17 = xc18 - Gap;
  xc11 = (xc23 + xc00) / 2;
  xc10 = xc11 - lineHeight;
  xc09 = xc10 - Gap;
  xc08 = xc09 - d1;
  xc07 = xc08 - Gap;
  xc12 = xc11 + Gap;
  xc13 = xc12 + lineHeight;
  xc14 = xc13 + Gap;
  xc15 = xc14 + d1;
  xc16 = xc15 + lineHeight;

  int h = max(lineHeight, iconHeight);

  yc00 = 0;                                        // Top
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
  yc14 = yc10 + h + 1.5 * lineHeight + 2 * Margin; // Bottom
  yc13 = yc14 - h - 2 * Margin;
  yc12 = yc13 - lineHeight;
  yc11 = yc14 - 2 * lineHeight;

  leftIcons = xc21;
  xs = 0;

//  dsyslog ("%s %s %d Lineheight %i\n", __FILE__, __func__,  __LINE__, lineHeight);
//  dsyslog ("%s %s %d Iconheight %i\n", __FILE__, __func__,  __LINE__, iconHeight);
//  dsyslog ("%s %s %d Ellipse AU %ix%i\n", __FILE__, __func__,  __LINE__, (xc01 - 1 - xc00), (yc14 - 1 - yc11));
//  dsyslog ("%s %s %d Ellipse AO %ix%i\n", __FILE__, __func__,  __LINE__, (xc01 - 1 - xc00), (yc04 - 1 - yc00));

  int y1 = withInfo ? yc14 : yc06;
  int y0 = cOsd::OsdTop() + (Setup.ChannelInfoPos ? 0 : cOsd::OsdHeight() - y1);
  osd = CreateOsd(cOsd::OsdLeft(), y0, xc00, yc00, xc23 - 1, y1 - 1);

  animatedInfo.x0 = xc16; // info window left
  animatedInfo.x1 = xc23; // info window right
  animatedInfo.y0 = -y0  + lineHeight;
  animatedInfo.y1 = -Gap;
  animatedInfo.textColorBg = textColorBg;
  animatedInfo.shortTextColorFg = Theme.Color(clrEventShortText);
  animatedInfo.frameColorBr = frameColorBr;
  animatedInfo.frameColorBg = frameColorBg;

  if (withInfo) {
     // Rectangles:
     osd->DrawRectangle(xc00, yc00, xc23 - 1, y1 - 1, Theme.Color(clrBackground)); // Main background
     DrawRectangleOutline(osd, xc00, yc07, xc02 - 1, yc09 - 1, frameColorBr, frameColorBg, 15); // Left middle
     DrawRectangleOutline(osd, xc06, yc00, xc19 - 1, (lineHeight + 2 * Margin - Gap) / 2 - 1, frameColorBr, frameColorBg, 15); // Top middle
     // Upper Elbow:
     DrawRectangleOutline(osd, xc00, yc00, xc02 - 1, yc01 - 1, frameColorBr, frameColorBg, 3);
     DrawRectangleOutline(osd, xc00, yc01, xc02 - 1, yc06 - 1, frameColorBr, frameColorBg, 13);
     DrawRectangleOutline(osd, xc02, yc00, xc05 - 1, yc01 - 1, frameColorBr, frameColorBg, 14);
     osd->DrawRectangle(xc00, yc00, xc01 - 1, yc04 - 1, clrTransparent);
     osd->DrawEllipse  (xc00, yc00, xc01 - 1, yc04 - 1, frameColorBr, 2);
     osd->DrawEllipse  (xc00 + Margin, yc00 + Margin, xc01 - 1, yc04 - 1, frameColorBg, 2);
     osd->DrawEllipse  (xc02, yc01, xc03 - 1, yc03 - 1, frameColorBr, -2);
     osd->DrawEllipse  (xc02 - Margin, yc01 - Margin, xc03 - 1, yc03 - 1, frameColorBg, -2);
     // Lower Elbow:
     DrawRectangleOutline(osd, xc00, yc10, xc02 - 1, yc13 - 1, frameColorBr, frameColorBg, 7);
     DrawRectangleOutline(osd, xc00, yc13, xc02 - 1, yc14 - 1, frameColorBr, frameColorBg, 8);
     DrawRectangleOutline(osd, xc02, yc13, xc05 - 1, yc14 - 1, frameColorBr, frameColorBg, 14);
     osd->DrawRectangle(xc00, yc11, xc01 - 1, yc14 - 1, clrTransparent);
     osd->DrawEllipse  (xc00, yc11, xc01 - 1, yc14 - 1, frameColorBr, 3);
     osd->DrawEllipse  (xc00 + Margin, yc11, xc01 - 1, yc14 - 1 - Margin, frameColorBg, 3);
     osd->DrawEllipse  (xc02, yc12, xc03 - 1, yc13 - 1, frameColorBr, -3);
     osd->DrawEllipse  (xc02 - Margin, yc12, xc03 - 1, yc13 - 1 + Margin, frameColorBg, -3);
     // Status area:
     DrawRectangleOutline(osd, xc14, yc13, xc17 - 1, yc14 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xc22, yc13, xc22 + lineHeight / 2 - 1, yc14 - 1, frameColorBr, frameColorBg, 11);
     osd->DrawRectangle(xc22 + lineHeight / 2, yc13 + lineHeight / 2, xc23 - 1, yc14 - 1, clrTransparent);
     osd->DrawEllipse  (xc22 + lineHeight / 2, yc13, xc23 - 1, yc14 - 1, frameColorBr, 5);
     osd->DrawEllipse  (xc22 + lineHeight / 2, yc13 + Margin, xc23 - 1 - Margin, yc14 - 1 - Margin, frameColorBg, 5);
     // Status area tail middle:
     // Middle left middle
     osd->DrawEllipse  (xc10, yc07, xc11 - 1, yc08 - 1, frameColorBr, 1);
     osd->DrawEllipse  (xc10, yc07 + Margin, xc11 - 1 - Margin, yc08 - 1, frameColorBg, 1);
     osd->DrawRectangle(xc10, yc07, xc10 + Margin, yc08 - 1, frameColorBr);
     osd->DrawRectangle(xc10, yc08 - Margin, xc10 + lineHeight / 2 - 1, yc08 - 1, frameColorBr);
     DrawRectangleOutline(osd, xc10 + lineHeight / 2, yc08, xc11 - 1, yc09 - 1, frameColorBr, frameColorBg, 13);
     osd->DrawEllipse  (xc10, yc08, xc10 + lineHeight / 2 - 1, yc08 + lineHeight / 2 - 1, frameColorBr, -1);
     osd->DrawEllipse  (xc10, yc08 - Margin, xc10 + lineHeight / 2 - 1 + Margin, yc08 + lineHeight / 2 - 1, frameColorBg, -1);
     // Middle left bottom
     osd->DrawEllipse  (xc10, yc13, xc11 - 1, yc14 - 1, frameColorBr, 4);
     osd->DrawEllipse  (xc10, yc13, xc11 - 1 - Margin, yc14 - 1 - Margin, frameColorBg, 4);
     osd->DrawRectangle(xc10, yc13, xc10 + Margin, yc14 - 1, frameColorBr);
     osd->DrawRectangle(xc10, yc13, xc10 + lineHeight / 2 - 1, yc13 - 1 + Margin, frameColorBr);
     DrawRectangleOutline(osd, xc10 + lineHeight / 2, yc10, xc11 - 1, yc13 - 1, frameColorBr, frameColorBg, 7);
     osd->DrawEllipse  (xc10, yc13 - lineHeight / 2, xc10 + lineHeight / 2 - 1, yc13 - 1, frameColorBr, -4);
     osd->DrawEllipse  (xc10, yc13 - lineHeight / 2, xc10 + lineHeight / 2 - 1 + Margin, yc13 - 1 + Margin, frameColorBg, -4);
     //Middle right top
     osd->DrawEllipse  (xc12, yc02, xc13 - 1, yc05 - 1, frameColorBr, 2);
     osd->DrawEllipse  (xc12 + Margin, yc02 + Margin, xc13 - 1, yc05 - 1, frameColorBg, 2);
     osd->DrawRectangle(xc13 - Margin, yc02, xc13 -1, yc05 - 1, frameColorBr);
     osd->DrawRectangle(xc13 - lineHeight / 2, yc05 - Margin, xc13 - 1, yc05 - 1, frameColorBr);
     DrawRectangleOutline(osd, xc12, yc02 + lineHeight + Margin, xc12 + lineHeight / 2 - 1, yc06 - 1, frameColorBr, frameColorBg, 13);
     osd->DrawEllipse  (xc12 + lineHeight / 2, yc05, xc13 - 1, yc05 + lineHeight / 2 - 1, frameColorBr, -2);
     osd->DrawEllipse  (xc12 + lineHeight / 2 - Margin, yc05 - Margin, xc13 - 1, yc05 + lineHeight / 2 - 1, frameColorBg, -2);
     // Middle right middle
     DrawRectangleOutline(osd, xc12, yc07, xc12 + lineHeight / 2 - 1, yc09 - 1, frameColorBr, frameColorBg, 15);
     osd->DrawEllipse  (xc12, yc07, xc13 - 1, yc08 - 1, frameColorBr, 2);
     osd->DrawEllipse  (xc12 + Margin, yc07 + Margin, xc13 - 1, yc08 - 1, frameColorBg, 2);
     osd->DrawRectangle(xc13 - Margin, yc07, xc13 -1, yc08 - 1, frameColorBr);
     osd->DrawRectangle(xc13 - lineHeight / 2, yc08 - Margin, xc13 - 1, yc08 - 1, frameColorBr);
     osd->DrawEllipse  (xc12 + lineHeight / 2, yc08, xc13 - 1, yc08 + lineHeight / 2 - 1, frameColorBr, -2);
     osd->DrawEllipse  (xc12 + lineHeight / 2 - Margin, yc08 - Margin, xc13 - 1, yc08 + lineHeight / 2 - 1, frameColorBg, -2);
     osd->DrawRectangle(xc12 + Margin, yc07 + Margin, xc12 + lineHeight / 2 - 1 - Margin, yc09 - 1 - Margin, frameColorBg);
     // Middle right bottom
     osd->DrawEllipse  (xc12, yc13, xc13 - 1, yc14 - 1, frameColorBr, 3);
     osd->DrawEllipse  (xc12 + Margin, yc13, xc13 - 1, yc14 - 1 - Margin, frameColorBg, 3);
     osd->DrawRectangle(xc13 - Margin, yc13, xc13 - 1, yc14 - 1, frameColorBr);
     osd->DrawRectangle(xc13 - lineHeight / 2, yc13, xc13 - 1, yc13 - 1 + Margin, frameColorBr);
     DrawRectangleOutline(osd, xc12, yc10, xc12 + lineHeight / 2 - 1, yc13 - 1, frameColorBr, frameColorBg, 7);
     osd->DrawEllipse  (xc12 + lineHeight / 2, yc13 - lineHeight / 2, xc13 - 1, yc13 - 1, frameColorBr, -3);
     osd->DrawEllipse  (xc12 + lineHeight / 2 - Margin, yc13 - lineHeight / 2, xc13 - 1, yc13 - 1 + Margin, frameColorBg, -3);
     // Middle
     DrawRectangleOutline(osd, xc14, yc02, xc15 - 1, yc05 - 1, frameColorBr, frameColorBg, 15); // "Event time 1"
     DrawRectangleOutline(osd, xc14, yc07, xc15 - 1, yc08 - 1, frameColorBr, frameColorBg, 15); // "Event time 2"
     DrawRectangleOutline(osd, xc08, yc07, xc09 - 1, yc08 - 1, frameColorBr, frameColorBg, 15); // "Timer"
     osd->DrawText(xc08 + Margin, yc07 + Margin, "Timer", frameColorFg, frameColorBg, osdFont, xc09 - xc08 - 1 - 2 * Margin, yc08 - yc07 - 1 - 2 * Margin, taLeft | taBorder);
     // Top Right:
     DrawRectangleOutline(osd, xc22, yc00, xc22 + lineHeight / 2 - 1, yc01 - 1, frameColorBr, frameColorBg, 11);
     osd->DrawRectangle(xc22 + lineHeight / 2, yc00, xc23 - 1, yc00 + lineHeight / 2 - 1, clrTransparent);
     osd->DrawEllipse  (xc22 + lineHeight / 2, yc00, xc23 - 1, yc01 - 1, frameColorBr, 5);
     osd->DrawEllipse  (xc22 + lineHeight / 2, yc00 + Margin, xc23 - 1 - Margin, yc01 - 1 - Margin, frameColorBg, 5);
     }
  else {
     // Rectangles:
     osd->DrawRectangle(xc00, yc02, xc23 - 1, y1 - 1, Theme.Color(clrBackground));
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
     osd->DrawText(xc20, yc00, s, Theme.Color(clrDateFg), Theme.Color(clrDateBg), osdFont, xc21 - xc20, yc01, taRight | taBorder);
     lastDate = s;
     }
}

void cLCARSNGDisplayChannel::DrawTrack(void)
{
  cDevice *Device = cDevice::PrimaryDevice();
  const tTrackId *Track = Device->GetTrack(Device->GetCurrentAudioTrack());
  if (Track ? strcmp(lastTrackId.description, Track->description) : *lastTrackId.description) {
     osd->DrawText(xc14 + Margin, yc13 + Margin, Track ? Track->description : "", Theme.Color(clrTrackName), frameColorBg, osdFont, xc17 - xc14 - 1 - 2 * Margin, yc14 - yc13 - 2 * Margin, taTop | taLeft | taBorder);
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
  int Seen = (Total > 0) ? min(xc19 - xc06, int((xc19 - xc06) * double(Current) / Total)) : 0;
  if (initial || Seen != lastSeen) {
     int y0 = (lineHeight + 2 * Margin) / 2;
     int y1 = y0 + lineHeight / 3;
     // progress bar
     osd->DrawRectangle(xc06, y0, xc19 - 1, y1 - 1, Theme.Color(clrSeen));
     osd->DrawRectangle(xc06 + 2 + Seen, y0 + 2, xc19 - 3, y1 - 3, frameColorBg);
     // timeshift buffer
     int Buffer = GetLiveBuffer();
     if (Buffer > 0) {
        int Timeshift = max(0, int((xc19 - xc06) * double(Current - Buffer) / Total));
        int x1 = min(xc19 - 3, xc06 + 2 + Seen);
        osd->DrawRectangle(xc06 + 2 + Timeshift, y0 + 2, x1, y1 - 3, Theme.Color(clrChannelSymbolRecBg));
        }
     // display time remaining
     cString time = ((Current / 60.0) > 0.1) ? cString::sprintf("-%d", max((int)ceil((Total - Current) / 60.0), 0)) : "";
     int w = smlFont->Width(time);
     osd->DrawRectangle(xc14 + Margin, yc02 + lineHeight + 2 * Margin + Gap / 2, xc15 - Margin, yc06 - Margin, Theme.Color(clrBackground)); //Backgroung time remaining
     osd->DrawText(xc14 + Margin + (xc15 - xc14 - 1 - 2 * Margin - w), yc02 + lineHeight + 2 * Margin + Gap / 2, time, Theme.Color(clrEventShortText), textColorBg, smlFont, w, lineHeight - 2 * Margin, taRight | taBorder); // time remaining
     lastSeen = Seen;
     }
}

void cLCARSNGDisplayChannel::DrawDevice(void)
{
  const cDevice *Device = cDevice::ActualDevice();
  if (DrawDeviceData(osd, Device, xc06, yc13, xc09, yc14, xs, tinyFont, lastDeviceType, lastCamSlot, Device->DeviceNumber() != lastDeviceNumber)) {
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
     DrawDeviceSignal(osd, cDevice::ActualDevice(), xs + lineHeight / 2, yc13, xc09, yc14, lastSignalStrength, lastSignalQuality, initial);
     lastSignalDisplay = Now;
     }
}

void cLCARSNGDisplayChannel::DrawScreenResolution(void)
{
  cString resolution = GetScreenResolutionIcon();
  if (!(strcmp(resolution, oldResolution) == 0)) {
     if (strcmp(resolution, "") == 0) {
        osd->DrawRectangle(xc18 + Margin, yc13 + Margin, leftIcons, yc14 - Margin, frameColorBg);
        }
     int w = osdFont->Width(*resolution) + 2 * textBorder;
     int x = leftIcons - w - SymbolSpacing;
     osd->DrawText(x, yc13 + Margin, cString::sprintf("%s", *resolution), Theme.Color(clrChannelSymbolOn), frameColorBr, osdFont, w, yc14 - yc13 - 2 * Margin, taRight | taBorder);
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
     int x = xc21;
     x -= bmRecording.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc13 + (yc14 - yc13 - bmRecording.Height()) / 2, bmRecording, Theme.Color(rec ? On ? clrChannelSymbolRecFg : clrChannelSymbolOff : clrChannelSymbolOff), rec ? On ? Theme.Color(clrChannelSymbolRecBg) : frameColorBr : frameColorBr);
     lastOn = On;
     }
}
 
void cLCARSNGDisplayChannel::DrawEventRec(const cEvent *Present, const cEvent *Following)
{
  for (int i = 0; i < 2; i++) {
      const cEvent *e = !i ? Present : Following;
      tColor recColor = !i ? Theme.Color(clrChannelSymbolRecBg) : Theme.Color(clrButtonYellowBg);
      int y0 = !i ? yc02 : yc07;
      int y1 = !i ? yc05 : yc08;
      if (e) {
         LOCK_TIMERS_READ;
         eTimerMatch TimerMatch = tmNone;
         const cTimer *Timer = Timers->GetMatch(e, &TimerMatch);
         if (Timer && Timer->HasFlags(tfActive) && TimerMatch == tmFull)
            osd->DrawEllipse(xc15 + Gap, y0 + Margin + Gap, xc16 - Gap - 1, y1 - Margin - Gap - 1, recColor, 0);
         else
            osd->DrawRectangle(xc15, y0, xc16 - 1, y1 - 1, Theme.Color(clrBackground));
         }
      }
}

void cLCARSNGDisplayChannel::DrawTimer(void)
{
//  return;
  int CountTimers = 0;
  LOCK_TIMERS_READ;
  for (const cTimer *Timer = Timers->First(); Timer; Timer = Timers->Next(Timer)) {
     if (Timer->HasFlags(tfActive))
        CountTimers++;
     }
  if (CountTimers == 0)
     return;
  cSortedTimers SortedTimers(Timers);
  int i = 0;
  int j = 0;
  while (i < min(CountTimers, 3)) {
     int y = yc07 + Margin + i * lineHeight + (i == 2 ? Margin : 0);
     if (const cTimer *Timer = SortedTimers[i + j]) {
        time_t Now = time(NULL);
        if (!(Timer->HasFlags(tfActive)) || (Timer->StopTime() < Now))
           j++;
        else {
           cString Date;
           cString Number;
           bool isRecording = false;
           if (Timer->Recording()) {
              isRecording = true;
              if (cRecordControl *RecordControl = cRecordControls::GetRecordControl(Timer))
                 if (const cDevice *Device = RecordControl->Device())
                    Number = itoa(Device->DeviceNumber() + 1);
              Date = cString::sprintf("- %s", *TimeString(Timer->StopTime()));
              }
           else
              Date = DayDateTime(Timer->StartTime());
           const cChannel *Channel = Timer->Channel();
           const cEvent *Event = Timer->Event();
           int y1 = i * Gap / 2;
           int x1 = xc04 + smlFont->Width("Mo. 00.00. 00:00") + 2 * textBorder;
           int x2 = x1 + Gap;
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
              osd->DrawText(xc01, y + y1 + Margin, cString::sprintf("%d", Channel->Number()), frameColorFg, frameColorBg, smlFont, xc02 - xc01 - Gap - 1, lineHeight - 3 * Margin, taRight | taBorder);
              osd->DrawText(xc04, y + y1, cString::sprintf("%s", *Date), timerColor, textColorBg, smlFont, x1 - xc04 - Gap - 1, lineHeight - Gap / 2, taRight | taBorder);
	      if (isRecording)
                 osd->DrawText(xc04, y + y1, cString::sprintf("Rec: #%s %s", *Number, *Date), Theme.Color(clrChannelSymbolRecBg), textColorBg, smlFont, x1 - xc04 - 1, lineHeight - Gap / 2, taRight | taBorder);
              else
                 osd->DrawText(xc04, y + y1, cString::sprintf("%s", *Date), timerColor, textColorBg, smlFont, x1 - xc04 - 1, lineHeight - Gap / 2, taRight | taBorder);
              int w = smlFont->Width(File) + 2 * textBorder; // smlFont width to short
              osd->DrawRectangle(x2, y + y1, xc07 - 1, y + y1 + lineHeight - Gap / 2, Theme.Color(clrBackground));
              osd->DrawText(x2, y + y1, cString::sprintf("%s", File), timerColor, textColorBg, smlFont, min(w, xc07 - x1 - 1), lineHeight - Gap / 2, taLeft | taBorder);
              }
           i++;
           }
        }
     }
}

void cLCARSNGDisplayChannel::SetChannel(const cChannel *Channel, int Number)
{
  DELETENULL(drawDescription);
  int x = xc21;
  DrawRectangleOutline(osd, xc18, yc13, xc21 - 1, yc14 - 1, frameColorBr, frameColorBg, 15);
  if (Channel && !Channel->GroupSep()) {
     x -= bmRecording.Width() + SymbolSpacing;
     x -= bmEncrypted.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc13 + (yc14 - yc13 - bmEncrypted.Height()) / 2, bmEncrypted, Theme.Color(Channel->Ca() ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
     x -= bmDolbyDigital.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc13 + (yc14 - yc13 - bmDolbyDigital.Height()) / 2, bmDolbyDigital, Theme.Color(Channel->Dpid(0) ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
     x -= bmAudio.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc13 + (yc14 - yc13 - bmAudio.Height()) / 2, bmAudio, Theme.Color(Channel->Apid(1) ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
     if (Channel->Vpid()) {
        x -= bmTeletext.Width() + SymbolSpacing;
        osd->DrawBitmap(x, yc13 + (yc14 - yc13 - bmTeletext.Height()) / 2, bmTeletext, Theme.Color(Channel->Tpid() ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
        }
     else if (Channel->Apid(0)) {
        x -= bmRadio.Width() + SymbolSpacing;
        osd->DrawBitmap(x, yc13 + (yc14 - yc13 - bmRadio.Height()) / 2, bmRadio, Theme.Color(clrChannelSymbolOn), frameColorBr);
        }
     initial = true; // make shure DrawBlinkingRec() refreshs recording icon
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

  int xR = (withInfo) ? xc09 : xc21;
  int w = tallFont->Width(ChName);
  osd->DrawText(xc00 + Margin, yc02 + Margin, ChNumber, frameColorFg, frameColorBg, tallFont, xc02 - xc00 - 2 * Margin, yc06 - yc02 - 2 * Margin, taCenter | taRight | taBorder);
  osd->DrawRectangle(xc03, yc02, xR - 1, yc06 - 1, Theme.Color(clrBackground));
  osd->DrawText(xc03, yc02 + Margin, ChName, Theme.Color(clrChannelName), textColorBg, tallFont, min(w, xR - xc03 - 1), yc06 - yc02 - 2 * Margin, taCenter | taLeft);
  lastSignalDisplay = 0;

  if (withInfo) {
     if (Channel) {
        osd->DrawText(xc02, yc13 + Margin, cSource::ToString(Channel->Source()), frameColorFg, frameColorBg, osdFont, xc05 - xc02 - Gap, yc14 - yc13 - 2 * Margin, taRight | taBorder);
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
      int x = xc16;
      int y0 = !i ? yc02 : yc07;
      int y1 = !i ? yc05 : yc08;
      int y2 = !i ? yc06 : yc09;
      if (e) {
//       draw Time:
         osd->DrawText(xc14 + Margin, y0  + Margin, e->GetTimeString(), frameColorFg, frameColorBg, osdFont, xc15 - xc14 - 1 - 2 * Margin, y1 - y0 - 1 - 2 * Margin, taRight | taBorder);
//       draw Title:
         int w = osdFont->Width(e->Title());
         osd->DrawRectangle(x, y0, xc21 - 1, y1 - 1, Theme.Color(clrBackground));
         osd->DrawText(x, y0 + Margin, e->Title(), Theme.Color(clrEventTitle), textColorBg, osdFont, min(w, xc21 - x), y1 - y0 - 1 - 2 * Margin);
//       draw ShortText:
         w = smlFont->Width(e->ShortText());
         osd->DrawRectangle(x, y1, xc21 - 1, y2 - 1, Theme.Color(clrBackground));
         osd->DrawText(x, y1 + Gap / 2, e->ShortText(), Theme.Color(clrEventShortText), textColorBg, smlFont, min(w, xc21 - x), lineHeight - 2 * Margin, taBorder);
         }
      else {
         DrawRectangleOutline(osd, xc14, y0, xc15 - 1, y1 - 1, frameColorBr, frameColorBg, 15);
         osd->DrawRectangle(xc15, y0, xc21 - 1, y2 - 1, Theme.Color(clrBackground));
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
     int xv00, xv01, yv00, yv01;
     if (withInfo) {
        xv00 = xc06;
        xv01 = xc19;
        yv00 = yc00;
        yv01 = yc01;
        } 
     else {
        xv00 = xc03;
        xv01 = xc21;
        yv00 = yc02;
        yv01 = yc06;
        }
     if (!messageBox)
        messageBox = new cLCARSNGMessageBox(osd, cRect(xv00, yv00, xv01 - xv00, yv01 - yv00), false);
     messageBox->SetMessage(Type, Text);
     }
  else {
     DELETENULL(messageBox);
     message = false;
     }
}

#if APIVERSNUM > 20101
void cLCARSNGDisplayChannel::SetPositioner(const cPositioner *Positioner)
{
  if (Positioner) {
     int y0 = yc01 - (yc01 - yc00) / 2 + Gap / 2;
     int y1 = yc01 + ShowSeenExtent;
     DrawDevicePosition(osd, Positioner, xc06, y0, xc19, y1, lastCurrentPosition);
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
void cLCARSNGDisplayChannel::DrawVolume(void)
{
   if (!message && withInfo) {
      int volume = statusMonitor->GetVolume();
      if (volume != lastVolume) {
         if (!volumeBox)
            volumeBox = new cLCARSNGVolumeBox(osd, cRect(xc06, yc00, xc19 - xc06, yc01 - yc00), false);
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

void cLCARSNGDisplayChannel::DrawRaster(void)
{
  int bottom = yc14;
  int top = yc00;
  tColor gridColor = 0xffff7700;
  int offset = lineHeight;

// int xc00, xc01, xc02, xc03, xc04, xc05, xc06, xc07, xc08, xc09, xc10, xc11, xc12, xc13, xc14, xc15, xc16, xc17, xc18, xc19, xc20, xc21, xc22, xc23;
  int xc[26] = { xs, leftIcons, xc00, xc01, xc02, xc03, xc04, xc05, xc06,
                 xc07, xc08, xc09, xc10, xc11, xc12, xc13, xc14, xc15, xc16,
                 xc17, xc18, xc19, xc20, xc21, xc22, xc23};
  char strxc[27][6] = { "xs", "lIcon", "xc00", "xc01", "xc02", "xc03", "xc04", "xc05", "xc06",
                       "xc07", "xc08", "xc09", "xc10", "xc11", "xc12", "xc13", "xc14", "xc15", "xc16",
                       "xc17", "xc18", "xc19", "xc20", "xc21", "xc22", "xc23", '\0'};

  for (int i = 0; strxc[i][0]; i++) {
     osd->DrawRectangle(xc[i], top, xc[i] + 1, bottom - 1, gridColor);
     osd->DrawText(xc[i], top + offset, cString(strxc[i]), gridColor, clrTransparent, tinyFont);

     offset = offset + lineHeight;

     if ((i % 3) == 0)
        offset = lineHeight;
  }
//  return;
  offset = lineHeight;

// int yc00, yc01, yc02, yc03, yc04, yc05, yc06, yc07, yc08, yc09, yc10, yc11, yc12, yc13, yc14;
  int yc[15] = { yc00, yc01, yc02, yc03, yc04, yc05, yc06, yc07, yc08, yc09, yc10, yc11, yc12, yc13, yc14};
  char stryc[16][6] = { "yc00", "yc01", "yc02", "yc03", "yc04", "yc05", "yc06", "yc07", "yc08", "yc09", "yc10", "yc11", "yc12", "yc13", "yc14", '\0'};

  for (int i = 0; stryc[i][0]; i++) {
     osd->DrawRectangle(xc00, yc[i], xc23 - 1, yc[i] + 1, gridColor);
     osd->DrawText(xs + 1.5 * offset, yc[i], cString(stryc[i]), gridColor, clrTransparent, tinyFont);

     offset = offset + lineHeight;

     if ((i % 3) == 0)
        offset = lineHeight;
  }
}

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
//  DrawRaster();
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
  int d = 5 * lineHeight;
  int xy = lineHeight + 2 * Margin;
  int yh = (xy - Gap) / 2; //lineHeight / 2 + 2 * Margin;
  int x0 = 0;              // rectangle left
  int x6 = aI.x1 - aI.x0;  // rectangle right
  int x1 = x0 + xy;
  int x5 = x6 - lineHeight;
  int x4 = x5 - Gap;
  int x3 = x4 - 2 * d;
  int x2 = x3 - Gap; 
  int y0 = 0;              // rectangle top
  int y1 = height;         // rectangle bottom

  BracketPixmap = osd->CreatePixmap(2, cRect(aI.x0, aI.y1 - height, aI.x1 - aI.x0, height));
  if (!BracketPixmap) 
     return; 
  
  BracketPixmap->SetAlpha(255);
  BracketPixmap->Fill(aI.textColorBg);

  DrawRectangleOutline(BracketPixmap, x1, 0, x2 - x1, yh, aI.frameColorBr, aI.frameColorBg, 14);
  DrawRectangleOutline(BracketPixmap, x3, 0, x4 - x3, yh, aI.frameColorBr, aI.frameColorBg, 15);
  DrawRectangleOutline(BracketPixmap, x3, yh + Gap, x4 - x3, xy - yh - Gap, aI.frameColorBr, aI.frameColorBg, 15);
  DrawRectangleOutline(BracketPixmap, 0, xy, x1, y1 - y0 - 2 * xy, aI.frameColorBr, aI.frameColorBg, 1);
  DrawRectangleOutline(BracketPixmap, x1, y1 - y0 - yh, x2 - x1, yh, aI.frameColorBr, aI.frameColorBg, 14);
  DrawRectangleOutline(BracketPixmap, x3, y1 - y0 - xy, x4 - x3, xy, aI.frameColorBr, aI.frameColorBg, 15);

  // Upper Elbow part 1:
  BracketPixmap->DrawRectangle(cRect(0, 0, xy, xy), clrTransparent);
  BracketPixmap->DrawEllipse(cRect(0, 0, x1, xy), aI.frameColorBr, 2);
  BracketPixmap->DrawEllipse(cRect(0 + Margin, 0 + Margin, x1 - Margin, xy - Margin), aI.frameColorBg, 2);
  // Lower Elbow part 1:
  BracketPixmap->DrawRectangle(cRect(0, y1 - xy, xy, xy), clrTransparent);
  BracketPixmap->DrawEllipse(cRect(0, y1 - y0 - xy, x1, xy), aI.frameColorBr, 3);
  BracketPixmap->DrawEllipse(cRect(0 + Margin, y1 - y0 - xy, x1 - Margin, x1 - Margin), aI.frameColorBg, 3);

  BracketPixmap->DrawRectangle(cRect(x1 - Margin, yh, Margin, y1 - y0 - 2 * yh), aI.frameColorBr);

  // Upper Elbow part 2:
  BracketPixmap->DrawEllipse(cRect(x1 , yh, yh, yh), aI.frameColorBr, -2);
  BracketPixmap->DrawEllipse(cRect(x1 - Margin, yh - Margin, yh + Margin, yh + Margin), aI.frameColorBg, -2);
  // Lower Elbow part 2:
  BracketPixmap->DrawEllipse(cRect(x1, y1 - y0 - 2 * yh, yh, yh), aI.frameColorBr, -3);
  BracketPixmap->DrawEllipse(cRect(x1 - Margin, y1 - y0 - 2 * yh, yh + Margin, yh + Margin), aI.frameColorBg, -3);

  // Top Right
  DrawRectangleOutline(BracketPixmap, x5, 0, lineHeight / 2, x1, aI.frameColorBr, aI.frameColorBg, 11);
  BracketPixmap->DrawRectangle(cRect(x5 + lineHeight / 2, 0, lineHeight / 2, xy / 2), clrTransparent);
  BracketPixmap->DrawEllipse(cRect(x5 + lineHeight / 2, 0, lineHeight / 2, xy), aI.frameColorBr, 5);
  BracketPixmap->DrawEllipse(cRect(x5 + lineHeight / 2, 0 + Margin, lineHeight / 2 - Margin, xy - 2 * Margin), aI.frameColorBg, 5);
  // Bottom Right
  DrawRectangleOutline(BracketPixmap, x5, y1 - y0 - xy, lineHeight / 2, xy, aI.frameColorBr, aI.frameColorBg, 11);
  BracketPixmap->DrawRectangle(cRect(x5 + lineHeight / 2, y1 - xy / 2, lineHeight / 2, xy / 2), clrTransparent);
  BracketPixmap->DrawEllipse(cRect(x5 + lineHeight / 2, y1 - xy, lineHeight / 2, xy), aI.frameColorBr, 5);
  BracketPixmap->DrawEllipse(cRect(x5 + lineHeight / 2, y1 - xy + Margin, lineHeight / 2 - Margin, xy - 2 * Margin), aI.frameColorBg, 5);  

  return;

  BracketPixmap->DrawRectangle(cRect(0, 0, x6, Margin), aI.frameColorBr);                // border oben
  BracketPixmap->DrawRectangle(cRect(0, y1 - y0 - Margin, x6, Margin), aI.frameColorBr); // border unten
  BracketPixmap->DrawRectangle(cRect(0, 0, Margin, y1 - y0), aI.frameColorBr);                // border links
  BracketPixmap->DrawRectangle(cRect(x6 - Margin, 0, Margin, y1 - y0), aI.frameColorBr); // border rechts
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
  int x1 = aI.x1 - lineHeight - Gap;                   // text right

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
     TextPixmap->Fill(clrTransparent);
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
