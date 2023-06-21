#include "config.h"
#include "lcarsng.h"
#include "displayreplay.h"
#include "status.h"

cBitmap cLCARSNGDisplayReplay::bmTeletext(teletext_xpm);
cBitmap cLCARSNGDisplayReplay::bmRadio(radio_xpm);
cBitmap cLCARSNGDisplayReplay::bmAudio(audio_xpm);
cBitmap cLCARSNGDisplayReplay::bmDolbyDigital(dolbydigital_xpm);
cBitmap cLCARSNGDisplayReplay::bmEncrypted(encrypted_xpm);
cBitmap cLCARSNGDisplayReplay::bmRecording(recording_xpm);

// --- cLCARSNGDisplayReplay -----------------------------------------------

cLCARSNGDisplayReplay::cLCARSNGDisplayReplay(bool ModeOnly):cThread("LCARS DisplRepl")
{
  osdFont = cFont::GetFont(fontOsd);
  smlFont = cFont::GetFont(fontSml);
  modeOnly = ModeOnly;
  lineHeight = osdFont->Height();
  smlLineHeight = smlFont->Height();
  iconHeight = bmRecording.Height();
  frameColorBg = Theme.Color(clrReplayFrameBg);
  frameColorFg = Theme.Color(clrReplayFrameFg);
  frameColorBr = (Theme.Color(clrReplayFrameBr) == CLR_BLACK) ? frameColorBg : Theme.Color(clrReplayFrameBr);
  textColorBg = Theme.Color(clrReplayTextBg);
  Margin = Config.Margin;
  lastCurrentWidth = 0;
  lastTotalWidth = 0;
  lastRestWidth = 0;
  memset(&lastTrackId, 0, sizeof(lastTrackId));
  textBorder = lineHeight * TEXT_ALIGN_BORDER / 100;
  zoom = 0;
  initial = true;
  lastOn = false;
  On = false;
  message = false;
  isRecording = false;
  timshiftMode = false;
  fps = DEFAULTFRAMESPERSECOND;
  framesTotal = 0;
  pbinit = true;
  volumeBox = NULL;
  lastVolume = statusMonitor->GetVolume();
  lastVolumeTime = time(NULL);
  oldResolution = "";

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

  int d = 5 * lineHeight;

  xp00 = 0;                                        // Left
  xp01 = xp00 + d / 2;
  xp02 = xp00 + d;
  xp03 = xp02 + lineHeight;
  xp04 = xp02 + d / 4;
  xp05 = xp02 + ((zoom > 1) ? 0.5 : 1) * d;
  xp06 = xp05 + Gap;
  xp17 = cOsd::OsdWidth();                         // Right
  xp16 = xp17 - lineHeight - 2 * Margin;
  xp15 = xp16 - Gap;
  xp14 = xp15 - ((zoom > 1) ? 1.5 : 2) * d;
  xp13 = xp14 - Gap;
  xp07 = (xp17 + xp00) / 2;
  xp08 = xp07 + Gap;
  xp09 = xp08 + lineHeight;
  xp10 = xp09 + Gap;
  xp11 = (xp10 + xp15 + Gap) / 2;
  xp12 = xp11 + Gap;

  int h = max(lineHeight, iconHeight);

  yp00 = 0;                                        // Top
  yp01 = h + 2 * Margin;
  yp02 = yp01 + lineHeight / 2;
  yp03 = yp01 + d / 4;
  yp04 = yp00 + d / 2;
  yp05 = yp02 + 2 * lineHeight + 2 * Margin;
  yp06 = yp05 + Gap;
  yp07 = yp06 + 2 * lineHeight + 2 * Margin;
  yp08 = yp07 + Gap;
  yp12 = yp08 + h + 1.5 * lineHeight + 2 * Margin; // Bottom
  yp11 = yp12 - h - 2 * Margin;
  yp10 = yp11 - d / 4;
  yp09 = yp12 - d / 2;

  // message and volume box
  xv00 = (modeOnly) ? xp00 : xp06;
  xv01 = (modeOnly) ? xp17 : xp13;
  yv00 = yp00;
  yv01 = yp01;

  leftIcons = 0;

  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - yp12, xp00, yp00, xp17 - 1, yp12 - 1);
  osd->DrawRectangle(xp00, yp00, xp17 - 1, yp12 - 1, modeOnly ? clrTransparent : Theme.Color(clrBackground));
  // Rectangles:
  DrawRectangleOutline(osd, xp00, yp06, xp02 - 1, yp07 - 1, frameColorBr, frameColorBg, 15);
  if (!modeOnly) {
     // Top middle
     DrawRectangleOutline(osd, xp06, yp00, xp13 - 1, yp01 / 2 - 1, frameColorBr, frameColorBg, 15);
     // Upper Elbow:
     DrawRectangleOutline(osd, xp00, yp00, xp02 - 1, yp03 - 1, frameColorBr, frameColorBg, 3); 
     DrawRectangleOutline(osd, xp00, yp01, xp02 - 1, yp05 - 1, frameColorBr, frameColorBg, 13);
     DrawRectangleOutline(osd, xp02, yp00, xp05 - 1, yp01 - 1, frameColorBr, frameColorBg, 14);
     osd->DrawRectangle(xp00, yp00, xp01 - 1, yp04 - 1, clrTransparent);
     DrawEllipseOutline(osd, xp00, yp00, xp01 - 1, yp04 - 1, frameColorBr, frameColorBg, 2);
     DrawEllipseOutline(osd, xp02, yp01, xp04 - 1, yp03 - 1, frameColorBr, frameColorBg, -2);
     // Lower Elbow:
     DrawRectangleOutline(osd, xp00, yp08, xp02 - 1, yp11 - 1, frameColorBr, frameColorBg, 7);
     DrawRectangleOutline(osd, xp00, yp11, xp02 - 1, yp12 - 1, frameColorBr, frameColorBg, 8);
     DrawRectangleOutline(osd, xp02, yp11, xp05 - 1, yp12 - 1, frameColorBr, frameColorBg, 14);
     osd->DrawRectangle(xp00, yp09, xp01 - 1, yp12 - 1, clrTransparent);
     DrawEllipseOutline(osd, xp00, yp09, xp01 - 1, yp12 - 1, frameColorBr, frameColorBg, 3);
     DrawEllipseOutline(osd, xp02, yp10, xp04 - 1, yp11 - 1, frameColorBr, frameColorBg, -3);
     // Status area:
     DrawRectangleOutline(osd, xp06, yp11, xp07 - 1, yp12 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xp08, yp11, xp09 - 1, yp12 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xp10, yp11, xp11 - 1, yp12 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xp12, yp11, xp15 - 1, yp12 - 1, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(osd, xp16, yp11, xp16 + lineHeight / 2 + Margin - 1, yp12 - 1, frameColorBr, frameColorBg, 11);
     osd->DrawRectangle(xp16 + lineHeight / 2 + Margin, yp11 + lineHeight / 2, xp17 - 1, yp12 - 1, clrTransparent);
     DrawEllipseOutline(osd, xp16 + lineHeight / 2 + Margin, yp11, xp17 - 1, yp12 - 1, frameColorBr, frameColorBg, 5);
     // Upper Right:
     DrawRectangleOutline(osd, xp16, yp00, xp16 + lineHeight / 2 + Margin - 1, yp01 - 1, frameColorBr, frameColorBg, 11);
     osd->DrawRectangle(xp16 + lineHeight / 2 + Margin, yp00, xp17 - 1, yp01 - lineHeight / 2 - 1, clrTransparent);
     DrawEllipseOutline(osd, xp16 + lineHeight / 2 + Margin, yp00, xp17 - 1, yp01 - 1, frameColorBr, frameColorBg, 5);
     // Progressbar
     DrawRectangleOutline(osd, xp03, yp07 - lineHeight, xp15 - 1, yp07 - 1, frameColorBr, frameColorBg, 15);
     }
}

