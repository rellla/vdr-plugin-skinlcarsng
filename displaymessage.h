#include "messagebox.h"

// --- cLCARSNGDisplayMessage ----------------------------------------------

class cLCARSNGDisplayMessage : public cSkinDisplayMessage {
private:
  cOsd *osd;
  cLCARSNGMessageBox *messageBox = NULL;
public:
  cLCARSNGDisplayMessage(void);
  virtual ~cLCARSNGDisplayMessage();
  virtual void SetMessage(eMessageType Type, const char *Text);
  virtual void Flush(void);
  };
