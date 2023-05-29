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
  xc00 = 0;
  xc00m = xc00 + lineHeight / 2;
  xc01 = xc00 + 2 * lineHeight;
  xc02 = xc00 + d;
  xc02m = xc02 + lineHeight;
  xc03 = xc02 + lineHeight;
  xc04 = xc02 + d / 4;
  xc05 = xc02 + d;
  xc06 = xc05 + Gap;
  xc15 = cOsd::OsdWidth();
  xc14 = xc15 - lineHeight;
  xc13 = xc14 - Gap;
  xc07 = (xc15 + xc00) / 2;
  xc06n = xc07 - lineHeight;
  xc06m = xc06n - Gap;
  xc06l = xc06m - d1;
  xc06k = xc06l - Gap;
  xc08 = xc07 + Gap;
  xc09 = xc08 + lineHeight;
  xc10 = xc09 + Gap;
  xc10m = xc10 + d1;
  xc10n = xc10m + lineHeight;
  xc11 = (xc10 + xc13 + Gap) / 2;
  xc10o = xc11 - 2 * lineHeight;
  xc10p = xc10o + Gap;
  xc12 = xc11 + Gap;

  leftIcons = xc13;

  yc0B = 0;
  yc0A = yc0B + max(lineHeight, iconHeight) + 2 * Margin;
  yc00 = yc0A + lineHeight;
  yc00m = yc0B + 2 * lineHeight;
  yc01 = yc00 + lineHeight + Margin;
  yc02 = yc01 + lineHeight + Margin;
  yc03 = yc00; // + Gap;
  yc04 = yc03 + 2 * lineHeight + 2 * Margin;
  yc05 = yc04 + Gap;
  yc06 = yc05 + 2 * lineHeight + 2 * Margin;

  yc07 = yc06 + Gap;
  yc12 = yc07 + 2.5 * lineHeight + Gap / 2 + 2 * Margin;
  yc11 = yc12 - yc0A;
  yc10 = yc11 - lineHeight;
  yc09 = yc11 - (d / 4 - lineHeight / 2);
  yc08 = yc12 - (d / 2 - lineHeight / 2);

  xs = 0;

