#include <vdr/menu.h>

class cLCARSNGSetup : public cMenuSetupPage {
private:
  const char *menuView[4];
protected:
  virtual void Store(void);
public:
  cLCARSNGSetup(void);
};
