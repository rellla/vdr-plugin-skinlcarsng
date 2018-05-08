/*
 * skinlcars.c: A VDR skin with Star Trek's "LCARS" layout
 *
 * See the main source file 'vdr.c' for copyright information and
 * how to reach the author.
 *
 * $Id: skinlcars.c 4.1 2015/09/01 10:07:07 kls Exp $
 */

#include "lcarsng.h"
#include "displaychannel.h"
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

cBitmap cLCARSNGDisplayChannel::bmTeletext(teletext_xpm);
cBitmap cLCARSNGDisplayChannel::bmRadio(radio_xpm);
cBitmap cLCARSNGDisplayChannel::bmAudio(audio_xpm);
cBitmap cLCARSNGDisplayChannel::bmDolbyDigital(dolbydigital_xpm);
cBitmap cLCARSNGDisplayChannel::bmEncrypted(encrypted_xpm);
cBitmap cLCARSNGDisplayChannel::bmRecording(recording_xpm);

// --- cLCARSNGDisplayChannel ----------------------------------------------

cLCARSNGDisplayChannel::cLCARSNGDisplayChannel(bool WithInfo):cThread("LCARS DisplChan")
{
  tallFont = cFont::CreateFont(Setup.FontOsd, Setup.FontOsdSize * 1.8);
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
  const cFont *font = cFont::GetFont(fontOsd);
  withInfo = WithInfo;
  lineHeight = font->Height();
  tinyFont = CreateTinyFont(lineHeight);
  frameColor = Theme.Color(clrChannelFrameBg);
  message = false;
  lastOn = false;
  On = false;
  int d = 5 * lineHeight;
  xc00 = 0;
  xc01 = xc00 + d / 2;
  xc02 = xc00 + d;
  xc03 = xc02 + lineHeight;
  xc04 = xc02 + d / 4;
  xc05 = xc02 + d;
  xc06 = xc05 + Gap;
  xc15 = cOsd::OsdWidth();
  xc14 = xc15 - lineHeight;
  xc13 = xc14 - Gap;
  xc07 = (xc15 + xc00) / 2;
  xc08 = xc07 + Gap;
  xc09 = xc08 + lineHeight;
  xc10 = xc09 + Gap;
  xc11 = (xc10 + xc13 + Gap) / 2;
  xc12 = xc11 + Gap;

  yc00 = 0;
  yc01 = yc00 + lineHeight;
  yc02 = yc01 + lineHeight;
  yc03 = yc02 + Gap;
  yc04 = yc03 + 2 * lineHeight;
  yc05 = yc04 + Gap;
  yc06 = yc05 + 2 * lineHeight;

  yc07 = yc06 + Gap;
  yc12 = yc07 + 3 * lineHeight + Gap / 2;
  yc11 = yc12 - lineHeight;
  yc10 = yc11 - lineHeight;
  yc09 = yc11 - d / 4;
  yc08 = yc12 - d / 2;

  xs = 0;

  int y1 = withInfo ? yc12 : yc02;
  int y0 = cOsd::OsdTop() + (Setup.ChannelInfoPos ? 0 : cOsd::OsdHeight() - y1);
  osd = CreateOsd(cOsd::OsdLeft(), y0, xc00, yc00, xc15 - 1, y1 - 1);
  osd->DrawRectangle(xc00, yc00, xc15 - 1, y1 - 1, Theme.Color(clrBackground));
  // Rectangles:
  osd->DrawRectangle(xc00, yc00, xc02 - 1, yc02 - 1, frameColor);
  if (withInfo) {
     osd->DrawRectangle(xc00, yc03, xc02 - 1, yc04 - 1, frameColor);
     osd->DrawRectangle(xc00, yc05, xc02 - 1, yc06 - 1, frameColor);
     // Elbow:
     osd->DrawRectangle(xc00, yc07, xc01 - 1, yc08 - 1, frameColor);
     osd->DrawRectangle(xc00, yc08, xc01 - 1, yc12 - 1, clrTransparent);
     osd->DrawEllipse  (xc00, yc08, xc01 - 1, yc12 - 1, frameColor, 3);
     osd->DrawRectangle(xc01, yc07, xc02 - 1, yc12 - 1, frameColor);
     osd->DrawEllipse  (xc02, yc09, xc04 - 1, yc11 - 1, frameColor, -3);
     osd->DrawRectangle(xc02, yc11, xc05 - 1, yc12 - 1, frameColor);
     // Status area:
     osd->DrawRectangle(xc06, yc11 + lineHeight / 2, xc07 - 1, yc12 - 1, frameColor);
     osd->DrawRectangle(xc08, yc11, xc09 - 1, yc12 - 1, frameColor);
     osd->DrawRectangle(xc10, yc11, xc11 - 1, yc12 - 1, Theme.Color(clrDeviceBg));
//     osd->DrawRectangle(xc12, yc11, xc13 - 1, yc12 - 1, Theme.Color(clrDateBg));
     osd->DrawRectangle(xc14, yc11, xc14 + lineHeight / 2 - 1, yc12 - 1, frameColor);
     osd->DrawRectangle(xc14 + lineHeight / 2, yc11 + lineHeight / 2, xc15 - 1, yc12 - 1, clrTransparent);
     osd->DrawEllipse  (xc14 + lineHeight / 2, yc11, xc15 - 1, yc12 - 1, frameColor, 5);
     }
  // Icons:
  osd->DrawRectangle(xc14, yc00, xc14 + lineHeight / 2 - 1, yc01 - 1, frameColor);
  osd->DrawRectangle(xc14 + lineHeight / 2, yc00, xc15 - 1, yc00 + lineHeight / 2 - 1, clrTransparent);
  osd->DrawEllipse  (xc14 + lineHeight / 2, yc00, xc15 - 1, yc01 - 1, frameColor, 5);
}

