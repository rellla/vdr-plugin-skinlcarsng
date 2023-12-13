#include "services/scraper2vdr.h"
#include "imageloader.h"
#include <string>
#include <sstream>
#include <vector>
#include <vdr/skins.h>
#include "lcarsng.h"
#include "tools.h"
#include "config.h"

// --- Helper functions ------------------------------------------------------

cFont *CreateTinyFont(int LineHeight)
{
  // Creates a font that is not higher than half of LineHeight.
  LineHeight /= 1.85;
  int Height = LineHeight;
  for (;;) {
      cFont *TinyFont = cFont::CreateFont(Setup.FontOsd, Height);
      if (Height < 2 || TinyFont->Height() <= LineHeight)
         return TinyFont;
      delete TinyFont;
      Height -= 1;
      }
}

cOsd *CreateOsd(int Left, int Top, int x0, int y0, int x1, int y1)
{
  cOsd *Osd = cOsdProvider::NewOsd(Left, Top);
  int Bpp[] = { 32, 8, 4, 2, 1 };
  tArea Area = { x0, y0, x1, y1, 0 };
  for (unsigned int i = 0; i < sizeof(Bpp) / sizeof(int); i++) {
      Area.bpp = Bpp[i];
      if (Osd->CanHandleAreas(&Area, 1) == oeOk) {
         Osd->SetAreas(&Area, 1);
         Osd->SetAntiAliasGranularity(20, 16);
         TwoColors = Area.bpp == 1;
         break;
         }
      }
  return Osd;
}

cPixmap *CreatePixmap(cOsd *osd, cString Name, int Layer, const cRect &ViewPort, const cRect &DrawPort) {
    if (osd) {
        if (cPixmap *pixmap = osd->CreatePixmap(Layer, ViewPort, DrawPort)) {
            return pixmap;
        } else {
            esyslog("skinlcarsng: Could not create pixmap \"%s\" of size %i x %i", *Name, DrawPort.Size().Width(), DrawPort.Size().Height());
            cRect NewDrawPort = DrawPort;
            int width = std::min(DrawPort.Size().Width(), osd->MaxPixmapSize().Width());
            int height = std::min(DrawPort.Size().Height(), osd->MaxPixmapSize().Height());
            NewDrawPort.SetSize(width, height);
            if (cPixmap *pixmap = osd->CreatePixmap(Layer, ViewPort, NewDrawPort)) {
                esyslog("skinlcarsng: Create pixmap \"%s\" of reduced size %i x %i", *Name, width, height);
                return pixmap;
            } else {
                esyslog("skinlcarsng: Could not create pixmap \"%s\" of reduced size %i x %i", *Name, width, height);
            }
        }
    }
    return NULL;
}

cPixmap *DrawPoster(DrawPoster_t dP) {

  if (!dP.Event && !dP.Recording)
     return NULL;

  static cPlugin *pScraper = GetScraperPlugin();
  if (!pScraper)
     return NULL;

  ScraperGetPoster call;
  call.event = dP.Event;
  call.recording = dP.Recording;
  if (!pScraper->Service("GetPoster", &call))
     return NULL;

  if (!(call.poster.path.size() > 0 && call.poster.height > 0))
     return NULL;

  cImageLoader imgLoader;
  if (!imgLoader.LoadPoster(call.poster.path.c_str(), dP.width - 2 * dP.border, dP.height - 2 * dP.border))
     return NULL;

  cImage logo = imgLoader.GetImage();

  dP.x = dP.x + dP.width - logo.Width() - 2 * dP.border;
  dP.y = dP.y + (dP.height - logo.Height() - 2 * dP.border) / 2;

  cPixmap *PosterPixmap = CreatePixmap(dP.osd, "PosterPixmap", 3,
                                       cRect(dP.x,
                                             dP.y,
                                             logo.Width() + 2 * dP.border,
                                             logo.Height() + 2 * dP.border));
  if (!PosterPixmap)
     return NULL;

  PosterPixmap->SetAlpha(0);
  PosterPixmap->Fill(clrTransparent);
  PosterPixmap->DrawImage(cPoint(dP.border, dP.border), logo);

  return PosterPixmap;
}

