#include "lcarsng.h"
#include <vdr/font.h>

// --- cLCARSNGDisplayVolume -----------------------------------------------

class cLCARSNGDisplayVolume : public cSkinDisplayVolume {
private:
  cOsd *osd;
  int Margin;
  int x0, x1, x2, x3, x4, x5, x6, x7;
  int y0, y1;
  tColor frameColor;
  int mute;
public:
  cLCARSNGDisplayVolume(void);
  virtual ~cLCARSNGDisplayVolume();
  virtual void SetVolume(int Current, int Total, bool Mute);
  virtual void Flush(void);
  };
