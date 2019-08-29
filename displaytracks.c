#include "lcarsng.h"
#include "displaytracks.h"
#include "config.h"
#include <vdr/font.h>

// --- cLCARSNGDisplayTracks -----------------------------------------------

cBitmap cLCARSNGDisplayTracks::bmAudioLeft(audioleft_xpm);
cBitmap cLCARSNGDisplayTracks::bmAudioRight(audioright_xpm);
cBitmap cLCARSNGDisplayTracks::bmAudioStereo(audiostereo_xpm);

cLCARSNGDisplayTracks::cLCARSNGDisplayTracks(const char *Title, int NumTracks, const char * const *Tracks)
{
  const cFont *font = cFont::GetFont(fontOsd);
  lineHeight = font->Height();
  frameColorFg = Theme.Color(clrTrackFrameFg);
  frameColorBg = Theme.Color(clrTrackFrameBg);
  frameColorMg = Theme.Color(clrTrackFrameMg);
  currentIndex = -1;
  Margin = Config.Margin;
  xt00 = 0;
  xt01 = xt00 + lineHeight / 2;
  xt02 = xt01 + Gap;
  xt03 = xt00 + 2 * lineHeight;
  int ItemsWidth = font->Width(Title) + xt03 - xt02;
  for (int i = 0; i < NumTracks; i++)
      ItemsWidth = max(ItemsWidth, font->Width(Tracks[i]) + 2 * TextFrame);
  xt04 = xt02 + ItemsWidth + 2 * Margin;
  xt05 = xt04 + Gap;
  xt06 = xt04 + lineHeight;
  xt07 = xt05 + lineHeight;
  xt08 = xt07 + lineHeight;
  xt09 = xt08 + Gap;
  xt10 = xt09 + lineHeight / 2;
  xt11 = xt10 + Gap;
  xt12 = xt11 + lineHeight;
  yt00 = 0;
  yt01 = yt00 + lineHeight + Margin;
  yt02 = yt01 + lineHeight + Margin;
  yt03 = yt02 + Gap;
  yt04 = yt03 + NumTracks * (lineHeight + 2 * Margin) + (NumTracks - 1) * Gap;
  yt05 = yt04 + Gap;
  yt06 = yt05 + lineHeight + Margin;
  yt07 = yt06 + lineHeight + Margin;
  while (yt07 > cOsd::OsdHeight()) {
        yt04 -= lineHeight + Gap - 2 * Margin;
        yt05 = yt04 + Gap;
        yt06 = yt05 + lineHeight + Margin;
        yt07 = yt06 + lineHeight + Margin;
        }
  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - yt07, xt00, yt00, xt12 - 1, yt07 - 1);
  // The upper elbow:
  osd->DrawRectangle(xt00, yt00, xt12 - 1, yt07 - 1, Theme.Color(clrBackground));
  osd->DrawRectangle(xt00, yt00, xt03 - 1, yt02 - 1, clrTransparent);
  osd->DrawEllipse  (xt00, yt00, xt03 - 1, yt02 - 1, frameColorMg, 2);
  osd->DrawEllipse  (xt00 + Margin, yt00 + Margin, xt03 - 1, yt02 - 1 - Margin, frameColorBg, 2);
  DrawRectangleOutline(osd, xt03, yt00, xt04 - 1, yt01 - 1, frameColorMg, frameColorBg, 2);
  DrawRectangleOutline(osd, xt03, yt01, xt04 - 1, yt02 - 1, frameColorMg, frameColorBg, 12);
  DrawRectangleOutline(osd, xt04, yt00, xt08 - 1, yt01 - 1, frameColorMg, frameColorBg, 14);
  osd->DrawEllipse  (xt04 - 1, yt01, xt06 - 1, yt02 - 1, frameColorMg, -2);
  osd->DrawEllipse  (xt04 - 1 - Margin, yt01 - Margin, xt06 - 1 - Margin, yt02 - 1 - Margin, frameColorBg, -2);
  DrawRectangleOutline(osd, xt09, yt00, xt10 - 1, yt01 - 1, frameColorMg, frameColorBg, 15);
  DrawRectangleOutline(osd, xt11, yt00, xt11 + lineHeight / 2 - 1, yt01 - 1, frameColorMg, frameColorBg, 11);
  osd->DrawRectangle(xt11 + lineHeight / 2, yt00, xt12 - 1, yt00 + lineHeight / 2 - 1, clrTransparent);
  osd->DrawEllipse  (xt11 + lineHeight / 2, yt00, xt12 - 1, yt01 - 1, frameColorMg, 5);
  osd->DrawEllipse  (xt11 + lineHeight / 2, yt00 + Margin, xt12 - 1 - Margin, yt01 - 1 - Margin, frameColorBg, 5);
  osd->DrawText(xt03, yt00 + Margin, Title, frameColorFg, frameColorBg, font, xt04 - xt03, 0, taTop | taRight);
  // The items:
  for (int i = 0; i < NumTracks; i++)
      SetItem(Tracks[i], i, false);
  // The lower elbow:
  osd->DrawRectangle(xt00, yt05, xt03 - 1, yt07 - 1, clrTransparent);
  osd->DrawEllipse  (xt00, yt05, xt03 - 1, yt07 - 1, frameColorMg, 3);
  osd->DrawEllipse  (xt00 + Margin, yt05 + Margin, xt03 - 1, yt07 - 1 - Margin, frameColorBg, 3);
  DrawRectangleOutline(osd, xt03, yt05, xt04 - 1, yt06 - 1, frameColorMg, frameColorBg, 6);
  DrawRectangleOutline(osd, xt03, yt06, xt04 - 1, yt07 - 1, frameColorMg, frameColorBg, 8);
  DrawRectangleOutline(osd, xt04, yt06, xt08 - 1, yt07 - 1, frameColorMg, frameColorBg, 14);
  osd->DrawEllipse  (xt04 - 1, yt05, xt06 - 1, yt06 - 1, frameColorMg, -3);
  osd->DrawEllipse  (xt04 - 1 - Margin, yt05 + Margin, xt06 - 1 - Margin, yt06 - 1 + Margin, frameColorBg, -3);
  DrawRectangleOutline(osd, xt09, yt06, xt10 - 1, yt07 - 1, frameColorMg, frameColorBg, 15);
  DrawRectangleOutline(osd, xt11, yt06, xt11 + lineHeight / 2 - 1, yt07 - 1, frameColorMg, frameColorBg, 11);
  osd->DrawRectangle(xt11 + lineHeight / 2, yt06 + lineHeight / 2, xt12 - 1, yt07 - 1, clrTransparent);
  osd->DrawEllipse  (xt11 + lineHeight / 2, yt06, xt12 - 1, yt07 - 1, frameColorMg, 5);
  osd->DrawEllipse  (xt11 + lineHeight / 2, yt06 + Margin, xt12 - 1 - Margin, yt07 - 1 - Margin, frameColorBg, 5);
}