bool DrawDeviceData(cPixmap *Pixmap, const cDevice *Device, int x, int y, int w, int h, int &xs, const cFont *TinyFont, cString &LastDeviceType, cCamSlot *&LastCamSlot, bool Initial)
{
  cString DeviceType = Device->DeviceType();
  cCamSlot *CamSlot = Device->CamSlot();
  if (Initial || strcmp(DeviceType, LastDeviceType) || CamSlot != LastCamSlot) {
     const cFont *font = cFont::GetFont(fontOsd);
     tColor ColorFg = Theme.Color(clrDeviceFg);
     tColor ColorBg = Theme.Color(clrDeviceBg);
     Pixmap->DrawRectangle(cRect(x, y, w, h), ColorBg);
     // Device number:
     cString Nr = itoa(Device->DeviceNumber() + 1);
     int W = max(font->Width(Nr), h);
     Pixmap->DrawText(cPoint(x, y), Nr, ColorFg, ColorBg, font, W, h, taCenter);
     x += W;
     // Device type:
     Pixmap->DrawText(cPoint(x, y), DeviceType, ColorFg, ColorBg, TinyFont);
     xs = max(xs, W + TinyFont->Width(DeviceType));
     LastDeviceType = DeviceType;
     // CAM:
     if (CamSlot) {
#if APIVERSNUM > 20302
        cString s = cString::sprintf("CAM %d", CamSlot->MasterSlotNumber());
#else
        cString s = cString::sprintf("CAM %d", CamSlot->SlotNumber());
#endif
        Pixmap->DrawText(cPoint(x, y + h - TinyFont->Height()), s, ColorFg, ColorBg, TinyFont);
        xs = max(xs, W + TinyFont->Width(s));
        }
     LastCamSlot = CamSlot;
     return true;
     }
  return false;
}

bool DrawDeviceData(cOsd *Osd, const cDevice *Device, int x0, int y0, int x1, int y1, int &xs, const cFont *TinyFont, cString &LastDeviceType, cCamSlot *&LastCamSlot, bool Initial)
{
  cString DeviceType = Device->DeviceType();
  cCamSlot *CamSlot = Device->CamSlot();
  if (Initial || strcmp(DeviceType, LastDeviceType) || CamSlot != LastCamSlot) {
     const cFont *font = cFont::GetFont(fontOsd);
     tColor ColorFg = Theme.Color(clrDeviceFg);
     tColor ColorBg = Theme.Color(clrDeviceBg);
     Osd->DrawRectangle(x0, y0, x1 - 1, y1 - 1, ColorBg);
     int x = x0;
     // Device number:
     cString Nr = itoa(Device->DeviceNumber() + 1);
     int w = max(font->Width(Nr), y1 - y0);
     Osd->DrawText(x, y0, Nr, ColorFg, ColorBg, font, w, y1 - y0, taCenter);
     x += w;
     // Device type:
     Osd->DrawText(x, y0, DeviceType, ColorFg, ColorBg, TinyFont);
     xs = max(xs, x + TinyFont->Width(DeviceType));
     LastDeviceType = DeviceType;
     // CAM:
     if (CamSlot) {
#if APIVERSNUM > 20302
        cString s = cString::sprintf("CAM %d", CamSlot->MasterSlotNumber());
#else
        cString s = cString::sprintf("CAM %d", CamSlot->SlotNumber());
#endif
        Osd->DrawText(x, y1 - TinyFont->Height(), s, ColorFg, ColorBg, TinyFont);
        xs = max(xs, x + TinyFont->Width(s));
        }
     LastCamSlot = CamSlot;
     return true;
     }
  return false;
}

