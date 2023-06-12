#ifndef __LCARSNG_MESSAGEBOX_H
#define __LCARSNG_MESSAGEBOX_H

#include <vdr/skins.h>

class cLCARSNGMessageBox {
private:
  cOsd *osd;
  cPixmap *pixmap;
  cPixmap *pixmapBg;
  bool Full;
public:
  cLCARSNGMessageBox(cOsd *Osd, const cRect &Rect, bool Full = true);
  ~cLCARSNGMessageBox();
  void SetMessage(eMessageType Type, const char *Text);
  };

#endif //__LCARSNG_MESSAGEBOX_H
