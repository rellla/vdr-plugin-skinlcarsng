#include "lcarsng.h"
#include "displaymessage.h"
#include <vdr/font.h>

// --- cLCARSNGDisplayMessage ----------------------------------------------

cLCARSNGDisplayMessage::cLCARSNGDisplayMessage(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  int lineHeight = font->Height();
  x0 = 0;
  x1 = lineHeight / 2;
  x2 = lineHeight;
  x3 = x2 + Gap + 2 * Margin;
  x7 = cOsd::OsdWidth();
  x6 = x7 - lineHeight / 2;
  x5 = x6 - lineHeight / 2;
  x4 = x5 - Gap - 2 * Margin;;
  y0 = 0;
  y1 = lineHeight + 2 * Margin;
  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - y1, x0, y0, x7 - 1, y1 - 1);
}

cLCARSNGDisplayMessage::~cLCARSNGDisplayMessage()
{
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayMessage::SetMessage(eMessageType Type, const char *Text)
{
  tColor ColorFg = Theme.Color(clrMessageStatusFg + 2 * Type);
  tColor ColorBg = Theme.Color(clrMessageStatusBg + 2 * Type);
  osd->DrawRectangle(x0, y0, x7 - 1, y1 - 1, Theme.Color(clrBackground));
  osd->DrawRectangle(x0, y0, x1 - 1, y1 - 1, clrTransparent);
  osd->DrawEllipse  (x0, y0, x1 - 1, y1 - 1, ColorFg, 7);
  osd->DrawEllipse  (x0 + Margin, y0 + Margin, x1 - 1, y1 - 1 - Margin, ColorBg, 7);
  DrawRectangleOutline(osd, x1, y0, x2 - 1, y1 - 1, ColorFg, ColorBg, 14);
  DrawRectangleOutline(osd, x3 - Margin, y0, x4 - 1 + Margin, y1 - 1, ColorFg, ColorBg, 15);
  int w = cFont::GetFont(fontSml)->Width(Text);
  int x = (x4 -x3 - w) / 2;
  osd->DrawText(x3 + x + Margin, y0 + Margin, Text, ColorFg, ColorBg, cFont::GetFont(fontSml), w, y1 - y0 - 2 * Margin, taCenter);
  DrawRectangleOutline(osd, x5, y0, x6 - 1, y1 - 1, ColorFg, ColorBg, 11);
  osd->DrawRectangle(x6, y0, x7 - 1, y1 - 1, clrTransparent);
  osd->DrawEllipse  (x6, y0, x7 - 1, y1 - 1, ColorFg, 5);
  osd->DrawEllipse  (x6, y0 + Margin, x7 - 1 - Margin, y1 - 1 - Margin, ColorBg, 5);
}

void cLCARSNGDisplayMessage::Flush(void)
{
  osd->Flush();
}
