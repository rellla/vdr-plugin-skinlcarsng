#include "messagebox.h"
#include "volumebox.h"

typedef struct {
  const cEvent *Event = NULL;
  int x0 = 0;
  int x1 = 0;
  int y0 = 0;
  int y1 = 0;
  int d = 0;
  int zoom = 0;
  tColor textColorBg, shortTextColorFg, frameColorBr, frameColorBg;
} AnimatedChannelInfo_t;

class cDrawChannelDescription {
private:
  cOsd *osd;
  AnimatedChannelInfo_t aI;
  int Margin;
  bool dowait = false;
  uint64_t StartTime = 0;
  cPixmap *BracketPixmap = NULL;
  cPixmap *TextPixmap = NULL;
  cTextWrapper wrapper;
  void DrawBracket(int lines = 0);
  void Draw(void);
public:
  cDrawChannelDescription(cOsd *osd, AnimatedChannelInfo_t animatedInfo);
  virtual ~cDrawChannelDescription();
  void Animate(void);
  };

// --- cLCARSNGDisplayChannel ----------------------------------------------

#ifdef USE_ZAPCOCKPIT
class cLCARSNGDisplayChannel : public cSkinDisplayChannelExtended, cThread {
#else
class cLCARSNGDisplayChannel : public cSkinDisplayChannel, cThread {
#endif
private:
  cOsd *osd;
  int xc00, xc01, xc02, xc03, xc04, xc05, xc06, xc07, xc08, xc09, xc10, xc11, xc12, xc13, xc14, xc15, xc16, xc17, xc18, xc19, xc20, xc21, xc22, xc23;
  int yc00, yc01, yc02, yc03, yc04, yc05, yc06, yc07, yc08, yc09, yc10, yc11, yc12, yc13, yc14;
  int xv00, xv01, yv00, yv01;
  int xs; // starting column for signal display
  bool withInfo;
  int iconHeight;
  int lineHeight;
  int smlLineHeight;
  int zoom;
  bool lastOn;
  bool On;
  int Margin;
  int lastVolume;
  time_t lastVolumeTime;
  AnimatedChannelInfo_t animatedInfo;
  cDrawChannelDescription *drawDescription = NULL;
  cLCARSNGMessageBox *messageBox = NULL;
  cLCARSNGVolumeBox *volumeBox = NULL;
  uint64_t StartTime = 0;
  const cFont *osdFont;
  const cFont *tinyFont;
  const cFont *tallFont;
  const cFont *smlFont;
  tColor frameColorFg;
  tColor frameColorBg;
  tColor frameColorBr;
  tColor textColorBg;
  int textBorder;
  cString oldResolution;
  int oldZapcockpitUseInfo = 0;
  int leftIcons;
  bool message;
  const cEvent *present;
  const cEvent *following;
  bool initial;
  cString lastDate;
  int lastSeen;
  int lastCurrentPosition;
  int lastDeviceNumber;
  cString lastDeviceType;
  cCamSlot *lastCamSlot;
  int lastSignalStrength;
  int lastSignalQuality;
  time_t lastSignalDisplay;
  tTrackId lastTrackId;
  static cBitmap bmTeletext, bmRadio, bmAudio, bmDolbyDigital, bmEncrypted, bmRecording;
  cTextWrapper wrapper;
  int GetLiveBuffer(void);
  void Action(void);
  void DrawDate(void);
  void DrawTrack(void);
  void DrawSeen(int Current, int Total);
  void DrawDevice(void);
  void DrawSignal(void);
  void DrawScreenResolution(void);
  void DrawBlinkingRec(void);
  void DrawEventRec(const cEvent *Present, const cEvent *Following);
  void DrawTimer(void);
  void DrawVolume(void);
  void SetInfo(bool showInfo);
#ifdef DRAWGRID
  void DrawGrid(void);
#endif
#ifdef USE_ZAPCOCKPIT
/*  cVeDcChannelHints           *veChannelHints;
  cVeDcChannelDetail          *veChannelDetail;
  cVeDcChannelListDetail      *veChannelListDetail;
  cVeDcGroupChannelListDetail *veGroupChannelListDetail;
  cViewListChannelList        *channelList;
  cViewListGroupList          *groupList;
  cViewListChannelList        *groupChannelList;*/
  eDisplaychannelView viewType;
  eDisplaychannelView viewTypeLast;
//  bool initExtended;
  bool displayList;
  bool initList;
  bool channelHints;
  bool channelInput;
#endif
public:
  cLCARSNGDisplayChannel(bool WithInfo);
  virtual ~cLCARSNGDisplayChannel();
  virtual void SetChannel(const cChannel *Channel, int Number);
  virtual void SetEvents(const cEvent *Present, const cEvent *Following);
  virtual void SetMessage(eMessageType Type, const char *Text);
#if APIVERSNUM > 20101
  virtual void SetPositioner(const cPositioner *Positioner);
#endif
#ifdef USE_ZAPCOCKPIT
  virtual void SetViewType(eDisplaychannelView ViewType);
  virtual int MaxItems(void);
  virtual bool KeyRightOpensChannellist(void);
  virtual void SetChannelInfo(const cChannel *Channel);
  virtual void SetChannelList(const cChannel *Channel, int Index, bool Current);
  virtual void SetGroupList(const char *Group, int NumChannels, int Index, bool Current);
  virtual void SetGroupChannelList(const cChannel *Channel, int Index, bool Current);
  virtual void ClearList(void);
  virtual void SetNumChannelHints(int Num);
  virtual void SetChannelHint(const cChannel *Channel);
#endif
  virtual void Flush(void);
  };
