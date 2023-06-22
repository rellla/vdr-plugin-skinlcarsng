#include <vdr/menu.h>

class cLCARSNGSetup : public cMenuSetupPage {
private:
  const char *menuView[5];
  const char *volumeBar[3];
  const char *infoChannel[3];
protected:
  virtual void Store(void);
public:
  cLCARSNGSetup(void);
};
