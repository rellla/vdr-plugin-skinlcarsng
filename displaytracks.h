#include "lcarsng.h"
#include <vdr/font.h>
#include <vdr/menu.h>

// --- cLCARSNGDisplayTracks -----------------------------------------------

class cLCARSNGDisplayTracks : public cSkinDisplayTracks {
private:
  cOsd *osd;
  int xt00, xt01, xt02, xt03, xt04, xt05, xt06, xt07, xt08, xt09, xt10, xt11, xt12;
  int yt00, yt01, yt02, yt03, yt04, yt05, yt06, yt07;
  int lineHeight;
  tColor frameColorFg;
  tColor frameColorBg;
  tColor frameColorMg;
  int currentIndex;
  static cBitmap bmAudioLeft, bmAudioRight, bmAudioStereo;
  void SetItem(const char *Text, int Index, bool Current);
public:
  cLCARSNGDisplayTracks(const char *Title, int NumTracks, const char * const *Tracks);
  virtual ~cLCARSNGDisplayTracks();
  virtual void SetTrack(int Index, const char * const *Tracks);
  virtual void SetAudioChannel(int AudioChannel);
  virtual void Flush(void);
  };