cLCARSNGDisplayTracks::~cLCARSNGDisplayTracks()
{
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayTracks::SetItem(const char *Text, int Index, bool Current)
{
  int y0 = yt03 + Index * (lineHeight + Gap + 2 * Margin);
  int y1 = y0 + lineHeight + 2 * Margin;
  if (y1 > yt04)
     return;
  tColor ColorFg, ColorBg;
  if (Current) {
     ColorFg = Theme.Color(clrTrackItemCurrentFg);
     ColorBg = Theme.Color(clrTrackItemCurrentBg);
     DrawRectangleOutline(osd, xt00, y0, xt01 - 1, y1 - 1, frameColorMg, frameColorBg, 15);
     DrawRectangleOutline(osd, xt02, y0, xt04 - 1, y1 - 1, frameColorMg, frameColorBg, 15);
     DrawRectangleOutline(osd, xt05, y0, xt05 + lineHeight / 2 - 1, y1 - 1, frameColorMg, ColorBg, 11);
     osd->DrawEllipse  (xt05 + lineHeight / 2, y0, xt07 - 1, y1 - 1, frameColorMg, 5);
     osd->DrawEllipse  (xt05 + lineHeight / 2, y0 + Margin, xt07 - 1 - Margin, y1 - 1 - Margin, ColorBg, 5);
     currentIndex = Index;
     }
  else {
     ColorFg = Theme.Color(clrTrackItemFg);
     ColorBg = Theme.Color(clrTrackItemBg);
     DrawRectangleOutline(osd, xt00, y0, xt01 - 1, y1 - 1, frameColorMg, frameColorBg, 15);
     DrawRectangleOutline(osd, xt02, y0, xt04 - 1, y1 - 1, frameColorMg, frameColorBg, 15);
     if (currentIndex == Index)
        osd->DrawRectangle(xt05, y0, xt07 - 1, y1 - 1, Theme.Color(clrBackground));
     }
  const cFont *font = cFont::GetFont(fontOsd);
  osd->DrawText(xt02 + Margin, y0 + Margin, Text, ColorFg, ColorBg, font, xt04 - xt02 - 2 * Margin, y1 - y0 - 2 * Margin, taTop | taLeft | taBorder);
}

void cLCARSNGDisplayTracks::SetTrack(int Index, const char * const *Tracks)
{
  if (currentIndex >= 0)
     SetItem(Tracks[currentIndex], currentIndex, false);
  SetItem(Tracks[Index], Index, true);
}

void cLCARSNGDisplayTracks::SetAudioChannel(int AudioChannel)
{
  cBitmap *bm = NULL;
  switch (AudioChannel) {
    case 0: bm = &bmAudioStereo; break;
    case 1: bm = &bmAudioLeft;   break;
    case 2: bm = &bmAudioRight;  break;
    default: ;
    }
  if (bm)
     osd->DrawBitmap(xt04 - bm->Width(), (yt06 + yt07 - bm->Height()) / 2 - 2 * Margin, *bm, Theme.Color(clrTrackItemCurrentFg), frameColorBg);
  else
     osd->DrawRectangle(xt03, yt06, xt04 - 1, yt07 - 1 - Margin, frameColorBg);
}

void cLCARSNGDisplayTracks::Flush(void)
{
  osd->Flush();
}
