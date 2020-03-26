/*
 * skinlcars.c: A VDR skin with Star Trek's "LCARS" layout
 *
 * See the main source file 'vdr.c' for copyright information and
 * how to reach the author.
 *
 * $Id: skinlcars.c 4.1 2015/09/01 10:07:07 kls Exp $
 */

// "Star Trek: The Next Generation"(R) is a registered trademark of Paramount Pictures,
// registered in the United States Patent and Trademark Office, all rights reserved.
// The LCARS system is based upon the designs of Michael Okuda and his Okudagrams.
//
// "LCARS" is short for "Library Computer Access and Retrieval System".
// Some resources used for writing this skin can be found at
// http://www.lcars.org.uk
// http://www.lcarsdeveloper.com
// http://www.lcarscom.net
// http://lds-jedi.deviantart.com/art/LCARS-Swept-Tutorial-213936938
// http://lds-jedi.deviantart.com/art/LCARS-Button-Tutorial-210783437
// http://zelldenver.deviantart.com/art/LCARS-Color-Standard-179565780
// http://www.lcars47.com
// http://www.bracercom.com/tutorial/content/CoherentLCARSInterface/LCARSCoherentInterface.html
// http://www.bracercom.com/tutorial/content/lcars_manifesto/the_lcars_manifesto.html

#include "lcarsng.h"
#include "displayreplay.h"
#include "config.h"
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

cBitmap cLCARSNGDisplayReplay::bmTeletext(teletext_xpm);
cBitmap cLCARSNGDisplayReplay::bmRadio(radio_xpm);
cBitmap cLCARSNGDisplayReplay::bmAudio(audio_xpm);
cBitmap cLCARSNGDisplayReplay::bmDolbyDigital(dolbydigital_xpm);
cBitmap cLCARSNGDisplayReplay::bmEncrypted(encrypted_xpm);
cBitmap cLCARSNGDisplayReplay::bmRecording(recording_xpm);

// --- cLCARSNGDisplayReplay -----------------------------------------------