void DrawDeviceSignal(cPixmap *Pixmap, const cDevice *Device, int x, int y, int w, int h, int &LastSignalStrength, int &LastSignalQuality, bool Initial)
{
  if (!Pixmap)
     return;

  int SignalStrength = Device->SignalStrength();
  int SignalQuality = Device->SignalQuality();
  int d = max(h / 10, 1);
  int x00 = x + d;
  int x01 = w - d;
  int H = (h - 3 * d) / 2;
  int y00 = y + d;
  int y03 = y + h - d;
  int y02 = y03 - H;
  tColor ColorSignalValue, ColorSignalRest;
  if (TwoColors) {
     ColorSignalValue = Theme.Color(clrBackground);
     ColorSignalRest = Theme.Color(clrMenuFrameBg);
     }
  else {
     ColorSignalValue = Theme.Color(clrSignalValue);
     ColorSignalRest = Theme.Color(clrSignalRest);
     }
  if (SignalStrength >= 0 && (Initial || SignalStrength != LastSignalStrength)) {
     int s = SignalStrength * w / 100;
     Pixmap->DrawRectangle(cRect(x00, y00, s - d, H), ColorSignalValue);
     Pixmap->DrawRectangle(cRect(x00 + s, y00, x01 - s - d, H), ColorSignalRest);
     LastSignalStrength = SignalStrength;
     }
  if (SignalQuality >= 0 && (Initial || SignalQuality != LastSignalQuality)) {
     int q = SignalQuality * w / 100;
     Pixmap->DrawRectangle(cRect(x00, y02, q - d, H), ColorSignalValue);
     Pixmap->DrawRectangle(cRect(x00 + q, y02, x01 - q - d, H), ColorSignalRest);
     LastSignalQuality = SignalQuality;
     }
}

void DrawDeviceSignal(cOsd *Osd, const cDevice *Device, int x0, int y0, int x1, int y1, int &LastSignalStrength, int &LastSignalQuality, bool Initial)
{
  int SignalStrength = Device->SignalStrength();
  int SignalQuality = Device->SignalQuality();
  int d = max((y1 - y0) / 10, 1);
  int x00 = x0 + d;
  int x01 = x1 - d;
  int h = (y1 - y0 - 3 * d) / 2;
  int w = x01 - x00;
  int y00 = y0 + d;
  int y01 = y00 + h;
  int y03 = y1 - d;
  int y02 = y03 - h;
  tColor ColorSignalValue, ColorSignalRest;
  if (TwoColors) {
     ColorSignalValue = Theme.Color(clrBackground);
     ColorSignalRest = Theme.Color(clrMenuFrameBg);
     }
  else {
     ColorSignalValue = Theme.Color(clrSignalValue);
     ColorSignalRest = Theme.Color(clrSignalRest);
     }
  if (SignalStrength >= 0 && (Initial || SignalStrength != LastSignalStrength)) {
     int s = SignalStrength * w / 100;
     Osd->DrawRectangle(x00, y00, x00 + s - 1, y01 - 1, ColorSignalValue);
     Osd->DrawRectangle(x00 + s, y00, x01 - 1, y01 - 1, ColorSignalRest);
     LastSignalStrength = SignalStrength;
     }
  if (SignalQuality >= 0 && (Initial || SignalQuality != LastSignalQuality)) {
     int q = SignalQuality * w / 100;
     Osd->DrawRectangle(x00, y02, x00 + q - 1, y03 - 1, ColorSignalValue);
     Osd->DrawRectangle(x00 + q, y02, x01 - 1, y03 - 1, ColorSignalRest);
     LastSignalQuality = SignalQuality;
     }
}

