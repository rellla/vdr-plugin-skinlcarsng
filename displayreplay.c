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

cLCARSNGDisplayReplay::cLCARSNGDisplayReplay(bool ModeOnly) : cThread("LCARS DisplRepl")
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
  resolution = "";
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
  xp18 = cOsd::OsdWidth();                         // Right
  xp17 = xp18 - lineHeight - 2 * Margin;
  xp16 = xp17 - Gap;
  xp15 = xp16;
  xp14 = xp16 - ((zoom > 1) ? 1.5 : 2) * d;
  xp13 = xp14 - Gap;
  xp07 = (xp18 + xp00) / 2;
  xp08 = xp07 + Gap;
  xp09 = xp08 + lineHeight;
  xp10 = xp09 + Gap;
  xp11 = (xp10 + xp16 + Gap) / 2;
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
  xv01 = (modeOnly) ? xp18 : xp13;
  yv00 = yp00;
  yv01 = yp01;

  leftIcons = 0;

  int x0 = xp00;
  int x1 = xp18;
  int y0 = yp00;
  int y1 = yp12;

  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - yp12, xp00, yp00, xp18 - 1, yp12 - 1);
  if (osd) {
     ReplayPixmap = osd->CreatePixmap(2, cRect(x0, y0, x1 - x0, y1 - y0));
     if (ReplayPixmap) {
        ReplayPixmap->SetAlpha(255);
        ReplayPixmap->Fill(CLR_Clear);
        }
     }
  if (!ReplayPixmap)
     return;

#ifdef DRAWGRID
  if (Config.displayGrid) {
     GridPixmap = osd->CreatePixmap(3, cRect(x0, y0, x1 - x0, y1 - y0));
     if (GridPixmap) {
        GridPixmap->SetAlpha(255);
        GridPixmap->Fill(clrTransparent);
        }
     }