cLCARSNGDisplayReplay::cLCARSNGDisplayReplay(bool ModeOnly):cThread("LCARS DisplRepl")
{
  font = cFont::GetFont(fontOsd);
  modeOnly = ModeOnly;
  lineHeight = font->Height();
  iconHeight = bmRecording.Height();
  frameColorBg = Theme.Color(clrReplayFrameBg);
  frameColorFg = Theme.Color(clrReplayFrameFg);
  frameColorBr = (Theme.Color(clrReplayFrameBr) == CLR_BLACK) ? frameColorBg : Theme.Color(clrReplayFrameBr);
  textColorBg = Theme.Color(clrReplayTextBg);
  Margin = Config.Margin;
  lastCurrentWidth = 0;
  lastTotalWidth = 0;
  memset(&lastTrackId, 0, sizeof(lastTrackId));
  initial = true;
  lastOn = false;
  On = false;
  message = false;
  isRecording = false;
  timshiftMode = false;
  fps = DEFAULTFRAMESPERSECOND;
  framesTotal = 0;
  pbinit = true;
  int d = 5 * lineHeight;
  xp00 = 0;
  xp01 = xp00 + d / 2;
  xp02 = xp00 + d;
  xp03 = xp02 + lineHeight;
  xp04 = xp02 + d / 4;
  xp05 = xp02 + d;
  xp06 = xp05 + Gap;
  xp15 = cOsd::OsdWidth();
  xp14 = xp15 - lineHeight;
  xp13 = xp14 - Gap;
  xp07 = (xp15 + xp00) / 2;
  xp08 = xp07 + Gap;
  xp09 = xp08 + lineHeight;
  xp10 = xp09 + Gap;
  xp11 = (xp10 + xp13 + Gap) / 2;
  xp12 = xp11 + Gap;

  yp00A = 0;
  yp00 = max(lineHeight, iconHeight) + 2 * Margin;
  yp01 = yp00 + 2 * lineHeight + 2 * Margin;
  yp02 = yp01 + Gap;
  yp03 = yp02 + 2 * lineHeight + 2 * Margin;
  yp04 = yp03 + Gap;
  yp09 = yp04 + 3 * lineHeight + Gap / 2 + 2 * Margin;
  yp08 = yp09 - max(lineHeight, iconHeight) - 2 * Margin;
  yp07 = yp08 - lineHeight;
  yp06 = yp08 - d / 4;
  yp05 = yp09 - d / 2;

  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - yp09, xp00, yp00A, xp15 - 1, yp09 - 1);
  osd->DrawRectangle(xp00, yp00, xp15 - 1, yp09 - 1, modeOnly ? clrTransparent : Theme.Color(clrBackground));
  // Rectangles:
  if (!modeOnly) {
     DrawRectangleOutline(osd, xp00, yp00, xp02 - 1, yp01 - 1, frameColorBr, frameColorBg, 15);
     }
  DrawRectangleOutline(osd, xp00, yp02, xp02 - 1, yp03 - 1, frameColorBr, frameColorBg, 15);
  if (!modeOnly) {
     // Elbow:
     DrawRectangleOutline(osd, xp00, yp04, xp02 - 1, yp08 - 1, frameColorBr, frameColorBg, 7);
     DrawRectangleOutline(osd, xp00, yp08, xp02 - 1, yp09 - 1, frameColorBr, frameColorBg, 8);
     DrawRectangleOutline(osd, xp02, yp08, xp05 - 1, yp09 - 1, frameColorBr, frameColorBg, 14);
     osd->DrawRectangle(xp00, yp05, xp01 - 1, yp09 - 1, clrTransparent);
     osd->DrawEllipse  (xp00, yp05, xp01 - 1, yp09 - 1, frameColorBr, 3);
     osd->DrawEllipse  (xp00 + Margin, yp05, xp01 - 1, yp09 - 1 - Margin, frameColorBg, 3);
     osd->DrawEllipse  (xp02 - 1, yp06, xp04 - 1, yp08, frameColorBr, -3);
     osd->DrawEllipse  (xp02 - 1 - Margin, yp06, xp04 - 1, yp08 + Margin, frameColorBg, -3);
     // Status area:
     DrawRectangleOutline(osd, xp06, yp08, xp07 - 1, yp09 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xp08, yp08, xp09 - 1, yp09 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xp10, yp08, xp11 - 1, yp09 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xp12, yp08, xp13 - 1, yp09 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xp14, yp08, xp14 + lineHeight / 2 - 1, yp09 - 1, frameColorBr, frameColorBg, 11);
     osd->DrawRectangle(xp14 + lineHeight / 2, yp08 + lineHeight / 2, xp15 - 1, yp09 - 1, clrTransparent);
     osd->DrawEllipse  (xp14 + lineHeight / 2, yp08, xp15 - 1, yp09 - 1, frameColorBr, 5);
     osd->DrawEllipse  (xp14 + lineHeight / 2, yp08 + Margin, xp15 - 1 - Margin, yp09 - 1 - Margin, frameColorBg, 5);
     // Upper Right:
     osd->DrawRectangle(xp13, yp00A, xp15 - 1, yp00 - 1, Theme.Color(clrBackground));
     DrawRectangleOutline(osd, xp14, yp00A, xp14 + lineHeight / 2 - 1, yp00 - 1, frameColorBr, frameColorBg, 11);
     osd->DrawRectangle(xp14 + lineHeight / 2, yp00A, xp15 - 1, yp00 - lineHeight / 2 - 1, clrTransparent);
     osd->DrawEllipse  (xp14 + lineHeight / 2, yp00A, xp15 - 1, yp00 - 1, frameColorBr, 5);
     osd->DrawEllipse  (xp14 + lineHeight / 2, yp00A + Margin, xp15 - 1 - Margin, yp00 - 1 - Margin, frameColorBg, 5);
     // Progressbar
     DrawRectangleOutline(osd, xp03, yp03 - lineHeight, xp13 - 1, yp03 - 1, frameColorBr, frameColorBg, 15);
     }
}

cLCARSNGDisplayReplay::~cLCARSNGDisplayReplay()
{
  Cancel(3);
  delete osd;
}

void cLCARSNGDisplayReplay::DrawDate(void)
{
  cString s = DayDateTime();
  if (!*lastDate || strcmp(s, lastDate)) {
     osd->DrawText(xp12, yp00A, s, Theme.Color(clrDateFg), Theme.Color(clrDateBg), font, xp13 - xp12 - Margin - 1, yp00, taRight | taBorder);
     lastDate = s;
     }
}

void cLCARSNGDisplayReplay::DrawTrack(void)
{
  cDevice *Device = cDevice::PrimaryDevice();
  const tTrackId *Track = Device->GetTrack(Device->GetCurrentAudioTrack());
  if (Track ? strcmp(lastTrackId.description, Track->description) : *lastTrackId.description) {
     osd->DrawText(xp10 + Margin, yp08 + Margin, Track ? Track->description : "", Theme.Color(clrTrackName), clrTransparent, font, xp11 - xp10 - 2 * Margin, yp09 - yp08 - 2 * Margin, taRight | taBorder);
     strn0cpy(lastTrackId.description, Track ? Track->description : "", sizeof(lastTrackId.description));
     }
}