#if APIVERSNUM > 20101
void DrawDevicePosition(cOsd *Osd, const cPositioner *Positioner, int x0, int y0, int x1, int y1, int &LastCurrent)
{
  int HorizonLeft = Positioner->HorizonLongitude(cPositioner::pdLeft);
  int HorizonRight = Positioner->HorizonLongitude(cPositioner::pdRight);
  int HardLimitLeft = cPositioner::NormalizeAngle(HorizonLeft - Positioner->HardLimitLongitude(cPositioner::pdLeft));
  int HardLimitRight = cPositioner::NormalizeAngle(Positioner->HardLimitLongitude(cPositioner::pdRight) - HorizonRight);
  int HorizonDelta = cPositioner::NormalizeAngle(HorizonLeft - HorizonRight);
  int Current = cPositioner::NormalizeAngle(HorizonLeft - Positioner->CurrentLongitude());
  int Target = cPositioner::NormalizeAngle(HorizonLeft - Positioner->TargetLongitude());
  int d = (y1 - y0) / 2;
  int w = x1 - x0 - 2 * d;
  int l = max(x0 + d, x0 + d + w * HardLimitLeft / HorizonDelta);
  int r = min(x1 - d, x1 - d - w * HardLimitRight / HorizonDelta) - 1;
  int c = constrain(x0 + d + w * Current / HorizonDelta, l, r);
  int t = constrain(x0 + d + w * Target / HorizonDelta, l, r);
  if (c == LastCurrent)
     return;
  if (c > t)
     swap(c, t);
  tColor ColorRange, ColorMove;
  if (TwoColors) {
     ColorRange = Theme.Color(clrChannelFrameBg);
     ColorMove = Theme.Color(clrBackground);
     }
  else {
     ColorRange = Theme.Color(clrChannelFrameBg);
     ColorMove = Theme.Color(clrDeviceBg);
     }
  Osd->DrawRectangle(x0, y0, x1 - 1, y1 - 1, Theme.Color(clrBackground));
  Osd->DrawEllipse(l - d, y0, l, y1 - 1, ColorRange, 7);
  Osd->DrawRectangle(l, y0, r, y1 - 1, ColorRange);
  Osd->DrawEllipse(r, y0, r + d, y1 - 1, ColorRange, 5);
  Osd->DrawEllipse(c - d, y0, c, y1 - 1, ColorMove, 7);
  Osd->DrawRectangle(c, y0, t, y1 - 1, ColorMove);
  Osd->DrawEllipse(t, y0, t + d, y1 - 1, ColorMove, 5);
  LastCurrent = c;
}

void DrawDevicePosition(cPixmap *Pixmap, const cPositioner *Positioner, int x0, int y0, int x1, int y1, int &LastCurrent)
{
  int HorizonLeft = Positioner->HorizonLongitude(cPositioner::pdLeft);
  int HorizonRight = Positioner->HorizonLongitude(cPositioner::pdRight);
  int HardLimitLeft = cPositioner::NormalizeAngle(HorizonLeft - Positioner->HardLimitLongitude(cPositioner::pdLeft));
  int HardLimitRight = cPositioner::NormalizeAngle(Positioner->HardLimitLongitude(cPositioner::pdRight) - HorizonRight);
  int HorizonDelta = cPositioner::NormalizeAngle(HorizonLeft - HorizonRight);
  int Current = cPositioner::NormalizeAngle(HorizonLeft - Positioner->CurrentLongitude());
  int Target = cPositioner::NormalizeAngle(HorizonLeft - Positioner->TargetLongitude());
  int d = (y1 - y0) / 2;
  int w = x1 - x0 - 2 * d;
  int l = max(x0 + d, x0 + d + w * HardLimitLeft / HorizonDelta);
  int r = min(x1 - d, x1 - d - w * HardLimitRight / HorizonDelta) - 1;
  int c = constrain(x0 + d + w * Current / HorizonDelta, l, r);
  int t = constrain(x0 + d + w * Target / HorizonDelta, l, r);
  if (c == LastCurrent)
     return;
  if (c > t)
     swap(c, t);
  tColor ColorRange, ColorMove;
  if (TwoColors) {
     ColorRange = Theme.Color(clrChannelFrameBg);
     ColorMove = Theme.Color(clrBackground);
     }
  else {
     ColorRange = Theme.Color(clrChannelFrameBg);
     ColorMove = Theme.Color(clrDeviceBg);
     }
  Pixmap->DrawRectangle(cRect(x0, y0, x1 - x0, y1 - y0), Theme.Color(clrBackground));
  //todo
  Pixmap->DrawEllipse(cRect(l - d, y0, l, y1 - 1), ColorRange, 7);
  Pixmap->DrawRectangle(cRect(l, y0, r, y1 - 1), ColorRange);
  Pixmap->DrawEllipse(cRect(r, y0, r + d, y1 - 1), ColorRange, 5);
  Pixmap->DrawEllipse(cRect(c - d, y0, c, y1 - 1), ColorMove, 7);
  Pixmap->DrawRectangle(cRect(c, y0, t, y1 - 1), ColorMove);
  Pixmap->DrawEllipse(cRect(t, y0, t + d, y1 - 1), ColorMove, 5);
  LastCurrent = c;
}
#endif

