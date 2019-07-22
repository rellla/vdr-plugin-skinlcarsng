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
  x3 = x2 + Gap;
  x7 = cOsd::OsdWidth();
  x6 = x7 - lineHeight / 2;
  x5 = x6 - lineHeight / 2;
  x4 = x5 - Gap;
  y0 = 0;
  y1 = lineHeight;
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
  osd->DrawEllipse  (x0, y0, x1 - 1, y1 - 1, ColorBg, 7);
  osd->DrawRectangle(x1, y0, x2 - 1, y1 - 1, ColorBg);
  osd->DrawText(x3, y0, Text, ColorFg, ColorBg, cFont::GetFont(fontSml), x4 - x3, y1 - y0, taCenter);
  osd->DrawRectangle(x5, y0, x6 - 1, y1 - 1, ColorBg);
  osd->DrawRectangle(x6, y0, x7 - 1, y1 - 1, clrTransparent);
  osd->DrawEllipse  (x6, y0, x7 - 1, y1 - 1, ColorBg, 5);
}

void cLCARSNGDisplayMessage::Flush(void)
{
  osd->Flush();
}