#endif

  DrawRectangle(ReplayPixmap, xp00, yp00, xp18, yp12, modeOnly ? clrTransparent : Theme.Color(clrBackground));
  // Rectangles:
  DrawRectangleOutline(ReplayPixmap, xp00, yp06, xp02, yp07, frameColorBr, frameColorBg, 15);
  if (!modeOnly) {
     // Top middle
     DrawRectangleOutline(ReplayPixmap, xp06, yp00, xp13, yp01 / 2, frameColorBr, frameColorBg, 15);
     // Upper Elbow:
     DrawRectangleOutline(ReplayPixmap, xp00, yp00, xp02, yp03, frameColorBr, frameColorBg, 3); 
     DrawRectangleOutline(ReplayPixmap, xp00, yp01, xp02, yp05, frameColorBr, frameColorBg, 13);
     DrawRectangleOutline(ReplayPixmap, xp02, yp00, xp05, yp01, frameColorBr, frameColorBg, 14);
     DrawRectangle(ReplayPixmap, xp00, yp00, xp01, yp04, clrTransparent);
     DrawEllipseOutline(ReplayPixmap, xp00, yp00, xp01, yp04, frameColorBr, frameColorBg, 2);
     DrawEllipseOutline(ReplayPixmap, xp02, yp01, xp04, yp03, frameColorBr, frameColorBg, -2);
     // Lower Elbow:
     DrawRectangleOutline(ReplayPixmap, xp00, yp08, xp02, yp11, frameColorBr, frameColorBg, 7);
     DrawRectangleOutline(ReplayPixmap, xp00, yp11, xp02, yp12, frameColorBr, frameColorBg, 8);
     DrawRectangleOutline(ReplayPixmap, xp02, yp11, xp05, yp12, frameColorBr, frameColorBg, 14);
     DrawRectangle(ReplayPixmap, xp00, yp09, xp01, yp12, clrTransparent);
     DrawEllipseOutline(ReplayPixmap, xp00, yp09, xp01, yp12, frameColorBr, frameColorBg, 3);
     DrawEllipseOutline(ReplayPixmap, xp02, yp10, xp04, yp11, frameColorBr, frameColorBg, -3);
     // Status area:
     DrawRectangleOutline(ReplayPixmap, xp06, yp11, xp07, yp12, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(ReplayPixmap, xp08, yp11, xp09, yp12, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(ReplayPixmap, xp10, yp11, xp11, yp12, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(ReplayPixmap, xp12, yp11, xp16, yp12, frameColorBr, frameColorBg, 15);
     DrawRectangleOutline(ReplayPixmap, xp17, yp11, xp17 + lineHeight / 2 + Margin, yp12, frameColorBr, frameColorBg, 11);
     DrawRectangle(ReplayPixmap, xp17 + lineHeight / 2 + Margin, yp11 + lineHeight / 2, xp18, yp12, clrTransparent);
     DrawEllipseOutline(ReplayPixmap, xp17 + lineHeight / 2 + Margin, yp11, xp18, yp12, frameColorBr, frameColorBg, 5);
     // Upper Right:
     DrawRectangleOutline(ReplayPixmap, xp17, yp00, xp17 + lineHeight / 2 + Margin, yp01, frameColorBr, frameColorBg, 11);
     DrawRectangle(ReplayPixmap, xp17 + lineHeight / 2 + Margin, yp00, xp18, yp01 - lineHeight / 2, clrTransparent);
     DrawEllipseOutline(ReplayPixmap, xp17 + lineHeight / 2 + Margin, yp00, xp18, yp01, frameColorBr, frameColorBg, 5);
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
     DrawText(ReplayPixmap, xp14, yp00, s, Theme.Color(clrDateFg), Theme.Color(clrDateBg), osdFont, xp16 - xp14, yp01, taRight | taBorder);
     lastDate = s;
     }
}

void cLCARSNGDisplayReplay::DrawTrack(void)
{
  cDevice *Device = cDevice::PrimaryDevice();
  const tTrackId *Track = Device->GetTrack(Device->GetCurrentAudioTrack());
  if (Track ? strcmp(lastTrackId.description, Track->description) : *lastTrackId.description) {
     DrawText(ReplayPixmap, xp10 + Margin, yp11 + Margin, Track ? Track->description : "", Theme.Color(clrTrackName), clrTransparent, (zoom) ? smlFont : osdFont, xp11 - xp10 - textBorder - 2 * Margin, yp12 - yp11 - 2 * Margin, taCenter | taRight | taBorder);
     strn0cpy(lastTrackId.description, Track ? Track->description : "", sizeof(lastTrackId.description));
     }
}

void cLCARSNGDisplayReplay::DrawScreenResolution(void)
{
  if (isempty(resolution))
     resolution = GetScreenResolutionIcon();

  // Draw video format
  if (!(strcmp(resolution, oldResolution) == 0)) {
     if (strcmp(resolution, "") == 0) {
        DrawRectangle(ReplayPixmap, xp11 + Margin, yp11 + Margin, leftIcons, yp12 - Margin, frameColorBg);
        }
     const cFont *font = (zoom) ? smlFont : osdFont;
     int w = font->Width(*resolution) + 2 * textBorder;
     int x = leftIcons - w - SymbolSpacing;
     int h = ((zoom) && (iconHeight < lineHeight)) ? (lineHeight - iconHeight) / 2 : 0;
     DrawText(ReplayPixmap, x, yp11 + h + Margin, cString::sprintf("%s", *resolution), Theme.Color(clrChannelSymbolOn), frameColorBr, font, w, yp12 - yp11 - 2 * Margin - 2 * h, taCenter | taRight | taBorder);
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
     int x = xp16;
     x -= bmRecording.Width() + SymbolSpacing;
     DrawBitmap(ReplayPixmap, x, yp11 + (yp12 - yp11 - bmRecording.Height()) / 2, bmRecording, Theme.Color(rec ? On ? clrChannelSymbolRecFg : clrChannelSymbolOff : clrChannelSymbolOff), rec ? On ? Theme.Color(clrChannelSymbolRecBg) : frameColorBr : frameColorBr);
     lastOn = On;
     leftIcons = x;
     }
}

void cLCARSNGDisplayReplay::SetRecording(const cRecording *Recording)
{
  fps = Recording->FramesPerSecond();

  if (PosterPixmap) {
     osd->DestroyPixmap(PosterPixmap);
     PosterPixmap = NULL;
     }

  if (Config.displayScraperInfo && Config.displReplayPoster) {
     DrawPoster_t dP;
     dP.osd = osd;
     dP.Recording = Recording;
     dP.x = xp16 - 5 * lineHeight;
     dP.y = yp02;
     dP.width = 5 * lineHeight;
     dP.height = yp11 - lineHeight / 2 - yp02;

     PosterPixmap = DrawPoster(dP);
     if (PosterPixmap) {
        PosterPixmap->SetAlpha(255);
        xp15 = xp16 - PosterPixmap->ViewPort().Width() - lineHeight;
        }
     }

  const cRecordingInfo *RecordingInfo = Recording->Info();
  if (!RecordingInfo)
     return;
 
#if (APIVERSNUM >= 20605)
  // Get video format from info file
  if (!isempty(RecordingInfo->FrameParams())) {
     if (Config.displReplayVideoFormat)
        resolution = RecordingInfo->FrameParams();
     else {
        uint16_t frameHeight = RecordingInfo->FrameHeight();
        if (frameHeight) {
           if (frameHeight > 2000)
              resolution = "UHD";
           else if (frameHeight > 700)
              resolution = "HD";
           else
              resolution = "SD";
           resolution.Append(cString::sprintf("%d", (frameHeight)));
           if (RecordingInfo->ScanType() != stUnknown)
              resolution.Append(RecordingInfo->ScanTypeChar());
           }
        }
     }
#endif

  recordingErrors = RecordingInfo->Errors();
  SetTitle(RecordingInfo->Title());
  int w = smlFont->Width(RecordingInfo->ShortText()) + 2 * textBorder;
  DrawText(ReplayPixmap, xp03, yp05 - lineHeight - Margin, RecordingInfo->ShortText(), Theme.Color(clrEventShortText), textColorBg, smlFont, min(xp15, (xp03 + w)) - xp03, lineHeight, taCenter | taBorder);
  if (Config.displReplayDateTime) {
     // ShortDateString
     // must be (xp00 + Gap + Margin) to not interact with the ellipse
     DrawText(ReplayPixmap, xp00 + Gap + Margin, yp02 + Margin, ShortDateString(Recording->Start()), frameColorFg, frameColorBg, osdFont, xp02 - xp00 - Gap - 2 * Margin, lineHeight, taTop | taRight | taBorder);
     // TimeString
     DrawText(ReplayPixmap, xp00 + Margin, yp05 - lineHeight - Margin, TimeString(Recording->Start()), frameColorFg, frameColorBg, osdFont, xp02 - xp00 - 2 * Margin, lineHeight, taBottom | taRight | taBorder);
     }

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
  int w = osdFont->Width(Title) + 2 * textBorder;
  DrawText(ReplayPixmap, xp03, yp02 + Margin, Title, Theme.Color(clrEventTitle), textColorBg, osdFont, min(xp15, (xp03 + w)) - xp03, lineHeight, taCenter | taBorder);
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
  DrawBitmap(ReplayPixmap, xp01 - bm.Width() / 2, (yp06 + yp07 - bm.Height()) / 2, bm, frameColorFg, frameColorBg);
}

void cLCARSNGDisplayReplay::SetProgress(int Current, int Total)
{
  current = Current;
  total = Total;
  int x = 0;
  int pbHeight = lineHeight - 2 * Margin;
  int rest = pbHeight % 3;
  if (rest)
     pbHeight += rest;
  int lH = pbHeight / 3;

  if (pbinit) {
     // Draw progressbar outline
     int pH = pbHeight + 2 * Margin;
     DrawRectangleOutline(ReplayPixmap, xp03, yp07 - pH, xp15, yp07, frameColorBr, frameColorBg, 15);
     }

  if (timshiftMode) {
     cString tM = "TimeshiftMode";
     int w = osdFont->Width(tM) + 2 * textBorder;
     DrawText(ReplayPixmap, xp03, yp05, tM, Theme.Color(clrReplayPosition), textColorBg, osdFont, w, 0, taCenter | taBorder);
  }
  if (isRecording) {
     int w = osdFont->Width(endTime) + 2 * textBorder;
     if (Total > framesTotal) {
        // Clear endTime
        DrawRectangle(ReplayPixmap, xp15 - w, yp05, xp15, yp07 - lineHeight, Theme.Color(clrBackground));
        isRecording = false;
        x = 0;
     }
     else {
        // Draw endTime
        double rest = ((double)framesTotal - (double)Total) / (double)framesTotal;
        x = (int)((xp15 - xp03) * rest);
        DrawText(ReplayPixmap, xp15 - w, yp05, *endTime, Theme.Color(clrReplayPosition), textColorBg, osdFont, w, 0, taCenter | taBorder);
     }
     if (pbinit) {
        // small middle rectangle
        int y1 = yp07 - lineHeight + Margin + lH;
        int y2 = yp07 - Margin - lH;
        DrawRectangle(ReplayPixmap, xp03 + Margin, y1, xp15 - Margin, y2, frameColorBr);
     }
  }
  pbinit = false;

  if ((xp15 - xp03 - x - 5) > 0) {
     // Draw progressbar
#if APIVERSNUM >= 30004
     cProgressBar pb(xp15 - xp03 - x - 2 * Margin, pbHeight, Current, Total, marks, errors, Theme.Color(clrReplayProgressSeen), Theme.Color(clrReplayProgressRest), Theme.Color(clrReplayProgressSelected), Theme.Color(clrReplayProgressMark), Theme.Color(clrReplayProgressCurrent), Theme.Color(clrReplayProgressError));
#else
     cProgressBar pb(xp15 - xp03 - x - 2 * Margin, pbHeight, Current, Total, marks, Theme.Color(clrReplayProgressSeen), Theme.Color(clrReplayProgressRest), Theme.Color(clrReplayProgressSelected), Theme.Color(clrReplayProgressMark), Theme.Color(clrReplayProgressCurrent));
#endif
     DrawBitmap(ReplayPixmap, xp03 + Margin, yp07 - lineHeight + Margin, pb);
  }
}

void cLCARSNGDisplayReplay::SetCurrent(const char *Current)
{
  int numFramesAfterEdit = -1;
  int currentFramesAfterEdit = -1;

  if (marks) {
#if (APIVERSNUM >= 20608)
     numFramesAfterEdit = marks->GetFrameAfterEdit(total, total);;
     if (numFramesAfterEdit >= 0)
        currentFramesAfterEdit = marks->GetFrameAfterEdit(current, total);
#else
     numFramesAfterEdit = GetFrameAfterEdit(marks, total, total);
     if (numFramesAfterEdit >= 0)
        currentFramesAfterEdit = GetFrameAfterEdit(marks, current, total);
#endif
     }

  // Display current time below progressbar
  cString currentTime;
  if (numFramesAfterEdit >= 0 && Config.displReplayCutted == 2)
     currentTime = cString::sprintf("%s", *IndexToHMSF(currentFramesAfterEdit, false, fps));
  else if (numFramesAfterEdit >= 0 && Config.displReplayCutted == 1)
     currentTime = cString::sprintf("%s / %s", *IndexToHMSF(current, false, fps), *IndexToHMSF(currentFramesAfterEdit, false, fps));
  else
     currentTime = cString::sprintf("%s", *IndexToHMSF(current, false, fps));

  int w = osdFont->Width(currentTime) + 2 * textBorder;
  if (lastCurrentWidth > w)
     DrawRectangle(ReplayPixmap, xp03, yp08, xp03 + lastCurrentWidth, yp08 + lineHeight, Theme.Color(clrBackground));

  DrawText(ReplayPixmap, xp03, yp08, currentTime, Theme.Color(clrReplayPosition), textColorBg, osdFont, w, 0, taCenter | taBorder);
  lastCurrentWidth = w;

  // Display remaining time below progressbar
  int restcutted = numFramesAfterEdit - currentFramesAfterEdit;
  int rest = total - current;

  cString restTime;
  if (numFramesAfterEdit >= 0 && Config.displReplayCutted == 2)
     restTime = cString::sprintf("-%s", *IndexToHMSF(restcutted, false, fps));
  else if (numFramesAfterEdit >= 0 && Config.displReplayCutted == 1)
     restTime = cString::sprintf("-%s / -%s", *IndexToHMSF(restcutted, false, fps), *IndexToHMSF(rest, false, fps));
  else
     restTime = cString::sprintf("-%s", *IndexToHMSF(rest, false, fps));

  int wR = osdFont->Width(*restTime) + 2 * textBorder;
  if (lastRestWidth > wR)
     DrawRectangle(ReplayPixmap, xp15 - lastRestWidth, yp08, xp15, yp08 + lineHeight, Theme.Color(clrBackground));

  DrawText(ReplayPixmap, xp15 - wR, yp08, *restTime, Theme.Color(clrReplayPosition), textColorBg, osdFont, wR, 0, taCenter | taBorder);
  lastRestWidth = wR;

  // Display total time above progressbar
  if (isRecording && !(total > framesTotal))
     endTime = cString::sprintf("%s: %s / %s", tr("length"), *IndexToHMSF(total, false, fps), *IndexToHMSF(framesTotal, false, fps));
  else {
     if (Config.displReplayEnd == 1) {
        if (numFramesAfterEdit >= 0 && Config.displReplayCutted == 2)
           endTime = cString::sprintf("%s %s", tr("ends at"), *TimeString(time(0) + (restcutted / fps)));
        else if (numFramesAfterEdit >= 0 && Config.displReplayCutted == 1)
           endTime = cString::sprintf("%s %s / %s", tr("ends at"), *TimeString(time(0) + (restcutted / fps)), *TimeString(time(0) + (rest / fps)));
        else
           endTime = cString::sprintf("%s %s", tr("ends at"), *TimeString(time(0) + (rest / fps)));
        }
     else {
        if (numFramesAfterEdit >= 0 && Config.displReplayCutted == 2)
           endTime = cString::sprintf("%s: %s", tr("length"), *IndexToHMSF(numFramesAfterEdit, false, fps));
        else if (numFramesAfterEdit >= 0 && Config.displReplayCutted == 1)
           endTime = cString::sprintf("%s: %s / %s", tr("length"), *IndexToHMSF(numFramesAfterEdit, false, fps), *IndexToHMSF(total, false, fps));
        else
           endTime = cString::sprintf("%s: %s", tr("length"), *IndexToHMSF(total, false, fps));
        }
     }

  int wT = osdFont->Width(*endTime) + 2 * textBorder;
  if (lastTotalWidth > wT)
     DrawRectangle(ReplayPixmap, xp15 - lastTotalWidth, yp06, xp15, yp06 + lineHeight, Theme.Color(clrBackground));

  DrawText(ReplayPixmap, xp15 - wT, yp06, *endTime, Theme.Color(clrReplayPosition), textColorBg, osdFont, wT, 0, taCenter | taBorder);
  lastTotalWidth = wT;

  // Display recording error
  if (Config.displReplayErrors && recordingErrors >= Config.displReplayErrors)
     DrawText(ReplayPixmap, xp08 + Margin, yp11 + Margin, "!", clrRed, frameColorBg, smlFont, xp09 - xp08 - 2 * Margin, yp12 - yp11 - 2 * Margin, taCenter);
}

void cLCARSNGDisplayReplay::SetTotal(const char *Total)
{
}

void cLCARSNGDisplayReplay::SetJump(const char *Jump)
{
  if (!Jump)
     osd->DrawRectangle(xp06 + Margin, yp11 + Margin, xp07 - Margin, yp12 - Margin, frameColorBg);
  DrawText(ReplayPixmap, xp06 + Margin, yp11 + Margin, Jump, Theme.Color(clrReplayJumpFg), Jump ? Theme.Color(clrReplayJumpBg) : frameColorBg, osdFont, xp07 - xp06 - 2 * Margin, yp12 - yp11 - 2 * Margin, taCenter);
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
  if (initial && Config.displayGrid)
     DrawGrid();
#endif
  osd->Flush();
  initial = false;
}

#ifdef DRAWGRID
void cLCARSNGDisplayReplay::DrawGrid(void)
{
  if (!GridPixmap)
     return;

  int bottom = yp12;
  int top = yp01;
  tColor gridColor = 0xffff7700;
  int offset = lineHeight;

//  int xp00, xp01, xp02, xp03, xp04, xp05, xp06, xp07, xp08, xp09, xp10, xp11, xp12, xp13, xp14, xp16, xp17, xp18;
//  int xv00, xv01, yv00, yv01;
  int xp[19] = { xp00, xp01, xp02, xp03, xp04, xp05, xp06, xp07, xp08,
                 xp09, xp10, xp11, xp12, xp13, xp14, xp15, xp16, xp17, xp18 };
  char strxp[20][6] = { "xp00", "xp01", "xp02", "xp03", "xp04", "xp05", "xp06", "xp07", "xp08",
                        "xp09", "xp10", "xp11", "xp12", "xp13", "xp14", "xp15", "xp16", "xp17", "xp18", '\0' };

  for (int i = 0; strxp[i][0]; i++) {
     DrawRectangle(GridPixmap, xp[i], top, xp[i] + 1, bottom, gridColor);
     DrawText(GridPixmap, xp[i], top + offset, cString(strxp[i]), gridColor, clrTransparent, smlFont);

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
     DrawRectangle(GridPixmap, xp00, yp[i], xp18, yp[i] + 1, gridColor);
     DrawText(GridPixmap, xp06 + (d / 2) + (2.5 * offset), yp[i], cString(stryp[i]), gridColor, clrTransparent, smlFont);

     offset = offset + lineHeight;

     if ((i % 4) == 0)
        offset = lineHeight;
  }
}
#endif