//  dsyslog ("%s %s %d Lineheight %i\n", __FILE__, __func__,  __LINE__, lineHeight);
//  dsyslog ("%s %s %d Iconheight %i\n", __FILE__, __func__,  __LINE__, iconHeight);
//  dsyslog ("%s %s %d Ellipse AU %ix%i\n", __FILE__, __func__,  __LINE__, (xc01 - 1 - xc00), (yc12 - 1 - yc08));
//  dsyslog ("%s %s %d Ellipse AO %ix%i\n", __FILE__, __func__,  __LINE__, (xc01 - 1 - xc00), (yc00m - 1 - yc0B));

  int y1 = withInfo ? yc12 : yc02;
  int y0 = cOsd::OsdTop() + (Setup.ChannelInfoPos ? 0 : cOsd::OsdHeight() - y1);
  osd = CreateOsd(cOsd::OsdLeft(), y0, xc00, yc0B, xc15 - 1, y1 - 1);

  animatedInfo.x0 = xc10n; // text left
  animatedInfo.x1 = xc13;  // text right
  animatedInfo.y0 = -y0  + lineHeight;
  animatedInfo.y1 = -Gap;
  animatedInfo.textColorBg = textColorBg;
  animatedInfo.shortTextColorFg = Theme.Color(clrEventShortText);
  animatedInfo.frameColorBr = frameColorBr;
  animatedInfo.frameColorBg = frameColorBg;

  if (withInfo) {
     // Rectangles:
     osd->DrawRectangle(xc00, yc0B, xc15 - 1, y1 - 1, Theme.Color(clrBackground)); // Main background
     DrawRectangleOutline(osd, xc00, yc05, xc02 - 1, yc06 - 1, frameColorBr, frameColorBg, 15); // Left middle
     DrawRectangleOutline(osd, xc06, yc0B, xc11 - 1, (yc0A - yc0B) / 2 - 1, frameColorBr, frameColorBg, 15); // Top middle
     // Upper Elbow:
     DrawRectangleOutline(osd, xc00, yc0B, xc02 - 1, yc0A - 1, frameColorBr, frameColorBg, 3);
     DrawRectangleOutline(osd, xc00, yc0A, xc02 - 1, yc02 - 1, frameColorBr, frameColorBg, 13);
     DrawRectangleOutline(osd, xc02, yc0B, xc05 - 1, yc0A - 1, frameColorBr, frameColorBg, 14);
     osd->DrawRectangle(xc00, yc0B, xc01 - 1, yc00m - 1, clrTransparent);
     osd->DrawEllipse  (xc00, yc0B, xc01 - 1, yc00m - 1, frameColorBr, 2);
     osd->DrawEllipse  (xc00 + Margin, yc0B + Margin, xc01 - 1, yc00m - 1, frameColorBg, 2);
     osd->DrawEllipse  (xc02 - 1, yc0A, xc02m - 1, yc00 - 1, frameColorBr, -2);
     osd->DrawEllipse  (xc02 - 1 - Margin, yc0A - Margin, xc02m - 1 - Margin, yc00 - 1 - Margin, frameColorBg, -2);
     // Lower Elbow:
     DrawRectangleOutline(osd, xc00, yc07, xc02 - 1, yc11 - 1, frameColorBr, frameColorBg, 7);
     DrawRectangleOutline(osd, xc00, yc11, xc02 - 1, yc12 - 1, frameColorBr, frameColorBg, 8);
     DrawRectangleOutline(osd, xc02, yc11, xc05 - 1, yc12 - 1, frameColorBr, frameColorBg, 14);
     osd->DrawRectangle(xc00, yc08, xc01 - 1, yc12 - 1, clrTransparent);
     osd->DrawEllipse  (xc00, yc08, xc01 - 1, yc12 - 1, frameColorBr, 3);
     osd->DrawEllipse  (xc00 + Margin, yc08, xc01 - 1, yc12 - 1 - Margin, frameColorBg, 3);
     osd->DrawEllipse  (xc02 - 1, yc10, xc02m - 1, yc11 - 1, frameColorBr, -3);
     osd->DrawEllipse  (xc02 - 1 - Margin, yc10, xc02m - 1, yc11 - 1 + Margin, frameColorBg, -3);
     // Status area:
     DrawRectangleOutline(osd, xc10, yc11, xc10o - 1, yc12 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xc14, yc11, xc14 + lineHeight / 2 - 1, yc12 - 1, frameColorBr, frameColorBg, 11);
     osd->DrawRectangle(xc14 + lineHeight / 2, yc11 + lineHeight / 2, xc15 - 1, yc12 - 1, clrTransparent);
     osd->DrawEllipse  (xc14 + lineHeight / 2, yc11, xc15 - 1, yc12 - 1, frameColorBr, 5);
     osd->DrawEllipse  (xc14 + lineHeight / 2, yc11 + Margin, xc15 - 1 - Margin, yc12 - 1 - Margin, frameColorBg, 5);
     // Status area tail middle:
     // Middle left middle
     osd->DrawEllipse  (xc06n, yc05, xc07 - 1, yc06 - lineHeight - 1, frameColorBr, 1);
     osd->DrawEllipse  (xc06n + Margin, yc05 + Margin, xc07 - 1 - Margin, yc06 - lineHeight - 1 - Margin, frameColorBg, 1);
     DrawRectangleOutline(osd, xc07 - lineHeight / 2, yc05 + lineHeight + Margin, xc07 - 1, yc06 - 1, frameColorBr, frameColorBg, 13);
     osd->DrawEllipse  (xc06n, yc06 - lineHeight - 1, xc07 - lineHeight / 2 - 1, yc06 - lineHeight / 2 - 1, frameColorBr, -1);
     osd->DrawEllipse  (xc06n + Margin, yc06 - lineHeight - 1 - Margin, xc07 - lineHeight / 2 - 1 + Margin, yc06 - lineHeight / 2 - 1 - Margin, frameColorBg, -1);
     // Middle left bottom
     DrawRectangleOutline(osd, xc06n, yc11, xc07 - 1, yc12 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xc07 - lineHeight / 2, yc07, xc07 - 1, yc11 + Margin, frameColorBr, frameColorBg, 7);
     osd->DrawEllipse  (xc06n, yc11 - lineHeight / 2, xc07 - lineHeight / 2 - 1, yc11, frameColorBr, -4);
     osd->DrawEllipse  (xc06n + Margin, yc11 - lineHeight / 2 + Margin, xc07 - lineHeight / 2 - 1 + Margin, yc11 + Margin, frameColorBg, -4);
     //Middle right top
     osd->DrawEllipse  (xc08, yc03, xc09 - 1, yc04 - lineHeight - 1, frameColorBr, 2);
     osd->DrawEllipse  (xc08 + Margin, yc03 + Margin, xc09 - 1 - Margin, yc04 - lineHeight - 1 - Margin, frameColorBg, 2);
     DrawRectangleOutline(osd, xc08, yc03 + lineHeight + Margin, xc08 + lineHeight / 2 - 1, yc04 - 1, frameColorBr, frameColorBg, 13);
     osd->DrawEllipse  (xc08 + lineHeight / 2, yc04 - lineHeight - 1, xc09 - 1, yc04 - lineHeight / 2 - 1, frameColorBr, -2);
     osd->DrawEllipse  (xc08 + lineHeight / 2 - Margin, yc04 - lineHeight - 1 - Margin, xc09 - 1 - Margin, yc04 - lineHeight / 2 - 1 - Margin, frameColorBg, -2);
     // Middle right middle
     DrawRectangleOutline(osd, xc08, yc05, xc08 + lineHeight / 2 - 1, yc06 - 1, frameColorBr, frameColorBg, 15);
     osd->DrawEllipse  (xc08, yc05, xc09 - 1, yc06 - lineHeight - 1, frameColorBr, 2);
     osd->DrawEllipse  (xc08 + Margin, yc05 + Margin, xc09 - 1 - Margin, yc06 - lineHeight - 1 - Margin, frameColorBg, 2);
     osd->DrawEllipse  (xc08 + lineHeight / 2, yc06 - lineHeight - 1, xc09 - 1, yc06 - lineHeight / 2 - 1, frameColorBr, -2);
     osd->DrawEllipse  (xc08 + lineHeight / 2 - Margin, yc06 - lineHeight - 1 - Margin, xc09 - 1 - Margin, yc06 - lineHeight / 2 - 1 - Margin, frameColorBg, -2);
     osd->DrawRectangle(xc08 + Margin, yc05 + Margin, xc08 + lineHeight / 2 - 1 - Margin, yc06 - 1 - Margin, frameColorBg);
     // Middle right bottom
     DrawRectangleOutline(osd, xc08, yc11, xc09 - 1, yc12 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xc08, yc07, xc08 + lineHeight / 2 - 1, yc11 + Margin, frameColorBr, frameColorBg, 7);
     osd->DrawEllipse  (xc08 + lineHeight / 2, yc11 - lineHeight / 2, xc09 - 1, yc11, frameColorBr, -3);
     osd->DrawEllipse  (xc08 + lineHeight / 2 - Margin, yc11 - lineHeight / 2 + Margin, xc09 - 1 - Margin, yc11 + Margin, frameColorBg, -3);
     // Middle
     DrawRectangleOutline(osd, xc10, yc03, xc10m - 1, yc04 - lineHeight - 1, frameColorBr, frameColorBg, 15); // "Time 1"
     DrawRectangleOutline(osd, xc10, yc05, xc10m - 1, yc06 - lineHeight - 1, frameColorBr, frameColorBg, 15); // "Time 2"
     DrawRectangleOutline(osd, xc06l, yc05, xc06m - 1, yc06 - lineHeight - 1, frameColorBr, frameColorBg, 15); // "Timer"
     osd->DrawText(xc06l + Margin, yc05 + Margin, "Timer", frameColorFg, frameColorBg, osdFont, xc06m - xc06l - 1 - 2 * Margin, lineHeight - 2 * Margin, taLeft | taBorder);
     // Top Right:
     DrawRectangleOutline(osd, xc14, yc0B, xc14 + lineHeight / 2 - 1, yc0A - 1, frameColorBr, frameColorBg, 11);
     osd->DrawRectangle(xc14 + lineHeight / 2, yc0B, xc15 - 1, yc0B + lineHeight / 2 - 1, clrTransparent);
     osd->DrawEllipse  (xc14 + lineHeight / 2, yc0B, xc15 - 1, yc0A - 1, frameColorBr, 5);
     osd->DrawEllipse  (xc14 + lineHeight / 2, yc0B + Margin, xc15 - 1 - Margin, yc0A - 1 - Margin, frameColorBg, 5);
     }
  else {
     // Rectangles:
     osd->DrawRectangle(xc00, yc00, xc15 - 1, y1 - 1, Theme.Color(clrBackground));
     osd->DrawRectangle(xc00, yc00, xc02 - 1, yc02 - 1, frameColorFg);
     }
}