cLCARSNGDisplayChannel::~cLCARSNGDisplayChannel()
{
  Cancel(-1);
  delete tallFont;
  delete tinyFont;
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayChannel::DrawDate(void)
{
  cString s = DayDateTime();
  if (initial || !*lastDate || strcmp(s, lastDate)) {
     osd->DrawText(xc12, yc00, s, Theme.Color(clrDateFg), Theme.Color(clrDateBg), cFont::GetFont(fontOsd), xc13 - xc12, lineHeight, taRight | taBorder);
     lastDate = s;
     }
}

void cLCARSNGDisplayChannel::DrawTrack(void)
{
  cDevice *Device = cDevice::PrimaryDevice();
  const tTrackId *Track = Device->GetTrack(Device->GetCurrentAudioTrack());
  if (Track ? strcmp(lastTrackId.description, Track->description) : *lastTrackId.description) {
     osd->DrawText(xc03, yc07, Track ? Track->description : "", Theme.Color(clrTrackName), Theme.Color(clrBackground), cFont::GetFont(fontOsd), xc07 - xc03);
     strn0cpy(lastTrackId.description, Track ? Track->description : "", sizeof(lastTrackId.description));
     }
}

void cLCARSNGDisplayChannel::DrawSeen(int Current, int Total)
{
  if (lastCurrentPosition >= 0)
     return; // to not interfere with SetPositioner()
  int Seen = (Total > 0) ? min(xc07 - xc06, int((xc07 - xc06) * double(Current) / Total)) : 0;
  if (initial || Seen != lastSeen) {
     int y0 = yc11 - ShowSeenExtent;
     int y1 = yc11 + lineHeight / 2 - Gap / 2;
     osd->DrawRectangle(xc06, y0, xc06 + Seen - 1, y1 - 1, Theme.Color(clrSeen));
     osd->DrawRectangle(xc06 + Seen, y0, xc07 - 1, y1 - 1, Theme.Color(clrBackground));
     // Restzeit anzeigen
     osd->DrawText(xc00, yc03 + lineHeight, ((Current / 60.0) > 0.1) ? cString::sprintf("-%d", max((int)ceil((Total - Current) / 60.0), 0)) : cString::sprintf(" "), Theme.Color(clrChannelFrameFg), frameColor, cFont::GetFont(fontOsd), xc02 - xc00, 0, taRight | taBorder);
     lastSeen = Seen;
     }
}

void cLCARSNGDisplayChannel::DrawDevice(void)
{
  const cDevice *Device = cDevice::ActualDevice();
  if (DrawDeviceData(osd, Device, xc10, yc11, xc11, yc12, xs, tinyFont, lastDeviceType, lastCamSlot, Device->DeviceNumber() != lastDeviceNumber)) {
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
     DrawDeviceSignal(osd, cDevice::ActualDevice(), xs + lineHeight / 2, yc11, xc11, yc12, lastSignalStrength, lastSignalQuality, initial);
     lastSignalDisplay = Now;
     }
}

void cLCARSNGDisplayChannel::DrawBlinkingRec(void)
{
  bool rec = cRecordControls::Active();
  if (rec) {
     if (!Running())
        Start();
     }
  else {
     if (Running()) {
        Cancel(3);
        On = false;
        }
     }
  if (initial || On != lastOn) {
     int x = xc13;
     x -= bmRecording.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmRecording.Height()) / 2, bmRecording, Theme.Color(rec ? On ? clrChannelSymbolRecFg : clrChannelSymbolOff : clrChannelSymbolOff), rec ? On ? Theme.Color(clrChannelSymbolRecBg) : frameColor : frameColor);
     lastOn = On;
     }
}
 
