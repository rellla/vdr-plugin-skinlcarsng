#include <vdr/menu.h>

class cLCARSNGSetup : public cMenuSetupPage {
private:
  cString spacer;
  const char *menuView[5];
  const char *volumeBar[3];
  const char *infoChannel[3];
  const char *replayEnd[2];
  const char *replayCutted[3];
  const char *zoomLevel[4];
  int oldcategorie = 0;
  int categorie = 0;
protected:
  virtual void Store(void);
public:
  cLCARSNGSetup(void);
  eOSState ProcessKey(eKeys Key);
  void Set(void);
};