cLCARSNGDisplayReplay::~cLCARSNGDisplayReplay()
{
  Cancel(3);
  delete messageBox;
  delete volumeBox;
  delete osd;
}

void cLCARSNGDisplayReplay::DrawDate(void)
{
  cString s = DayDateTime();
  if (!*lastDate || strcmp(s, lastDate)) {
     osd->DrawText(xp14, yp00, s, Theme.Color(clrDateFg), Theme.Color(clrDateBg), osdFont, xp15 - xp14 - Margin - 1, yp01, taRight | taBorder);
     lastDate = s;
     }
}

void cLCARSNGDisplayReplay::DrawTrack(void)
{
  cDevice *Device = cDevice::PrimaryDevice();
  const tTrackId *Track = Device->GetTrack(Device->GetCurrentAudioTrack());
  if (Track ? strcmp(lastTrackId.description, Track->description) : *lastTrackId.description) {
     osd->DrawText(xp10 + Margin, yp11 + Margin, Track ? Track->description : "", Theme.Color(clrTrackName), clrTransparent, (zoom) ? smlFont : osdFont, xp11 - xp10 - textBorder - 2 * Margin, yp12 - yp11 - 2 * Margin, taCenter | taRight | taBorder);
     strn0cpy(lastTrackId.description, Track ? Track->description : "", sizeof(lastTrackId.description));
     }
}

