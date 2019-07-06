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
  const cFont *font = cFont::GetFont(fontOsd);
  modeOnly = ModeOnly;
  lineHeight = font->Height();
  iconHeight = bmRecording.Height();
  frameColor = Theme.Color(clrReplayFrameBg);
  lastCurrentWidth = 0;
  lastTotalWidth = 0;
  memset(&lastTrackId, 0, sizeof(lastTrackId));
  initial = true;
  lastOn = false;
  On = false;
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

  yp00 = lineHeight;
  yp01 = yp00 + 2 * lineHeight;
  yp02 = yp01 + Gap;
  yp03 = yp02 + 2 * lineHeight;

  yp04 = yp03 + Gap;
  yp09 = yp04 + 3 * lineHeight + Gap / 2;
  yp08 = yp09 - max(lineHeight, iconHeight);
  yp07 = yp08 - lineHeight;
  yp06 = yp08 - d / 4;
  yp05 = yp09 - d / 2;

  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - yp09, xp00, yp00 - lineHeight, xp15 - 1, yp09 - 1);
  osd->DrawRectangle(xp00, yp00, xp15 - 1, yp09 - 1, modeOnly ? clrTransparent : Theme.Color(clrBackground));
  // Rectangles:
  if (!modeOnly)
     osd->DrawRectangle(xp00, yp00, xp02 - 1, yp01 - 1, frameColor);
  osd->DrawRectangle(xp00, yp02, xp02 - 1, yp03 - 1, frameColor);
  if (!modeOnly) {
     // Elbow:
     osd->DrawRectangle(xp00, yp04, xp01 - 1, yp05 - 1, frameColor);
     osd->DrawRectangle(xp00, yp05, xp01 - 1, yp09 - 1, clrTransparent);
     osd->DrawEllipse  (xp00, yp05, xp01 - 1, yp09 - 1, frameColor, 3);
     osd->DrawRectangle(xp01, yp04, xp02 - 1, yp09 - 1, frameColor);
     osd->DrawEllipse  (xp02, yp06, xp04 - 1, yp08 - 1, frameColor, -3);
     osd->DrawRectangle(xp02, yp08, xp05 - 1, yp09 - 1, frameColor);
     // Status area:
     osd->DrawRectangle(xp06, yp08, xp07 - 1, yp09 - 1, frameColor);
     osd->DrawRectangle(xp08, yp08, xp09 - 1, yp09 - 1, frameColor);
     osd->DrawRectangle(xp10, yp08, xp11 - 1, yp09 - 1, frameColor);
     osd->DrawRectangle(xp12, yp08, xp13 - 1, yp09 - 1, frameColor);
     osd->DrawRectangle(xp14, yp08, xp14 + lineHeight / 2 - 1, yp09 - 1, frameColor);
     osd->DrawRectangle(xp14 + lineHeight / 2, yp08 + lineHeight / 2, xp15 - 1, yp09 - 1, clrTransparent);
     osd->DrawEllipse  (xp14 + lineHeight / 2, yp08, xp15 - 1, yp09 - 1, frameColor, 5);
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
     osd->DrawText(xp12, yp00 - lineHeight, s, Theme.Color(clrDateFg), Theme.Color(clrDateBg), cFont::GetFont(fontOsd), xp13 - xp12, lineHeight, taRight | taBorder);
     lastDate = s;
     }
}

void cLCARSNGDisplayReplay::DrawTrack(void)
{
  cDevice *Device = cDevice::PrimaryDevice();
  const tTrackId *Track = Device->GetTrack(Device->GetCurrentAudioTrack());
  if (Track ? strcmp(lastTrackId.description, Track->description) : *lastTrackId.description) {
     osd->DrawText(xp03, yp04, Track ? Track->description : "", Theme.Color(clrTrackName), Theme.Color(clrBackground), cFont::GetFont(fontOsd), xp07 - xp03);
     strn0cpy(lastTrackId.description, Track ? Track->description : "", sizeof(lastTrackId.description));
     }
}

void cLCARSNGDisplayReplay::DrawBlinkingRec(void)
{ 
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
     osd->DrawBitmap(x, yp08 + (yp09 - yp08 - bmRecording.Height()) / 2, bmRecording, Theme.Color(rec ? On ? clrChannelSymbolRecFg : clrChannelSymbolOff : clrChannelSymbolOff), rec ? On ? Theme.Color(clrChannelSymbolRecBg) : frameColor : frameColor);
     lastOn = On;
     }
}

