#include "config.h"
#include "lcarsng.h"
#include "messagebox.h"

cLCARSNGMessageBox::cLCARSNGMessageBox(cOsd *Osd, const cRect &Rect, bool Full) {
  osd = Osd;

  pixmapBg = osd->CreatePixmap(0, Rect);
  pixmapBg->Fill(0x01000000);
  pixmap = osd->CreatePixmap(7, Rect);
  pixmap->Fill(clrTransparent);

  this->Full = Full;
}

cLCARSNGMessageBox::~cLCARSNGMessageBox() {
  osd->DestroyPixmap(pixmapBg);
  osd->DestroyPixmap(pixmap);
}

void cLCARSNGMessageBox::SetMessage(eMessageType Type, const char *Text)
{
  if (Text && pixmap && pixmapBg) {
     int lineHeight = cFont::GetFont(fontOsd)->Height();
     tColor ColorFg = Theme.Color(clrMessageStatusFg + 2 * Type);
     tColor ColorBg = Theme.Color(clrMessageStatusBg + 2 * Type);
     int Margin = Config.Margin;

     int x0 = 0;
     int x1 = lineHeight / 2;
     int x2 = lineHeight;
     int x3 = x2 + Gap;
     int x7 = pixmap->ViewPort().Width();
     int x6 = x7 - lineHeight / 2;
     int x5 = x7 - lineHeight;
     int x4 = x5 - Gap;
     int y0 = 0;
     int y1 = pixmap->ViewPort().Height();

     int xt0, xt1;

     if (Full) {
        pixmapBg->DrawRectangle(cRect(x0, y0, x7 - x0, y1 - y0), Theme.Color(clrBackground));
        pixmapBg->DrawRectangle(cRect(x0, y0, x1 - x0, y1 - y0), clrTransparent);
        DrawEllipseOutline(pixmapBg, x0, y0, x1 - x0, y1 - y0, ColorFg, ColorBg, 7);
        DrawRectangleOutline(pixmapBg, x1, y0, x2 - x1, y1 - y0, ColorFg, ColorBg, 14);
        DrawRectangleOutline(pixmapBg, x3, y0, x4 - x3, y1 - y0, ColorFg, ColorBg, 15);
        DrawRectangleOutline(pixmapBg, x5, y0, x6 - x5, y1 - y0, ColorFg, ColorBg, 11);
        pixmapBg->DrawRectangle(cRect(x6, y0, x7 - x6, y1 - y0), clrTransparent);
        DrawEllipseOutline(pixmapBg, x6, y0, x7 - x6, y1 - y0, ColorFg, ColorBg, 5);
        xt0 = x3;
        xt1 = x4;
        }
     else {
        DrawRectangleOutline(pixmapBg, x0, y0, x7 - x0, y1 - y0, ColorFg, ColorBg, 15);
        xt0 = x0;
        xt1 = x7;
        }

     int lx = xt1 - xt0 - 2 * Margin;
     int ly = y1 - y0 - 2 * Margin;

     pixmap->DrawText(cPoint(xt0 + Margin, y0 + Margin), Text, ColorFg, clrTransparent, cFont::GetFont(fontSml), lx, ly, taCenter);
     }
}
