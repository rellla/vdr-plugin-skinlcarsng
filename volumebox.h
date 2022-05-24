#ifndef __LCARSNG_VOLUMEBOX_H
#define __LCARSNG_VOLUMEBOX_H

#include <vdr/skins.h>

class cLCARSNGVolumeBox {
private:
  cOsd *osd;
  cPixmap *pixmap;
  cPixmap *pixmapBackground;
  int x0, x1, x2, x3, x4, x5, x6, x7;
  int y0, y1;
  tColor frameColor;
  int mute;
public:
  cLCARSNGVolumeBox(cOsd *Osd, const cRect &Rect);
  ~cLCARSNGVolumeBox();
  void SetVolume(int Current = 0, int Total = 0, bool Mute = false);
  };

#endif //__LCARSNG_VOLUMEBOX_H
