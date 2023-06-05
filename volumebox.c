#include "config.h"
#include "lcarsng.h"
#include "volumebox.h"

cLCARSNGVolumeBox::cLCARSNGVolumeBox(cOsd *Osd, const cRect &Rect, bool Full) {
  osd = Osd;
  pixmap = osd->CreatePixmap(7, Rect);
  pixmap->Fill(clrTransparent);
  pixmapBackground = osd->CreatePixmap(6, Rect);
  pixmapBackground->Fill(clrTransparent);

  const cFont *font = cFont::GetFont(fontOsd);
  int lineHeight = font->Height();
  frameColor = Theme.Color(clrVolumeFrame);
  mute = -1;
  full = Full;
  x0 = 0;
  x1 = lineHeight / 2;
  x2 = lineHeight;
  x3 = x2 + Gap;
  x7 = Rect.Width();
  x6 = x7 - lineHeight / 2;
  x5 = x6 - lineHeight / 2;
  x4 = x5 - Gap;
  y0 = 0;
  y1 = Rect.Height();
  if (Full) {
     pixmapBackground->DrawRectangle(cRect(x0, y0, x7, y1), Theme.Color(clrBackground));
     pixmapBackground->DrawRectangle(cRect(x0, y0, x1, y1), clrTransparent);
     pixmapBackground->DrawEllipse  (cRect(x0, y0, x1, y1), frameColor, 7);
     pixmapBackground->DrawRectangle(cRect(x1, y0, x1, y1), frameColor);
     pixmapBackground->DrawRectangle(cRect(x3, y0, x4 - x3, y1), frameColor);
     pixmapBackground->DrawRectangle(cRect(x5, y0, x6 - x5, y1), frameColor);
     pixmapBackground->DrawRectangle(cRect(x6, y0, x7 - x6, y1), clrTransparent);
     pixmapBackground->DrawEllipse  (cRect(x6, y0, x7 - x6, y1), frameColor, 5);
     }
  else {
     pixmapBackground->DrawRectangle(cRect(x0, y0, x7, y1), frameColor);
     }
}

cLCARSNGVolumeBox::~cLCARSNGVolumeBox() {
  osd->DestroyPixmap(pixmap);
  osd->DestroyPixmap(pixmapBackground);
}

void cLCARSNGVolumeBox::SetVolume(int Current, int Total, bool Mute) {
  int xl = (full ? x3 : x0) + TextSpacing;
  int xr = (full ? x4 : x7) - TextSpacing;
  int yt = y0 + TextFrame;
  int yb = y1 - TextFrame;
  if (mute != Mute) {
     pixmap->DrawRectangle(cRect(xl, y0, xr - xl, y1), frameColor);
     mute = Mute;
     }
  cBitmap bm(Mute ? mute_xpm : volume_xpm);
  pixmap->DrawBitmap(cPoint(xl, (y1 - bm.Height()) / 2), bm, Theme.Color(clrVolumeSymbol), frameColor);
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
         pixmap->DrawRectangle(cRect(x, yt, w, yb - yt), Color);
         x += w + d;
         }
     }
}
