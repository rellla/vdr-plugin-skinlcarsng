#include "lcarsng.h"
#include "displaytracks.h"
#include <vdr/font.h>

// --- cLCARSNGDisplayTracks -----------------------------------------------

cBitmap cLCARSNGDisplayTracks::bmAudioLeft(audioleft_xpm);
cBitmap cLCARSNGDisplayTracks::bmAudioRight(audioright_xpm);
cBitmap cLCARSNGDisplayTracks::bmAudioStereo(audiostereo_xpm);

cLCARSNGDisplayTracks::cLCARSNGDisplayTracks(const char *Title, int NumTracks, const char * const *Tracks)
{
  const cFont *font = cFont::GetFont(fontOsd);
  lineHeight = font->Height();
  frameColor = Theme.Color(clrTrackFrameBg);
  currentIndex = -1;
  xt00 = 0;
  xt01 = xt00 + lineHeight / 2;
  xt02 = xt01 + Gap;
  xt03 = xt00 + 2 * lineHeight;
  int ItemsWidth = font->Width(Title) + xt03 - xt02;
  for (int i = 0; i < NumTracks; i++)
      ItemsWidth = max(ItemsWidth, font->Width(Tracks[i]) + 2 * TextFrame);
  xt04 = xt02 + ItemsWidth;
  xt05 = xt04 + Gap;
  xt06 = xt04 + lineHeight;
  xt07 = xt05 + lineHeight;
  xt08 = xt07 + lineHeight;
  xt09 = xt08 + Gap;
  xt10 = xt09 + lineHeight / 2;
  xt11 = xt10 + Gap;
  xt12 = xt11 + lineHeight;
  yt00 = 0;
  yt01 = yt00 + lineHeight;
  yt02 = yt01 + lineHeight;
  yt03 = yt02 + Gap;
  yt04 = yt03 + NumTracks * lineHeight + (NumTracks - 1) * Gap;
  yt05 = yt04 + Gap;
  yt06 = yt05 + lineHeight;
  yt07 = yt06 + lineHeight;
  while (yt07 > cOsd::OsdHeight()) {
        yt04 -= lineHeight + Gap;
        yt05 = yt04 + Gap;
        yt06 = yt05 + lineHeight;
        yt07 = yt06 + lineHeight;
        }
  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - yt07, xt00, yt00, xt12 - 1, yt07 - 1);
  // The upper elbow:
  osd->DrawRectangle(xt00, yt00, xt12 - 1, yt07 - 1, Theme.Color(clrBackground));
  osd->DrawRectangle(xt00, yt00, xt03 - 1, yt02 - 1, clrTransparent);
  osd->DrawEllipse  (xt00, yt00, xt03 - 1, yt02 - 1, frameColor, 2);
  osd->DrawRectangle(xt03, yt00, xt04 - 1, yt02 - 1, frameColor);
  osd->DrawRectangle(xt04, yt00, xt08 - 1, yt01 - 1, frameColor);
  osd->DrawEllipse  (xt04, yt01, xt06 - 1, yt02 - 1, frameColor, -2);
  osd->DrawRectangle(xt09, yt00, xt10 - 1, yt01 - 1, frameColor);
  osd->DrawRectangle(xt11, yt00, xt11 + lineHeight / 2 - 1, yt01 - 1, frameColor);
  osd->DrawRectangle(xt11 + lineHeight / 2, yt00, xt12 - 1, yt00 + lineHeight / 2 - 1, clrTransparent);
  osd->DrawEllipse  (xt11 + lineHeight / 2, yt00, xt12 - 1, yt01 - 1, frameColor, 5);
  osd->DrawText(xt03, yt00, Title, Theme.Color(clrTrackFrameFg), frameColor, font, xt04 - xt03, 0, taTop | taRight);
  // The items:
  for (int i = 0; i < NumTracks; i++)
      SetItem(Tracks[i], i, false);
  // The lower elbow:
  osd->DrawRectangle(xt00, yt05, xt03 - 1, yt07 - 1, clrTransparent);
  osd->DrawEllipse  (xt00, yt05, xt03 - 1, yt07 - 1, frameColor, 3);
  osd->DrawRectangle(xt03, yt05, xt04 - 1, yt07 - 1, frameColor);
  osd->DrawRectangle(xt04, yt06, xt08 - 1, yt07 - 1, frameColor);
  osd->DrawEllipse  (xt04, yt05, xt06 - 1, yt06 - 1, frameColor, -3);
  osd->DrawRectangle(xt09, yt06, xt10 - 1, yt07 - 1, frameColor);
  osd->DrawRectangle(xt11, yt06, xt11 + lineHeight / 2 - 1, yt07 - 1, frameColor);
  osd->DrawRectangle(xt11 + lineHeight / 2, yt06 + lineHeight / 2, xt12 - 1, yt07 - 1, clrTransparent);
  osd->DrawEllipse  (xt11 + lineHeight / 2, yt06, xt12 - 1, yt07 - 1, frameColor, 5);
}

cLCARSNGDisplayTracks::~cLCARSNGDisplayTracks()
{
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayTracks::SetItem(const char *Text, int Index, bool Current)
{
  int y0 = yt03 + Index * (lineHeight + Gap);
  int y1 = y0 + lineHeight;
  if (y1 > yt04)
     return;
  tColor ColorFg, ColorBg;
  if (Current) {
     ColorFg = Theme.Color(clrTrackItemCurrentFg);
     ColorBg = Theme.Color(clrTrackItemCurrentBg);
     osd->DrawRectangle(xt00, y0, xt01 - 1, y1 - 1, frameColor);
     osd->DrawRectangle(xt02, y0, xt04 - 1, y1 - 1, ColorBg);
     osd->DrawRectangle(xt05, y0, xt05 + lineHeight / 2 - 1, y1 - 1, ColorBg);
     osd->DrawEllipse  (xt05 + lineHeight / 2, y0, xt07 - 1, y1 - 1, ColorBg, 5);
     currentIndex = Index;
     }
  else {
     ColorFg = Theme.Color(clrTrackItemFg);
     ColorBg = Theme.Color(clrTrackItemBg);
     osd->DrawRectangle(xt00, y0, xt01 - 1, y1 - 1, frameColor);
     osd->DrawRectangle(xt02, y0, xt04 - 1, y1 - 1, ColorBg);
     if (currentIndex == Index)
        osd->DrawRectangle(xt05, y0, xt07 - 1, y1 - 1, Theme.Color(clrBackground));
     }
  const cFont *font = cFont::GetFont(fontOsd);
  osd->DrawText(xt02, y0, Text, ColorFg, ColorBg, font, xt04 - xt02, y1 - y0, taTop | taLeft | taBorder);
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
     osd->DrawBitmap(xt04 - bm->Width(), (yt06 + yt07 - bm->Height()) / 2, *bm, Theme.Color(clrTrackFrameFg), frameColor);
  else
     osd->DrawRectangle(xt03, yt06, xt04 - 1, yt07 - 1, frameColor);
}

void cLCARSNGDisplayTracks::Flush(void)
{
  osd->Flush();
}
