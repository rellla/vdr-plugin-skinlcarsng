#include "lcarsng.h"
#include <vdr/font.h>

// --- cLCARSNGDisplayMessage ----------------------------------------------

class cLCARSNGDisplayMessage : public cSkinDisplayMessage {
private:
  cOsd *osd;
  int x0, x1, x2, x3, x4, x5, x6, x7;
  int y0, y1;
public:
  cLCARSNGDisplayMessage(void);
  virtual ~cLCARSNGDisplayMessage();
  virtual void SetMessage(eMessageType Type, const char *Text);
  virtual void Flush(void);
  };
