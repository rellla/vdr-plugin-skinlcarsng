#ifndef __SKINLCARSNG_TOOLS_H
#define __SKINLCARSNG_TOOLS_H

#include <vdr/font.h>
#include <vdr/plugin.h>

enum eMarginUsage {
  muNone     = 0x0000,
  muLeft     = 0x0001,
  muTop      = 0x0002,
  muRight    = 0x0004,
  muBottom   = 0x0008,
  };

typedef struct DrawPoster {
  cOsd *osd = NULL;
  const cEvent *Event = NULL;
  const cRecording *Recording = NULL;
  int layer = 3;
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
  int border = 0;
} DrawPoster_t;

cFont *CreateTinyFont(int LineHeight);
cOsd *CreateOsd(int Left, int Top, int x0, int y0, int x1, int y1);
cPixmap *CreatePixmap(cOsd *osd, cString Name = "", int Layer = 0, const cRect &ViewPort = cRect::Null, const cRect &DrawPort = cRect::Null);
cPixmap *DrawPoster(DrawPoster_t dP);
bool DrawDeviceData(cPixmap *Pixmap, const cDevice *Device, int x, int y, int w, int h, int &xs, const cFont *TinyFont, cString &LastDeviceType, cCamSlot *&LastCamSlot, bool Initial);
bool DrawDeviceData(cOsd *Osd, const cDevice *Device, int x0, int y0, int x1, int y1, int &xs, const cFont *TinyFont, cString &LastDeviceType, cCamSlot *&LastCamSlot, bool Initial);
void DrawDeviceSignal(cPixmap *Pixmap, const cDevice *Device, int x, int y, int w, int h, int &LastSignalStrength, int &LastSignalQuality, bool Initial);
void DrawDeviceSignal(cOsd *Osd, const cDevice *Device, int x0, int y0, int x1, int y1, int &LastSignalStrength, int &LastSignalQuality, bool Initial);
void DrawDevicePosition(cPixmap *Pixmap, const cPositioner *Positioner, int x0, int y0, int x1, int y1, int &LastCurrent);
void DrawDevicePosition(cOsd *Osd, const cPositioner *Positioner, int x0, int y0, int x1, int y1, int &LastCurrent);
void PixmapSetAlpha(cPixmap *Pixmap, int Alpha);
void PixmapFill(cPixmap *Pixmap, tColor Color);
void DrawBitmap(cPixmap *Pixmap, int x, int y, const cBitmap &Bitmap, tColor ColorFg = 0, tColor ColorBg = 0, bool Overlay = false);
void DrawText(cPixmap *Pixmap, int x, int y, const char *s, tColor ColorFg, tColor ColorBg, const cFont *Font, int Width = 0, int Height = 0, int Alignment = taDefault);
void DrawRectangle(cPixmap *Pixmap, int x1, int y1, int x2, int y2, tColor Color);
void DrawRectangleOutline(cOsd *Osd, int x1, int y1, int x2, int y2, tColor Color1, tColor Color2, int Usage = 0, int Margin = 0);
void DrawRectangleOutline(cPixmap *Pixmap, int x, int y, int w, int h, tColor Color1, tColor Color2, int Usage = 0, int Margin = 0);
void DrawEllipseOutline(cOsd *Osd, int x1, int y1, int x2, int y2, tColor Color1, tColor Color2, int Quadrants = 0, int Margin = 0);
void DrawEllipseOutline(cPixmap *Pixmap, int x, int y, int w, int h, tColor Color1, tColor Color2, int Quadrants = 0, int Margin = 0);
void DrawProgressbar(cPixmap *p, int left, int top, int width, int height, int Current, int Total, tColor color, tColor colorBlending, bool blend = false, bool partial = false);
int GetFrameAfterEdit(const cMarks *marks = NULL, int Frame = 0, int LastFrame = 0);
int FreeMB(const char *Base, bool menurecording = false);
std::string StripXmlTag(std::string &Line, const char *Tag);
cString GetScreenResolutionIcon(void);
cPlugin *GetScraperPlugin(void);
int GetLiveBuffer(void);

void DrawDate(cOsd *osd, int x0, int y0, int x1, int y1, tColor colorfg, tColor colorbg, const cFont *font, cString *lastDate);
void DrawDate(cPixmap *pixmap, int x0, int y0, int x1, int y1, tColor colorfg, tColor colorbg, const cFont *font, cString *lastDate);
void DrawTrack(cOsd *osd, int x0, int y0, int x1, int y1, tColor colorfg, tColor colorbg, const cFont *font, tTrackId *lastTrackId);
void DrawTrack(cPixmap *pixmap, int x0, int y0, int x1, int y1, tColor colorfg, tColor colorbg, const cFont *font, tTrackId *lastTrackId);
void DrawScreenResolution(cOsd *osd, int x0, int y0, int x1, int y1, tColor colorfg, tColor colorbg, const cFont *font, cString *oldResolution);
void DrawScreenResolution(cPixmap *pixmap, int x0, int y0, int x1, int y1, tColor colorfg, tColor colorbg, const cFont *font, cString resolution, cString *oldResolution);

std::string StrToLowerCase(std::string str);

#endif //__SKINLCARSNG_TOOLS_H