void cLCARSNGDisplayReplay::DrawBlinkingRec(void)
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
     if (Running())
        Cancel(3);
     On = false;
     }
  if (initial || On != lastOn) { 
     int x = xp13;
     x -= bmRecording.Width() + SymbolSpacing;
     osd->DrawBitmap(x - Margin, yp08 + (yp09 - yp08 - bmRecording.Height()) / 2, bmRecording, Theme.Color(rec ? On ? clrChannelSymbolRecFg : clrChannelSymbolOff : clrChannelSymbolOff), rec ? On ? Theme.Color(clrChannelSymbolRecBg) : frameColorBr : frameColorBr);
     lastOn = On;
     }
}

void cLCARSNGDisplayReplay::SetRecording(const cRecording *Recording)
{
  const cRecordingInfo *RecordingInfo = Recording->Info();
  if (!RecordingInfo)
     return;
 
  SetTitle(RecordingInfo->Title());
  const cFont *fontsml = cFont::GetFont(fontSml);
  int w = fontsml->Width(RecordingInfo->ShortText());
  osd->DrawText(xp03, yp01 - lineHeight - Margin, RecordingInfo->ShortText(), Theme.Color(clrEventShortText), textColorBg, fontsml, min(xp13, (xp03 + w)) - xp03, lineHeight, taCenter);
  osd->DrawText(xp00 + Margin, yp00 + Margin, ShortDateString(Recording->Start()), frameColorFg, frameColorBg, font, xp02 - xp00 - 2 * Margin, lineHeight, taTop | taRight | taBorder);
  osd->DrawText(xp00 + Margin, yp01 - lineHeight - Margin, TimeString(Recording->Start()), frameColorFg, frameColorBg, font, xp02 - xp00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);

  //check for instant recording
  const char *recName = Recording->Name();
  int usage = Recording->IsInUse();
  if (usage & ruTimer)
     isRecording = true;
  if ((recName && *recName == '@') && (isRecording)) {
     timshiftMode = true;
     return;
  }
  if (!isRecording)
     return;
  const cEvent *Event = RecordingInfo->GetEvent();
  if (!Event)
     return;
  fps = Recording->FramesPerSecond();
  time_t liveEventStop = Event->EndTime();
  time_t recordingStart = time(0) - Recording->LengthInSeconds();
  framesTotal = (liveEventStop - recordingStart) * fps;
  endTime = cString::sprintf("%s: %s", tr("length"), *IndexToHMSF(framesTotal, false, fps));
}

void cLCARSNGDisplayReplay::SetTitle(const char *Title)
{
  int w = font->Width(Title);
  osd->DrawText(xp03, yp00 + Margin, Title, Theme.Color(clrEventTitle), textColorBg, font, min(xp13, (xp03 + w)) - xp03, lineHeight);
}

static const char *const *ReplaySymbols[2][2][5] = {
  { { pause_xpm, srew_xpm, srew1_xpm, srew2_xpm, srew3_xpm },
    { pause_xpm, sfwd_xpm, sfwd1_xpm, sfwd2_xpm, sfwd3_xpm }, },
  { { play_xpm,  frew_xpm, frew1_xpm, frew2_xpm, frew3_xpm },
    { play_xpm,  ffwd_xpm, ffwd1_xpm, ffwd2_xpm, ffwd3_xpm } }
  };

void cLCARSNGDisplayReplay::SetMode(bool Play, bool Forward, int Speed)
{
  Speed = constrain(Speed, -1, 3);
  cBitmap bm(ReplaySymbols[Play][Forward][Speed + 1]);
  osd->DrawBitmap(xp01 - bm.Width() / 2, (yp02 + yp03 - bm.Height()) / 2, bm, frameColorFg, frameColorBg);
}