cLCARSNGDisplayChannel::~cLCARSNGDisplayChannel()
{
  Cancel(3);
  delete drawDescription;
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
     osd->DrawText(xc12, yc0B, s, Theme.Color(clrDateFg), Theme.Color(clrDateBg), osdFont, xc13 - xc12, yc0A, taRight | taBorder);
     lastDate = s;
     }
}

void cLCARSNGDisplayChannel::DrawTrack(void)
{
  cDevice *Device = cDevice::PrimaryDevice();
  const tTrackId *Track = Device->GetTrack(Device->GetCurrentAudioTrack());
  if (Track ? strcmp(lastTrackId.description, Track->description) : *lastTrackId.description) {
     osd->DrawText(xc10 + Margin, yc11 + Margin, Track ? Track->description : "", Theme.Color(clrTrackName), frameColorBg, osdFont, xc10o - xc10 - 1 - 2 * Margin, yc12 - yc11 - 2 * Margin, taTop | taLeft | taBorder);
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
  int Seen = (Total > 0) ? min(xc11 - xc06, int((xc11 - xc06) * double(Current) / Total)) : 0;
  if (initial || Seen != lastSeen) {
     int y0 = yc0A - (yc0A - yc0B) / 2 + Gap / 2;
     int y1 = yc0A - ShowSeenExtent;
     // progress bar
     osd->DrawRectangle(xc06, y0, xc11 - 1, y1 - 1, Theme.Color(clrSeen));
     osd->DrawRectangle(xc06 + 2 + Seen, y0 + 2, xc11 - 3, y1 - 3, frameColorBg);
     // timeshift buffer
     int Buffer = GetLiveBuffer();
     if (Buffer > 0) {
        int Timeshift = max(0, int((xc11 - xc06) * double(Current - Buffer) / Total));
        int x1 = min(xc11 - 3, xc06 + 2 + Seen);
        osd->DrawRectangle(xc06 + 2 + Timeshift, y0 + 2, x1, y1 - 3, Theme.Color(clrChannelSymbolRecBg));
        }
     // display time remaining
     cString time = ((Current / 60.0) > 0.1) ? cString::sprintf("-%d", max((int)ceil((Total - Current) / 60.0), 0)) : "";
     int w = smlFont->Width(time);
     osd->DrawRectangle(xc10 + Margin, yc03 + lineHeight + 2 * Margin + Gap / 2, xc10m - Margin, yc04 - Margin, Theme.Color(clrBackground)); //Backgroung time remaining
     osd->DrawText(xc10 + Margin + (xc10m - xc10 - 1 - 2 * Margin - w), yc03 + lineHeight + 2 * Margin + Gap / 2, time, Theme.Color(clrEventShortText), textColorBg, smlFont, w, lineHeight - 2 * Margin, taRight | taBorder); // time remaining
     lastSeen = Seen;
     }
}

void cLCARSNGDisplayChannel::DrawDevice(void)
{
  const cDevice *Device = cDevice::ActualDevice();
  if (DrawDeviceData(osd, Device, xc06, yc11, xc06m, yc12, xs, tinyFont, lastDeviceType, lastCamSlot, Device->DeviceNumber() != lastDeviceNumber)) {
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
     DrawDeviceSignal(osd, cDevice::ActualDevice(), xs + lineHeight / 2, yc11, xc06m, yc12, lastSignalStrength, lastSignalQuality, initial);
     lastSignalDisplay = Now;
     }
}

void cLCARSNGDisplayChannel::DrawScreenResolution(void)
{
  cString resolution = GetScreenResolutionIcon();
  if (!(strcmp(resolution, oldResolution) == 0)) {
     if (strcmp(resolution, "") == 0) {
        osd->DrawRectangle(xc10 + Margin, yc11 + Margin, leftIcons, yc12 - Margin, frameColorBg);
        }
     int w = osdFont->Width(*resolution) + 2 * textBorder;
     int x = leftIcons - w - SymbolSpacing;
     osd->DrawText(x, yc11 + Margin, cString::sprintf("%s", *resolution), Theme.Color(clrChannelSymbolOn), frameColorBr, osdFont, w, yc12 - yc11 - 2 * Margin, taRight | taBorder);
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
     int x = xc13;
     x -= bmRecording.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmRecording.Height()) / 2, bmRecording, Theme.Color(rec ? On ? clrChannelSymbolRecFg : clrChannelSymbolOff : clrChannelSymbolOff), rec ? On ? Theme.Color(clrChannelSymbolRecBg) : frameColorBr : frameColorBr);
     lastOn = On;
     }
}
 