void cLCARSNGDisplayReplay::DrawScreenResolution(void)
{
  cString resolution = GetScreenResolutionIcon();
  if (!(strcmp(resolution, oldResolution) == 0)) {
     if (strcmp(resolution, "") == 0) {
        osd->DrawRectangle(xp11 + Margin, yp11 + Margin, leftIcons, yp12 - Margin, frameColorBg);
        }
     const cFont *font = (zoom) ? smlFont : osdFont;
     int w = font->Width(*resolution) + 2 * textBorder;
     int x = leftIcons - w - SymbolSpacing;
     int h = ((zoom) && (iconHeight < lineHeight)) ? (lineHeight - iconHeight) / 2 : 0;
     osd->DrawText(x, yp11 + h + Margin, cString::sprintf("%s", *resolution), Theme.Color(clrChannelSymbolOn), frameColorBr, font, w, yp12 - yp11 - 2 * Margin - 2 * h, taCenter | taRight | taBorder);
     oldResolution = resolution;
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
     int x = xp15;
     x -= bmRecording.Width() + SymbolSpacing;
     osd->DrawBitmap(x, yp11 + (yp12 - yp11 - bmRecording.Height()) / 2, bmRecording, Theme.Color(rec ? On ? clrChannelSymbolRecFg : clrChannelSymbolOff : clrChannelSymbolOff), rec ? On ? Theme.Color(clrChannelSymbolRecBg) : frameColorBr : frameColorBr);
     lastOn = On;
     leftIcons = x;
     }
}

void cLCARSNGDisplayReplay::SetRecording(const cRecording *Recording)
{
  fps = Recording->FramesPerSecond();
  const cRecordingInfo *RecordingInfo = Recording->Info();
  if (!RecordingInfo)
     return;
 
  SetTitle(RecordingInfo->Title());
  int w = smlFont->Width(RecordingInfo->ShortText());
  osd->DrawText(xp03, yp05 - lineHeight - Margin, RecordingInfo->ShortText(), Theme.Color(clrEventShortText), textColorBg, smlFont, min(xp15, (xp03 + w)) - xp03, lineHeight, taCenter);
  // must be (xp00 + Gap + Margin) to not interact with the ellipse
  osd->DrawText(xp00 + Gap + Margin, yp02 + Margin, ShortDateString(Recording->Start()), frameColorFg, frameColorBg, osdFont, xp02 - xp00 - Gap - 2 * Margin, lineHeight, taTop | taRight | taBorder);
  osd->DrawText(xp00 + Margin, yp05 - lineHeight - Margin, TimeString(Recording->Start()), frameColorFg, frameColorBg, osdFont, xp02 - xp00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);

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

  time_t liveEventStop = Event->EndTime();
  time_t recordingStart = time(0) - Recording->LengthInSeconds();
  framesTotal = (liveEventStop - recordingStart) * fps;
  endTime = cString::sprintf("%s: %s", tr("length"), *IndexToHMSF(framesTotal, false, fps));
}