void cLCARSNGDisplayReplay::SetRecording(const cRecording *Recording)
{
  const cRecordingInfo *RecordingInfo = Recording->Info();
  int x = xp13;

  osd->DrawRectangle(xp12, yp08, xp13 - 1, yp09 - 1, frameColor);
  x -= bmRecording.Width() + SymbolSpacing;
 
  SetTitle(RecordingInfo->Title());
  osd->DrawText(xp03, yp01 - lineHeight, RecordingInfo->ShortText(), Theme.Color(clrEventShortText), Theme.Color(clrBackground), cFont::GetFont(fontSml), xp13 - xp03);
  osd->DrawText(xp00, yp00, ShortDateString(Recording->Start()), Theme.Color(clrReplayFrameFg), frameColor, cFont::GetFont(fontOsd), xp02 - xp00, 0, taTop | taRight | taBorder);
  osd->DrawText(xp00, yp01 - lineHeight, TimeString(Recording->Start()), Theme.Color(clrReplayFrameFg), frameColor, cFont::GetFont(fontOsd), xp02 - xp00, 0, taBottom | taRight | taBorder);
}

void cLCARSNGDisplayReplay::SetTitle(const char *Title)
{
  osd->DrawText(xp03, yp00, Title, Theme.Color(clrEventTitle), Theme.Color(clrBackground), cFont::GetFont(fontOsd), xp13 - xp03);
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
  osd->DrawBitmap(xp01 - bm.Width() / 2, (yp02 + yp03 - bm.Height()) / 2, bm, Theme.Color(clrReplayFrameFg), frameColor);
}

void cLCARSNGDisplayReplay::SetProgress(int Current, int Total)
{
  cProgressBar pb(xp13 - xp03, lineHeight, Current, Total, marks, Theme.Color(clrReplayProgressSeen), Theme.Color(clrReplayProgressRest), Theme.Color(clrReplayProgressSelected), Theme.Color(clrReplayProgressMark), Theme.Color(clrReplayProgressCurrent));
  osd->DrawBitmap(xp03, yp02, pb);
}

void cLCARSNGDisplayReplay::SetCurrent(const char *Current)
{
  const cFont *font = cFont::GetFont(fontOsd);
  int w = font->Width(Current);
  osd->DrawText(xp03, yp03 - lineHeight, Current, Theme.Color(clrReplayPosition), Theme.Color(clrBackground), font, max(lastCurrentWidth, w), 0, taLeft);
  lastCurrentWidth = w;
}

void cLCARSNGDisplayReplay::SetTotal(const char *Total)
{
  const cFont *font = cFont::GetFont(fontOsd);
  int w = font->Width(Total);
  osd->DrawText(xp13 - w, yp03 - lineHeight, Total, Theme.Color(clrReplayPosition), Theme.Color(clrBackground), font, max(lastTotalWidth, w), 0, taRight);
  lastTotalWidth = w;
}

void cLCARSNGDisplayReplay::SetJump(const char *Jump)
{
  osd->DrawText(xp06, yp08, Jump, Theme.Color(clrReplayJumpFg), Jump ? Theme.Color(clrReplayJumpBg) : frameColor, cFont::GetFont(fontOsd), xp07 - xp06, 0, taCenter);
}

void cLCARSNGDisplayReplay::SetMessage(eMessageType Type, const char *Text)
{
  if (Text) {
     osd->SaveRegion(xp06, yp08, xp13 - 1, yp09 - 1);
     osd->DrawText(xp06, yp08, Text, Theme.Color(clrMessageStatusFg + 2 * Type), Theme.Color(clrMessageStatusBg + 2 * Type), cFont::GetFont(fontSml), xp13 - xp06, yp09 - yp08, taCenter);
     }
  else
     osd->RestoreRegion();
}

void cLCARSNGDisplayReplay::Action(void)
{
  int i = 0;

  while (Running()) {
     i++;
     if (i > 9) {
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
     DrawBlinkingRec();
     }
  osd->Flush();
  initial = false;
}