void cLCARSNGDisplayChannel::DrawEventRec(const cEvent *Present, const cEvent *Following)
{
  for (int i = 0; i < 2; i++) {
      const cEvent *e = !i ? Present : Following;
      tColor recColor = !i ? Theme.Color(clrChannelSymbolRecBg) : Theme.Color(clrButtonYellowBg);
      int y = !i ? yc03 : yc05;
      if (e) {
         LOCK_TIMERS_READ;
         eTimerMatch TimerMatch = tmNone;
         const cTimer *Timer = Timers->GetMatch(e, &TimerMatch);
         if (Timer && Timer->HasFlags(tfActive) && TimerMatch == tmFull)
            osd->DrawEllipse(xc10m + Gap, y + Gap + Margin, xc10n - Gap - 1, y + lineHeight - Gap - 1, recColor, 0);
         else
            osd->DrawRectangle(xc10m + Gap, y + Margin, xc10n - Gap - 1, y + lineHeight - 1, Theme.Color(clrBackground));
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
  int i = 0;
  int j = 0;
  while (i < min(CountTimers, 3)) {
     int y = yc05 + Margin + i * lineHeight + (i == 2 ? Margin : 0);
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
           int x1 = xc04 + Gap + 2 + smlFont->Width("Do. 00.00. 00:00");
           int x2 = smlFont->Width(" - 00:00");
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
                 osd->DrawText(xc04, y + y1, cString::sprintf("Rec: #%s", *Number), Theme.Color(clrChannelSymbolRecBg), textColorBg, smlFont, x1 - xc04  - x2 - Gap - 1, lineHeight - Gap / 2, taRight | taBorder);
              int w = smlFont->Width(File) + 4; // smlFont width to short
              osd->DrawRectangle(x1, y + y1, xc06k - 1, y + y1 + lineHeight - Gap / 2, Theme.Color(clrBackground));
              osd->DrawText(x1, y + y1, cString::sprintf("%s", File), timerColor, textColorBg, smlFont, min(w, xc06k - x1 - 1), lineHeight - Gap / 2, taLeft | taBorder);
              }
           i++;
           }
        }
     }
}