void PixmapSetAlpha(cPixmap *Pixmap, int Alpha)
{
  if (Pixmap)
     Pixmap->SetAlpha(Alpha);
}

void PixmapFill(cPixmap *Pixmap, tColor Color)
{
  if (Pixmap)
     Pixmap->Fill(Color);
}

void DrawBitmap(cPixmap *Pixmap, int x, int y, const cBitmap &Bitmap, tColor ColorFg, tColor ColorBg, bool Overlay)
{
  if (Pixmap)
     Pixmap->DrawBitmap(cPoint(x, y), Bitmap, ColorFg, ColorBg, Overlay);
}

void DrawText(cPixmap *Pixmap, int x, int y, const char *s, tColor ColorFg, tColor ColorBg, const cFont *Font, int Width, int Height, int Alignment)
{
  if (Pixmap)
     Pixmap->DrawText(cPoint(x, y), s, ColorFg, ColorBg, Font, Width, Height, Alignment);
}

void DrawRectangle(cPixmap *Pixmap, int x1, int y1, int x2, int y2, tColor Color)
{
  if (Pixmap)
     Pixmap->DrawRectangle(cRect(x1, y1, x2 - x1, y2 - y1), Color);
}

void DrawRectangleOutline(cOsd *Osd, int x1, int y1, int x2, int y2, tColor Color1, tColor Color2, int Usage, int Margin)
{
  /* Color1 = border color
   * Color2 = inner color
   */

  int margin = (Margin > 0) ? Margin : Config.Margin;
  Osd->DrawRectangle(x1, y1, x2, y2, (margin == 0) ? Color2 : Color1);
  if ((margin == 0) || !Usage || (Color1 == Color2))
     return;

  Osd->DrawRectangle(x1 + ((Usage & muLeft)   ? margin : 0),
                     y1 + ((Usage & muTop)    ? margin : 0),
                     x2 - ((Usage & muRight)  ? margin : 0),
                     y2 - ((Usage & muBottom) ? margin : 0), Color2);
}

void DrawRectangleOutline(cPixmap *Pixmap, int x, int y, int x2, int y2, tColor Color1, tColor Color2, int Usage, int Margin)
{
  if (!Pixmap)
     return;

  /* Color1 = border color
   * Color2 = inner color
   */

  int w = x2 - x;
  int h = y2 - y;
  int margin = (Margin > 0) ? Margin : Config.Margin;
  Pixmap->DrawRectangle(cRect(x, y, w, h), (margin == 0) ? Color2 : Color1);
  if ((margin == 0) || !Usage || (Color1 == Color2))
     return;

  Pixmap->DrawRectangle(cRect(x + ((Usage & muLeft) ? margin : 0), y + ((Usage & muTop)    ? margin : 0),
                              w - ((Usage & muLeft) ? margin : 0) -    ((Usage & muRight)  ? margin : 0),
                              h - ((Usage & muTop)  ? margin : 0) -    ((Usage & muBottom) ? margin : 0)),
                              Color2);
}