void cLCARSNGDisplayReplay::SetProgress(int Current, int Total)
{
  int x = 0;
  int lH = lineHeight / 4;
  if (timshiftMode) {
     cString tM = "TimeshiftMode";
     int w = font->Width(tM);
     osd->DrawText(xp13 - w, yp01, tM, Theme.Color(clrReplayPosition), textColorBg, font, w, 0, taRight);
  }
  if (isRecording) {
     int w = font->Width(endTime);
     if (Total > framesTotal) {
        osd->DrawRectangle(xp13 - w, yp01, xp13 - 1, yp03 - lineHeight - 1, Theme.Color(clrBackground)); // Clear endTime
        isRecording = false;
        x = 0;
     }
     else {
        double rest = ((double)framesTotal - (double)Total) / (double)framesTotal;
        x = (int)((xp13 - xp03) * rest);
        osd->DrawText(xp13 - w, yp01, *endTime, Theme.Color(clrReplayPosition), textColorBg, font, w, 0, taRight);
     }
     if (pbinit) {
        osd->DrawRectangle(xp03 + Margin, yp03 - lineHeight + 4 + lH, xp13 - Margin, yp03 - 4 - lH, frameColorBr); // small rectangle
        pbinit = false;
     }
  }
  if ((xp13 - xp03 - x - 5) > 0) {
     cProgressBar pb(xp13 - xp03 - x - 2 * Margin, lineHeight - 2 * Margin, Current, Total, marks, Theme.Color(clrReplayProgressSeen), Theme.Color(clrReplayProgressRest), Theme.Color(clrReplayProgressSelected), Theme.Color(clrReplayProgressMark), Theme.Color(clrReplayProgressCurrent));
     osd->DrawBitmap(xp03 + Margin, yp03 - lineHeight + Margin, pb); // Progressbar
  }
}

void cLCARSNGDisplayReplay::SetCurrent(const char *Current)
{
  int w = font->Width(Current);
  osd->DrawText(xp03, yp04, Current, Theme.Color(clrReplayPosition), textColorBg, font, max(lastCurrentWidth, w), 0, taTop | taLeft);
  if (lastCurrentWidth > w)
     osd->DrawRectangle(xp03 + lastCurrentWidth - (lastCurrentWidth - w), yp04, xp03 + lastCurrentWidth, yp04 + lineHeight, Theme.Color(clrBackground));
  lastCurrentWidth = w;
}

void cLCARSNGDisplayReplay::SetTotal(const char *Total)
{
  int w = font->Width(Total);
  osd->DrawText(xp13 - w, yp04, Total, Theme.Color(clrReplayPosition), textColorBg, font, max(lastTotalWidth, w), 0, taTop | taRight);
  lastTotalWidth = w;
}

void cLCARSNGDisplayReplay::SetJump(const char *Jump)
{
  if (!Jump)
     osd->DrawRectangle(xp06 + Margin, yp08 + Margin, xp07 - 1 - Margin, yp09 - 1 - Margin, frameColorBg);
  osd->DrawText(xp06 + Margin, yp08 + Margin, Jump, Theme.Color(clrReplayJumpFg), Jump ? Theme.Color(clrReplayJumpBg) : frameColorBg, font, xp07 - xp06 - 1 - 2 * Margin, yp09 - yp08 - 1 - 2 * Margin, taCenter);
}

void cLCARSNGDisplayReplay::SetMessage(eMessageType Type, const char *Text)
{
  if (Text) {
     tColor ColorFg = Theme.Color(clrMessageStatusFg + 2 * Type);
     tColor ColorBg = Theme.Color(clrMessageStatusBg + 2 * Type);
     int x0, x1, y0, y1, lx, ly;
     x0 = xp06;
     x1 = xp13 - 1;
     y0 = yp08;
     y1 = yp09 - 1;
     lx = x1 - x0 - 2 * Margin;
     ly = y1 - y0 - 2 * Margin;
     message = true;
     osd->SaveRegion(x0, y0, x1, y1);
     DrawRectangleOutline(osd, x0, y0, x1, y1, ColorFg, ColorBg, 15);
     osd->DrawText(x0 + Margin, y0 + Margin, Text, ColorFg, ColorBg, cFont::GetFont(fontSml), lx, ly, taCenter);
     }
  else {
     osd->RestoreRegion();
     message = false;
     }
}

void cLCARSNGDisplayReplay::Action(void)
{
  int i = 0;

  while (Running()) {
     i++;
     if (message || i > 9) {
        i = 0;
        On = !On;
        DrawBlinkingRec();
        if (osd) osd->Flush();
        }
     cCondWait::SleepMs(100);
  }
}

void cLCARSNGDisplayReplay::Flush(void)
{
  if (!modeOnly) {
     DrawDate();
     DrawTrack();
     if (initial)
        DrawBlinkingRec();
     }
  osd->Flush();
  initial = false;
}
