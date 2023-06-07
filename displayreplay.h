#include "messagebox.h"
#include "volumebox.h"

// --- cLCARSNGDisplayReplay -----------------------------------------------

class cLCARSNGDisplayReplay : public cSkinDisplayReplay, cThread {
private:
  cOsd *osd;
  const cFont *font;
  int Margin;
  int xp00, xp01, xp02, xp03, xp04, xp05, xp06, xp07, xp08, xp09, xp10, xp11, xp12, xp13, xp14, xp15;
  int yp00A, yp00, yp01, yp02, yp03, yp04, yp05, yp06, yp07, yp08, yp09;
  int xv00, xv01, yv00, yv01;
  bool modeOnly;
  int iconHeight;
  int lineHeight;
  int current = 0;
  int total = 0;
  tColor frameColorFg;
  tColor frameColorBg;
  tColor frameColorBr;
  tColor textColorBg;
  int lastCurrentWidth;
  int lastTotalWidth;
  int lastRestWidth;
  cString lastDate;
  tTrackId lastTrackId;
  int lastVolume;
  time_t lastVolumeTime;
  cLCARSNGMessageBox *messageBox = NULL;
  cLCARSNGVolumeBox *volumeBox = NULL;
  bool message;
  bool isRecording;
  int framesTotal;
  double fps;
  cString endTime;
  static cBitmap bmTeletext, bmRadio, bmAudio, bmDolbyDigital, bmEncrypted, bmRecording;
  void Action(void);
  void DrawDate(void);
  void DrawTrack(void);
  void DrawBlinkingRec(void);
  void DrawVolume(void);
  bool initial;
  bool lastOn;
  bool On;
  bool pbinit;
  bool timshiftMode;
public:
  cLCARSNGDisplayReplay(bool ModeOnly);
  virtual ~cLCARSNGDisplayReplay();
  virtual void SetRecording(const cRecording *Recording);
  virtual void SetTitle(const char *Title);
  virtual void SetMode(bool Play, bool Forward, int Speed);
  virtual void SetProgress(int Current, int Total);
  virtual void SetCurrent(const char *Current);
  virtual void SetTotal(const char *Total);
  virtual void SetJump(const char *Jump);
  virtual void SetMessage(eMessageType Type, const char *Text);
  virtual void Flush(void);
  };