void DrawEllipseOutline(cOsd *Osd, int x1, int y1, int x2, int y2, tColor Color1, tColor Color2, int Quadrants, int Margin)
{
  /* Quadrants:
   * 0       draws the entire ellipse
   * 1..4    draws only the first, second, third or fourth quadrant, respectively
   * 5..8    draws the right, top, left or bottom half, respectively
   * -1..-4  draws the inverted part of the given quadrant
   *  Color1 = border color
   *  Color2 = inner color
   */

  int margin = (Margin > 0) ? Margin : Config.Margin;
  Osd->DrawEllipse(x1, y1, x2, y2, (margin == 0) ? Color2 : Color1, Quadrants);
  if (margin == 0)
     return;

  switch (Quadrants) {
             case  0: Osd->DrawEllipse(x1 + margin, y1 + margin, x2 - margin, y2 - margin, Color2, 0);                  // full ellipse
                      break;
             case  1: Osd->DrawEllipse(x1, y1 + margin, x2 - margin, y2, Color2, 1);                                    // top right
                      break;
             case  2: Osd->DrawEllipse(x1 + margin, y1 + margin, x2, y2, Color2, 2);                                    // top left
                      break;
             case  3: Osd->DrawEllipse(x1 + margin, y1, x2, y2 - margin, Color2, 3);                                    // bottom left
                      break;
             case  4: Osd->DrawEllipse(x1, y1, x2 - margin, y2 - margin, Color2, 4);                                    // bottom right
                      break;
             case  5: Osd->DrawEllipse(x1, y1 + margin, x2 - margin, y2 - margin, Color2, 5);                           // right
                      break;
             case  6: Osd->DrawEllipse(x1 + margin, y1 + margin, x2 - margin, y2, Color2, 6);                           // top
                      break;
             case  7: Osd->DrawEllipse(x1 + margin, y1 + margin, x2, y2 - margin, Color2, 7);                           // left
                      break;
             case  8: Osd->DrawEllipse(x1 + margin, y1, x2 - margin, y2 - margin, Color2, 8);                           // bottom
                      break;
             case -1: Osd->DrawEllipse(x1, y1 - margin, x2 + margin, y2, Color2, -1);                                   // top right invers
                      break;
             case -2: Osd->DrawEllipse(x1 - margin, y1 - margin, x2, y2, Color2, -2);                                   // top left invers
                      break;
             case -3: Osd->DrawEllipse(x1 - margin, y1, x2, y2 + margin, Color2, -3);                                   // bottom left invers
                      break;
             case -4: Osd->DrawEllipse(x1, y1, x2 + margin, y2 + margin, Color2, -4);                                   // bottom right invers
                      break;
             default: ;
             }
}

void DrawEllipseOutline(cPixmap *Pixmap, int x1, int y1, int x2, int y2, tColor Color1, tColor Color2, int Quadrants, int Margin)
{
  if (!Pixmap)
     return;

  /* Quadrants:
   * 0       draws the entire ellipse
   * 1..4    draws only the first, second, third or fourth quadrant, respectively
   * 5..8    draws the right, top, left or bottom half, respectively
   * -1..-4  draws the inverted part of the given quadrant
   *  Color1 = border color
   *  Color2 = inner color
   */

  int w = x2 - x1;
  int h = y2 - y1;
  int margin = (Margin > 0) ? Margin : Config.Margin;
  Pixmap->DrawEllipse(cRect(x1, y1, w, h), (margin == 0) ? Color2 : Color1, Quadrants);
  if (margin == 0)
     return;

  switch (Quadrants) {
             case  0: Pixmap->DrawEllipse(cRect(x1 + margin, y1 + margin, w - 2 * margin, h - 2 * margin), Color2, 0);  // full ellipse
                      break;
             case  1: Pixmap->DrawEllipse(cRect(x1, y1 + margin, w - margin, h - margin), Color2, 1);                   // top right
                      break;
             case  2: Pixmap->DrawEllipse(cRect(x1 + margin, y1 + margin, w - margin, h - margin), Color2, 2);          // top left
                      break;
             case  3: Pixmap->DrawEllipse(cRect(x1 + margin, y1, w - margin, h - margin), Color2, 3);                   // bottom left
                      break;
             case  4: Pixmap->DrawEllipse(cRect(x1, y1, w - margin, h - margin), Color2, 4);                            // bottom right
                      break;
             case  5: Pixmap->DrawEllipse(cRect(x1, y1 + margin, w - margin, h - 2 * margin), Color2, 5);               // right
                      break;
             case  6: Pixmap->DrawEllipse(cRect(x1 + margin, y1 + margin, w - 2 * margin, h - margin), Color2, 6);      // top
                      break;
             case  7: Pixmap->DrawEllipse(cRect(x1 + margin, y1 + margin, w - margin, h - 2 * margin), Color2, 7);      // left
                      break;
             case  8: Pixmap->DrawEllipse(cRect(x1 + margin, y1, w - 2 * margin, h - margin), Color2, 8);               // bottom
                      break;
             case -1: Pixmap->DrawEllipse(cRect(x1, y1 - margin, w + margin, h + margin), Color2, -1);                  // top right invers
                      break;
             case -2: Pixmap->DrawEllipse(cRect(x1 - margin, y1 - margin, w + margin, h + margin), Color2, -2);         // top left invers
                      break;
             case -3: Pixmap->DrawEllipse(cRect(x1 - margin, y1, w + margin, h + margin), Color2, -3);                  // bottom left invers
                      break;
             case -4: Pixmap->DrawEllipse(cRect(x1, y1, w + margin, h + margin), Color2, -4);                           // bottom right invers
                      break;
             default: ;
             }
}