void cLCARSNGDisplayReplay::SetTitle(const char *Title)
{
  int w = osdFont->Width(Title);
  osd->DrawText(xp03, yp02 + Margin, Title, Theme.Color(clrEventTitle), textColorBg, osdFont, min(xp15, (xp03 + w)) - xp03, lineHeight);
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
  osd->DrawBitmap(xp01 - bm.Width() / 2, (yp06 + yp07 - bm.Height()) / 2, bm, frameColorFg, frameColorBg);
}

void cLCARSNGDisplayReplay::SetProgress(int Current, int Total)
{
  current = Current;
  total = Total;
  int x = 0;
  int lH = lineHeight / 4;
  if (timshiftMode) {
     cString tM = "TimeshiftMode";
     int w = osdFont->Width(tM);
     osd->DrawText(xp03, yp05, tM, Theme.Color(clrReplayPosition), textColorBg, osdFont, w, 0, taRight);
  }
  if (isRecording) {
     int w = osdFont->Width(endTime);
     if (Total > framesTotal) {
        osd->DrawRectangle(xp15 - w, yp05, xp15 - 1, yp07 - lineHeight - 1, Theme.Color(clrBackground)); // Clear endTime
        isRecording = false;
        x = 0;
     }
     else {
        double rest = ((double)framesTotal - (double)Total) / (double)framesTotal;
        x = (int)((xp15 - xp03) * rest);
        osd->DrawText(xp15 - w, yp05, *endTime, Theme.Color(clrReplayPosition), textColorBg, osdFont, w, 0, taRight);
     }
     if (pbinit) {
        osd->DrawRectangle(xp03 + Margin, yp07 - lineHeight + 4 + lH, xp15 - Margin, yp07 - 4 - lH, frameColorBr); // small rectangle
        pbinit = false;
     }
  }
  if ((xp15 - xp03 - x - 5) > 0) {
     cProgressBar pb(xp15 - xp03 - x - 2 * Margin, lineHeight - 2 * Margin, Current, Total, marks, Theme.Color(clrReplayProgressSeen), Theme.Color(clrReplayProgressRest), Theme.Color(clrReplayProgressSelected), Theme.Color(clrReplayProgressMark), Theme.Color(clrReplayProgressCurrent));
     osd->DrawBitmap(xp03 + Margin, yp07 - lineHeight + Margin, pb); // Progressbar
  }
}

void cLCARSNGDisplayReplay::SetCurrent(const char *Current)
{
  int w = osdFont->Width(Current);
  osd->DrawText(xp03, yp08, Current, Theme.Color(clrReplayPosition), textColorBg, osdFont, max(lastCurrentWidth, w), 0, taTop | taLeft);
  if (lastCurrentWidth > w)
     osd->DrawRectangle(xp03 + lastCurrentWidth - (lastCurrentWidth - w), yp08, xp03 + lastCurrentWidth, yp08 + lineHeight, Theme.Color(clrBackground));
  lastCurrentWidth = w;

  // Display remaining time below progressbar
  int rest = current - total;
  cString restTime = cString::sprintf("%s", *IndexToHMSF(rest, false, fps));
  int wR = osdFont->Width(*restTime);
  osd->DrawText(xp15 - wR, yp08, *restTime, Theme.Color(clrReplayPosition), textColorBg, osdFont, max(lastRestWidth, wR), 0, taTop | taRight);
  lastRestWidth = wR;

  // Display total time above progressbar
  if (isRecording)
     return;

  endTime = cString::sprintf("%s: %s", tr("length"), *IndexToHMSF(total, false, fps));
  int wT = osdFont->Width(*endTime);
  osd->DrawText(xp15 - wT, yp06, *endTime, Theme.Color(clrReplayPosition), textColorBg, osdFont, max(lastTotalWidth, wT), 0, taTop | taRight);
  lastTotalWidth = wT;
}

void cLCARSNGDisplayReplay::SetTotal(const char *Total)
{
}

