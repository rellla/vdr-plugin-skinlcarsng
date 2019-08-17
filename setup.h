#include <vdr/menu.h>
//#include "config.h"

//extern cLCARSNGConfig config;

class cLCARSNGSetup : public cMenuSetupPage {
protected:
  virtual void Store(void);
private:
  cLCARSNGConfig tmpConfig;
public:
  cLCARSNGSetup(void);
};