void cLCARSNGDisplayChannel::DrawEventRec(const cEvent *Present, const cEvent *Following)
{
  for (int i = 0; i < 2; i++) {
      const cEvent *e = !i ? Present : Following;
      tColor recColor = !i ? Theme.Color(clrButtonRedBg) : Theme.Color(clrButtonYellowBg);
      int y = !i ? yc03 : yc05;
      if (e) {
         LOCK_TIMERS_READ;
         eTimerMatch TimerMatch = tmNone;
         const cTimer *Timer = Timers->GetMatch(e, &TimerMatch);
         if (Timer && Timer->HasFlags(tfActive) && TimerMatch == tmFull)
            osd->DrawRectangle(xc02 + 2 * Gap, y, xc03 - 2 * Gap, y + 2 * lineHeight -1, recColor);
         else
            osd->DrawRectangle(xc02 + 2 * Gap, y, xc03 - 2 * Gap, y + 2 * lineHeight -1, Theme.Color(clrBackground));
         }
      }
}

void cLCARSNGDisplayChannel::SetChannel(const cChannel *Channel, int Number)
{
  int x = xc13;
  int xi = x - SymbolSpacing -
           bmRecording.Width() - SymbolSpacing -
           bmEncrypted.Width() - SymbolSpacing -
           bmDolbyDigital.Width() - SymbolSpacing -
           bmAudio.Width() - SymbolSpacing -
           max(bmTeletext.Width(), bmRadio.Width()) - SymbolSpacing;
  osd->DrawRectangle(xc12, yc11, xc13 - 1, yc12 - 1, frameColor);
  if (Channel && !Channel->GroupSep()) {
     x -= bmRecording.Width() + SymbolSpacing;
     x -= bmEncrypted.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmEncrypted.Height()) / 2, bmEncrypted, Theme.Color(Channel->Ca() ? clrChannelSymbolOn : clrChannelSymbolOff), frameColor);
     x -= bmDolbyDigital.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmDolbyDigital.Height()) / 2, bmDolbyDigital, Theme.Color(Channel->Dpid(0) ? clrChannelSymbolOn : clrChannelSymbolOff), frameColor);
     x -= bmAudio.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmAudio.Height()) / 2, bmAudio, Theme.Color(Channel->Apid(1) ? clrChannelSymbolOn : clrChannelSymbolOff), frameColor);
     if (Channel->Vpid()) {
        x -= bmTeletext.Width() + SymbolSpacing;
        osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmTeletext.Height()) / 2, bmTeletext, Theme.Color(Channel->Tpid() ? clrChannelSymbolOn : clrChannelSymbolOff), frameColor);
        }
     else if (Channel->Apid(0)) {
        x -= bmRadio.Width() + SymbolSpacing;
        osd->DrawBitmap(x, yc11 + (yc12 - yc11 - bmRadio.Height()) / 2, bmRadio, Theme.Color(clrChannelSymbolOn), frameColor);
        }
     }
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
  osd->DrawText(xc00, yc00, ChNumber, Theme.Color(clrChannelFrameFg), frameColor, tallFont, xc02 - xc00, yc02 - yc00, taTop | taRight | taBorder);
  osd->DrawText(xc03, yc00, ChName, Theme.Color(clrChannelName), Theme.Color(clrBackground), tallFont, xi - xc03 - lineHeight, 0, taTop | taLeft);
  lastSignalDisplay = 0;
  if (withInfo) {
     if (Channel) {
        int x = xc00 + (yc10 - yc09); // compensate for the arc
        osd->DrawText(x, yc07, cSource::ToString(Channel->Source()), Theme.Color(clrChannelFrameFg), frameColor, cFont::GetFont(fontOsd), xc02 - x, yc10 - yc07, taTop | taRight | taBorder);
        }
     DrawDevice();
     }
}