void cLCARSNGDisplayReplay::SetJump(const char *Jump)
{
  if (!Jump)
     osd->DrawRectangle(xp06 + Margin, yp11 + Margin, xp07 - 1 - Margin, yp12 - 1 - Margin, frameColorBg);
  osd->DrawText(xp06 + Margin, yp11 + Margin, Jump, Theme.Color(clrReplayJumpFg), Jump ? Theme.Color(clrReplayJumpBg) : frameColorBg, osdFont, xp07 - xp06 - 1 - 2 * Margin, yp12 - yp11 - 1 - 2 * Margin, taCenter);
}

void cLCARSNGDisplayReplay::SetMessage(eMessageType Type, const char *Text)
{
  if (Text) {
     DELETENULL(volumeBox);
     message = true;
     if (!messageBox)
        messageBox = new cLCARSNGMessageBox(osd, cRect(xv00, yv00, xv01 - xv00, yv01 - yv00), modeOnly);
     messageBox->SetMessage(Type, Text);
     }
  else {
     DELETENULL(messageBox);
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

void cLCARSNGDisplayReplay::DrawVolume(void)
{ 
  if (!message) {
     int volume = statusMonitor->GetVolume();
     if (volume != lastVolume) {
        if (!volumeBox)
           volumeBox = new cLCARSNGVolumeBox(osd, cRect(xv00, yv00, xv01 - xv00, yv01 - yv00), modeOnly);
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

void cLCARSNGDisplayReplay::Flush(void)
{
  if (!modeOnly) {
     DrawDate();
     DrawTrack();
     DrawBlinkingRec();
     DrawScreenResolution();
     }
  DrawVolume();
#ifdef DRAWGRID
  if (Config.displayGrid)
     DrawGrid();
#endif
  osd->Flush();
  initial = false;
}

#ifdef DRAWGRID
void cLCARSNGDisplayReplay::DrawGrid(void)
{
  int bottom = yp12;
  int top = yp01;
  tColor gridColor = 0xffff7700;
  int offset = lineHeight;

//  int xp00, xp01, xp02, xp03, xp04, xp05, xp06, xp07, xp08, xp09, xp10, xp11, xp12, xp13, xp14, xp15, xp16, xp17;
//  int xv00, xv01, yv00, yv01;
  int xp[18] = { xp00, xp01, xp02, xp03, xp04, xp05, xp06, xp07, xp08,
                 xp09, xp10, xp11, xp12, xp13, xp14, xp15, xp16, xp17 };
  char strxp[19][6] = { "xp00", "xp01", "xp02", "xp03", "xp04", "xp05", "xp06", "xp07", "xp08",
                        "xp09", "xp10", "xp11", "xp12", "xp13", "xp14", "xp15", "xp16", "xp17", '\0' };

  for (int i = 0; strxp[i][0]; i++) {
     osd->DrawRectangle(xp[i], top, xp[i] + 1, bottom - 1, gridColor);
     osd->DrawText(xp[i], top + offset, cString(strxp[i]), gridColor, clrTransparent, smlFont);

     offset = offset + lineHeight;

     if ((i % 4) == 0)
        offset = lineHeight;
  }
//  return;
  offset = lineHeight;

//  int yp00, yp01, yp05, yp06, yp07, yp08, yp09, yp10, yp11, yp12;
  int yp[13] = { yp00, yp01, yp02, yp03, yp04, yp05, yp06, yp07, yp08, yp09, yp10, yp11, yp12 };
  char stryp[14][6] = { "yp00", "yp01", "yp02", "yp03", "yp04", "yp05", "yp06", "yp07", "yp08", "yp09", "yp10", "yp11", "yp12", '\0' };

  int d = 5 * lineHeight;
  for (int i = 0; stryp[i][0]; i++) {
     osd->DrawRectangle(xp00, yp[i], xp14 - 1, yp[i] + 1, gridColor);
     osd->DrawText(xp06 + (d / 2) + (2.5 * offset), yp[i], cString(stryp[i]), gridColor, clrTransparent, smlFont);

     offset = offset + lineHeight;

     if ((i % 4) == 0)
        offset = lineHeight;
  }
}
#endif
