#include "lcarsng.h"
#include "displayvolume.h"
#include <vdr/font.h>

// --- cLCARSNGDisplayVolume -----------------------------------------------

cLCARSNGDisplayVolume::cLCARSNGDisplayVolume(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  int lineHeight = font->Height();
  frameColor = Theme.Color(clrVolumeFrame);
  mute = -1;
  x0 = 0;
  x1 = lineHeight / 2;
  x2 = lineHeight;
  x3 = x2 + Gap;
  x7 = cOsd::OsdWidth();
  x6 = x7 - lineHeight / 2;
  x5 = x6 - lineHeight / 2;
  x4 = x5 - Gap;
  y0 = 0;
  y1 = lineHeight;
  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - y1, x0, y0, x7 - 1, y1 - 1);
  osd->DrawRectangle(x0, y0, x7 - 1, y1 - 1, Theme.Color(clrBackground));
  osd->DrawRectangle(x0, y0, x1 - 1, y1 - 1, clrTransparent);
  osd->DrawEllipse  (x0, y0, x1 - 1, y1 - 1, frameColor, 7);
  osd->DrawRectangle(x1, y0, x2 - 1, y1 - 1, frameColor);
  osd->DrawRectangle(x3, y0, x4 - 1, y1 - 1, frameColor);
  osd->DrawRectangle(x5, y0, x6 - 1, y1 - 1, frameColor);
  osd->DrawRectangle(x6, y0, x7 - 1, y1 - 1, clrTransparent);
  osd->DrawEllipse  (x6, y0, x7 - 1, y1 - 1, frameColor, 5);
}

cLCARSNGDisplayVolume::~cLCARSNGDisplayVolume()
{
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayVolume::SetVolume(int Current, int Total, bool Mute)
{
  int xl = x3 + TextSpacing;
  int xr = x4 - TextSpacing;
  int yt = y0 + TextFrame;
  int yb = y1 - TextFrame;
  if (mute != Mute) {
     osd->DrawRectangle(x3, y0, x4 - 1, y1 - 1, frameColor);
     mute = Mute;
     }
  cBitmap bm(Mute ? mute_xpm : volume_xpm);
  osd->DrawBitmap(xl, y0 + (y1 - y0 - bm.Height()) / 2, bm, Theme.Color(clrVolumeSymbol), frameColor);
  if (!Mute) {
     xl += bm.Width() + TextSpacing;
     int w = (y1 - y0) / 3;
     int d = TextFrame;
     int n = (xr - xl + d) / (w + d);
     int x = xr - n * (w + d);
     tColor Color = Theme.Color(clrVolumeBarLower);
     for (int i = 0; i < n; i++) {
         if (Total * i >= Current * n)
            Color = Theme.Color(clrVolumeBarUpper);
         osd->DrawRectangle(x, yt, x + w - 1, yb - 1, Color);
         x += w + d;
         }
     }
}

void cLCARSNGDisplayVolume::Flush(void)
{
  osd->Flush();
}