void cLCARSNGDisplayChannel::SetEvents(const cEvent *Present, const cEvent *Following)
{
  if (!withInfo)
     return;
  if (present != Present)
     lastSeen = -1;
  present = Present;
  following = Following;
  for (int i = 0; i < 2; i++) {
      const cEvent *e = !i ? Present : Following;
      int y = !i ? yc03 : yc05;
      if (e) {
         osd->DrawText(xc00, y, e->GetTimeString(), Theme.Color(clrChannelFrameFg), frameColor, cFont::GetFont(fontOsd), xc02 - xc00, 0, taRight | taBorder);
         osd->DrawText(xc03, y, e->Title(), Theme.Color(clrEventTitle), Theme.Color(clrBackground), cFont::GetFont(fontOsd), xc13 - xc03);
         osd->DrawText(xc03, y + lineHeight, e->ShortText(), Theme.Color(clrEventShortText), Theme.Color(clrBackground), cFont::GetFont(fontSml), xc13 - xc03);
         }
      else {
         osd->DrawRectangle(xc00, y, xc02 - 1, y + lineHeight, frameColor);
         osd->DrawRectangle(xc02, y, xc13 - 1, y + 2 * lineHeight, Theme.Color(clrBackground));
         }
      }
}

void cLCARSNGDisplayChannel::SetMessage(eMessageType Type, const char *Text)
{
  if (Text) {
     int x0, x1, y0, y1, y2;
     if (withInfo) {
        x0 = xc06;
        x1 = xc13;
        y0 = yc11 - ShowSeenExtent;
        y1 = yc11;
        y2 = yc12;
        }
     else {
        x0 = xc03;
        x1 = xc13;
        y0 = y1 = yc00;
        y2 = yc02;
        }
     osd->SaveRegion(x0, y0, x1 - 1, y2 - 1);
     if (withInfo)
        osd->DrawRectangle(xc06, y0, xc07, y1 - 1, Theme.Color(clrBackground)); // clears the "seen" bar
     osd->DrawText(x0, y1, Text, Theme.Color(clrMessageStatusFg + 2 * Type), Theme.Color(clrMessageStatusBg + 2 * Type), cFont::GetFont(fontSml), x1 - x0, y2 - y1, taCenter);
     message = true;
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
     int y0 = yc11 - ShowSeenExtent;
     int y1 = yc11 + lineHeight / 2 - Gap / 2;
     DrawDevicePosition(osd, Positioner, xc06, y0, xc07, y1, lastCurrentPosition);
     }
  else {
     lastCurrentPosition = -1;
     initial = true; // to have DrawSeen() refresh the progress bar
     }
  return;
}
#endif

void cLCARSNGDisplayChannel::Action(void)
{
  while (Running()) {
     On = !On;
     DrawBlinkingRec();
     osd->Flush();
     cCondWait::SleepMs(1000);
  }
}

void cLCARSNGDisplayChannel::Flush(void)
{
  if (withInfo) {
     if (!message) {
        DrawDate();
        DrawTrack();
        DrawDevice();
        DrawSignal();
        DrawBlinkingRec();
        int Current = 0;
        int Total = 0;
        if (present) {
           time_t t = time(NULL);
           if (t > present->StartTime())
              Current = t - present->StartTime();
           Total = present->Duration();
           }
        DrawSeen(Current, Total);
        DrawEventRec(present, following);
        }
     }
  osd->Flush();
  initial = false;
}