void DrawProgressbar(cPixmap *p, int left, int top, int width, int height, int Current, int Total, tColor clr1, tColor clr2, bool blend, bool partial) {
    if (!p)
        return;

/*    if (Current == 0) {
        p->DrawEllipse(cRect(left, top, height, height), blend ? clr2 : clr1);
        return;
    } else
        p->DrawEllipse(cRect(left, top, height, height), clr2);
*/
    width = width - height;                             // width of gradient (width - ellipse)
    double percent = ((double)Current) / (double)Total;
    double progresswidth = width * percent;

    int alpha = 0x0;                                    // 0...255
    int alphaStep = 0x1;
    int maximumsteps = 256;                             // alphaStep * maximumsteps <= 256
    int factor = 2;                                     // max. 128 steps

    double step = 0;
    if (partial) {
        step = (double)width / maximumsteps;            // shows a progresswidth part of color gradient
        maximumsteps = (double)maximumsteps * percent;
    } else
        step = progresswidth / maximumsteps;            // shows a progresswidth full color gradient

    if (!partial && progresswidth < 128) {              // width < 128
        factor = 4 * factor;                            // 32 steps
    } else if (progresswidth < 256) {                   // width < 256
        factor = 2 * factor;                            // 64 steps
    }

    step = step * factor;
    alphaStep = alphaStep * factor;
    maximumsteps = maximumsteps / factor;

    tColor clr3 = blend ? clr2 : clr1;
    tColor clr = 0x00000000;
    int x = left + height / 2;
    for (int i = 0; i < maximumsteps; i++) {
        x = left + height / 2 + i * step;
        clr = AlphaBlend(clr3, clr1, alpha);
        p->DrawRectangle(cRect(x, top, step + 1, height), clr);
        alpha += alphaStep;
    }
    x = x + step - height / 2;
    p->DrawRectangle(cRect(x, top, step + 1, height), clr);
//    p->DrawEllipse(cRect(x, top, height, height), clr);
}

static time_t lastDiskSpaceCheck = 0;
static int lastFreeMB = -1;

int FreeMB(const char *Base, bool menurecording)
{
  if (!menurecording)
     return cVideoDiskUsage::FreeMinutes();

  bool Directory = false;
  char *currentBase = Base ? strdup(Base) : NULL;

  if (currentBase) {
     const char *p = strchr(currentBase, ' ');
     if (p) {
        int n = p - currentBase;
        if (n == 3 || n == 6) {
           strshift(currentBase, n + 1);
           }
        }
     Directory = (strcmp(currentBase, cString::sprintf("%s", trVDR("Recordings"))) && strcmp(currentBase, cString::sprintf("%s", trVDR("Deleted Recordings")))) ? true : false;
     }
  if (!Directory)
     return cVideoDiskUsage::FreeMinutes();

  cStateKey recordingsStateKey;
  if (lastFreeMB <= 0 || (time(NULL) - lastDiskSpaceCheck) > DISKSPACECHEK) {
     dev_t fsid = 0;
     int freediskspace = 0;
     std::string path = cVideoDirectory::Name();
     path += "/";
     char *tmpbase = Directory ? ExchangeChars(strdup(currentBase), true) : NULL;
     if (tmpbase)
        path += tmpbase;
     struct stat statdir;
     if (!stat(path.c_str(), &statdir)) {
        if (statdir.st_dev != fsid) {
           fsid = statdir.st_dev;
           struct statvfs fsstat;
           if (!statvfs(path.c_str(), &fsstat)) {
              freediskspace = int((double)fsstat.f_bavail / (double)(1024.0 * 1024.0 / fsstat.f_bsize));
              if (const cRecordings *DeletedRecordings = cRecordings::GetDeletedRecordingsRead(recordingsStateKey)) {
                 for (const cRecording *rec = DeletedRecordings->First(); rec; rec = DeletedRecordings->Next(rec)) {
                    if (!stat(rec->FileName(), &statdir)) {
                       if (statdir.st_dev == fsid) {
                          int ds = DirSizeMB(rec->FileName());
                          if (ds > 0)
                             freediskspace += ds;
                          else
                             esyslog("DirSizeMB(%s) failed!", rec->FileName());
                          }
                       }
                    }
                 recordingsStateKey.Remove();
                 }
              }
           else {
              dsyslog("Error while getting filesystem size - statvfs (%s): %s", path.c_str(), strerror(errno));
              freediskspace = 0;
              }
           }
        else {
           freediskspace = lastFreeMB;
           }
        }
     else {
        dsyslog("Error while getting filesystem size - stat (%s): %s", path.c_str(), strerror(errno));
        freediskspace = 0;
        }
     free(tmpbase);
     lastFreeMB = freediskspace;
     lastDiskSpaceCheck = time(NULL);
     }
  free(currentBase);
  if (lastFreeMB == 0)
     return cVideoDiskUsage::FreeMinutes();
  LOCK_RECORDINGS_READ;
  double MBperMinute = Recordings->MBperMinute();
  return int(double(lastFreeMB) / (MBperMinute > 0 ? MBperMinute : MB_PER_MINUTE));
}