void cLCARSNGDisplayChannel::SetChannel(const cChannel *Channel, int Number)
{
  DELETENULL(drawDescription);
  int x = xc13;
  DrawRectangleOutline(osd, xc10p, yc11, xc13 - 1, yc12 - 1, frameColorBr, frameColorBg, 15);
  if (Channel && !Channel->GroupSep()) {
     x -= bmRecording.Width() + SymbolSpacing;
     x -= bmEncrypted.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmEncrypted.Height()) / 2, bmEncrypted, Theme.Color(Channel->Ca() ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
     x -= bmDolbyDigital.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmDolbyDigital.Height()) / 2, bmDolbyDigital, Theme.Color(Channel->Dpid(0) ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
     x -= bmAudio.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmAudio.Height()) / 2, bmAudio, Theme.Color(Channel->Apid(1) ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
     if (Channel->Vpid()) {
        x -= bmTeletext.Width() + SymbolSpacing;
        osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmTeletext.Height()) / 2, bmTeletext, Theme.Color(Channel->Tpid() ? clrChannelSymbolOn : clrChannelSymbolOff), frameColorBr);
        }
     else if (Channel->Apid(0)) {
        x -= bmRadio.Width() + SymbolSpacing;
        osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmRadio.Height()) / 2, bmRadio, Theme.Color(clrChannelSymbolOn), frameColorBr);
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
  int w = tallFont->Width(ChName);
  osd->DrawText(xc00m + Margin, yc00 + Margin, ChNumber, frameColorFg, frameColorBg, tallFont, xc02 - xc00m - 2 * Margin, yc02 - yc00 - 2 * Margin, taTop | taRight | taBorder);
  osd->DrawRectangle(xc03, yc00, xc06m - 1, yc02 - 1, Theme.Color(clrBackground));
  osd->DrawText(xc03, yc00, ChName, Theme.Color(clrChannelName), textColorBg, tallFont, min(w, xc06m - xc03 - 1), 0, taTop | taLeft);
  lastSignalDisplay = 0;
  if (withInfo) {
     if (Channel) {
//        int x = xc00 + (yc10 - yc09); // compensate for the arc
        osd->DrawText(xc02, yc11 + Margin, cSource::ToString(Channel->Source()), frameColorFg, frameColorBg, osdFont, xc05 - xc02 - Gap, yc12 - yc11 - 2 * Margin, taRight | taBorder);
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
      int x = xc10n;
      int y = !i ? yc03 : yc05;
      if (e) {
//       draw Time:
         osd->DrawText(xc10 + Margin, y  + Margin, e->GetTimeString(), frameColorFg, frameColorBg, osdFont, xc10m - xc10 - 1 - 2 * Margin, lineHeight, taRight | taBorder);
//       draw Title:
         int w = osdFont->Width(e->Title());
         osd->DrawRectangle(x, y + Margin, xc13, y + Margin + lineHeight, Theme.Color(clrBackground));
         osd->DrawText(x, y + Margin, e->Title(), Theme.Color(clrEventTitle), textColorBg, osdFont, min(w, xc13 - x), lineHeight);
//       draw ShortText:
         w = smlFont->Width(e->ShortText());
         osd->DrawRectangle(x, y + lineHeight + 2 * Margin + Gap / 2, xc13, y + 2 * lineHeight + Gap / 2, Theme.Color(clrBackground));
         osd->DrawText(x, y + lineHeight + 2 * Margin + Gap / 2, e->ShortText(), Theme.Color(clrEventShortText), textColorBg, smlFont, min(w, xc13 - x), lineHeight - 2 * Margin, taBorder);
         }
      else {
         DrawRectangleOutline(osd, xc10, y, xc10m - 1, y + lineHeight - 1 + 2 * Margin, frameColorBr, frameColorBg, 15);
         osd->DrawRectangle(xc10m, y, xc13 - 1, y + 2 * lineHeight + 2 * Margin, Theme.Color(clrBackground));
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
     tColor ColorFg = Theme.Color(clrMessageStatusFg + 2 * Type);
     tColor ColorBg = Theme.Color(clrMessageStatusBg + 2 * Type);
     int x0, x1, y0, y1, y2, lx, ly;
     if (withInfo) {
        x0 = xc06;
        x1 = xc13 - 1;
        y0 = yc11 - ShowSeenExtent;
        y1 = yc11;
        y2 = yc12 - 1;
        }
     else {
        x0 = xc03;
        x1 = xc13 - 1;
        y0 = y1 = yc00;
        y2 = yc02 - 1;
        }
     lx = x1 - x0 - 2 * Margin;
     ly = y2 - y1 - 2 * Margin;
     message = true;
     osd->SaveRegion(x0, y0, x1, y2);
     if (withInfo)
        osd->DrawRectangle(x0, y0, xc06m, y1 - 1, Theme.Color(clrBackground)); // clears the "seen" bar
     DrawRectangleOutline(osd, x0, y1, x1, y2, ColorFg, ColorBg, 15);
     osd->DrawText(x0 + Margin, y1 + Margin, Text, ColorFg, ColorBg, smlFont, lx, ly, taCenter);
     }
  else {
     osd->RestoreRegion();
     message = false;
     }
}

#if APIVERSNUM > 20101
void cLCARSNGDisplayChannel::SetPositioner(const cPositioner *Positioner)
{
  if (Positioner) {
     int y0 = yc0A - (yc0A - yc0B) / 2 + Gap / 2;
     int y1 = yc0A + ShowSeenExtent;
     DrawDevicePosition(osd, Positioner, xc06, y0, xc11, y1, lastCurrentPosition);
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
   if (!message) {
      int volume = statusMonitor->GetVolume();
      if (volume != lastVolume) {
         if (!volumeBox)
            volumeBox = new cLCARSNGVolumeBox(osd, cRect(0, yc11, xc15, yc12 - yc11));
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