std::string StripXmlTag(std::string &Line, const char *Tag) {
  // set the search strings
  std::stringstream strStart, strStop;
  strStart << "<" << Tag << ">";
  strStop << "</" << Tag << ">";
  // find the strings
  std::string::size_type locStart = Line.find(strStart.str());
  std::string::size_type locStop = Line.find(strStop.str());
  if (locStart == std::string::npos || locStop == std::string::npos)
     return "";
  // extract relevant text
  int pos = locStart + strStart.str().size();
  int len = locStop - pos;
  return len < 0 ? "" : Line.substr(pos, len);
}

cString GetScreenResolutionIcon(void) {
  int screenWidth = 0;
  int screenHeight = 0;
  double aspect = 0;
  cDevice::PrimaryDevice()->GetVideoSize(screenWidth, screenHeight, aspect);
  cString iconName("");
//  esyslog ("skinlcarsng: %s %s %d screenWidth=%i screenHeight=%i\n", __FILE__, __func__,  __LINE__, screenWidth, screenHeight);
  switch (screenHeight) {
     case 4320: // 7680 x 4320 = 8K UHD
     case 2160: // 3840 x 2160 = 4K UHD
                iconName = "UHD4k";
                break;
     case 1440: // 2560 x 1440 = QHD
     case 1152: // 2048 x 1152
     case 1080: // "HD1080i"; // 'i' is default, 'p' can't be detected currently
                iconName = "HD1080";
                break;
     case 768:  // 1366 x 768
     case 720:  // "HD720p"; // 'i' is not defined in standards
                iconName = "HD720p";
                break;
     case 576:  // "SD576i"; // assumed 'i'
                iconName = "SD576i";
                break;
     case 480:  // "SD480i"; // assumed 'i'
                iconName = "SD480i";
                break;
     default:   if (screenWidth > 4500)
                   iconName = "UHD8k";
                else if (screenWidth > 2100)
                   iconName = "UHD4k";
                else if (screenWidth > 1700)
                   iconName = "HD1080";
                else if (screenWidth > 1000)
                   iconName = "HD720p";
                else
                   iconName = "SD";
                break;
     }
  return iconName;
}

cPlugin *GetScraperPlugin(void) {
    static cPlugin *pScraper = cPluginManager::GetPlugin("scraper2vdr");
    if (!pScraper ) // if it doesn't exit, try tvscraper
        pScraper = cPluginManager::GetPlugin("tvscraper");
    return pScraper;
}

std::string StrToLowerCase(std::string str) {
    std::string lowerCase = str;
    const int length = lowerCase.length();
    for(int i=0; i < length; ++i) {
        lowerCase[i] = std::tolower(lowerCase[i]);
    }
    return lowerCase;
}
