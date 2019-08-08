/*
 * skinlcars.c: A VDR skin with Star Trek's "LCARS" layout
 *
 * See the main source file 'vdr.c' for copyright information and
 * how to reach the author.
 *
 * $Id: skinlcars.c 4.1 2015/09/01 10:07:07 kls Exp $
 */

// "Star Trek: The Next Generation"(R) is a registered trademark of Paramount Pictures,
// registered in the United States Patent and Trademark Office, all rights reserved.
// The LCARS system is based upon the designs of Michael Okuda and his Okudagrams.
//
// "LCARS" is short for "Library Computer Access and Retrieval System".
// Some resources used for writing this skin can be found at
// http://www.lcars.org.uk
// http://www.lcarsdeveloper.com
// http://www.lcarscom.net
// http://lds-jedi.deviantart.com/art/LCARS-Swept-Tutorial-213936938
// http://lds-jedi.deviantart.com/art/LCARS-Button-Tutorial-210783437
// http://zelldenver.deviantart.com/art/LCARS-Color-Standard-179565780
// http://www.lcars47.com
// http://www.bracercom.com/tutorial/content/CoherentLCARSInterface/LCARSCoherentInterface.html
// http://www.bracercom.com/tutorial/content/lcars_manifesto/the_lcars_manifesto.html

#include "lcarsng.h"
#include <vdr/font.h>
#include <vdr/menu.h>
#include <vdr/osd.h>
#if APIVERSNUM > 20101
#include <vdr/positioner.h>
#endif
#include <vdr/themes.h>
#include <vdr/thread.h>
#include <vdr/tools.h>
#include <vdr/videodir.h>
#include <sys/statvfs.h>
#include <string>

// --- cLCARSNGDisplayMenu -------------------------------------------------

class cLCARSNGDisplayMenu : public cSkinDisplayMenu {
private:
  cOsd *osd;
  int xa00, xa01, xa02, xa03, xa04, xa05, xa06, xa07, xa08, xa09;
  int yt00, yt01, yt02, yt03, yt04, yt05, yt06, yt07, yt08, yt09, yt10;
  int yc00, yc01, yc02, yc03, yc04, yc05, yc06, yc07, yc08, yc09, yc10, yc11;
  int yb00, yb01, yb02, yb03, yb04, yb05, yb06, yb07, yb08, yb081, yb082, yb09, yb10, yb11, yb12, yb13, yb14, yb15;
  int xm00, xm01, xm02, xm03, xm04, xm05, xm06, xm07, xm08;
  int ym00, ym01, ym02, ym03, ym04, ym05, ym06, ym07;
  int xs00, xs01, xs02, xs03, xs04, xs05, xs06, xs07, xs08, xs09, xs10, xs11, xs12, xs13;
  int ys00, ys01, ys02, ys03, ys04, ys05;
  int xi00, xi01, xi02, xi03;
  int yi00, yi01;
  int xb00, xb01, xb02, xb03, xb04, xb05, xb06, xb07, xb08, xb09, xb10, xb11, xb12, xb13, xb14, xb15;
  int xd00, xd01, xd02, xd03, xd04, xd05, xd06, xd07;
  int yd00, yd01, yd02, yd03, yd04, yd05;
  int xs; // starting column for signal display
  int lineHeight;
  cFont *tinyFont;
  cFont *tallFont;
  tColor frameColorFg;
  tColor frameColorBg;
  tColor frameColorMg;
  int currentIndex;
  cVector<int> deviceOffset;
  cVector<bool> deviceRecording;
  cString lastDeviceType[MAXDEVICES];
  cVector<cCamSlot *> lastCamSlot;
  cVector<int> lastSignalStrength;
  cVector<int> lastSignalQuality;
  bool initial;
  bool videoScaled;
  enum eCurrentMode { cmUnknown, cmLive, cmPlay };
  eCurrentMode lastMode;
  cString lastDate;
  const char *currentTitle;
  int lastDiskUsageState;
  bool lastDiskAlert;
  double lastSystemLoad;
#if APIVERSNUM > 20300
  cStateKey timersStateKey;
#else
  int lastTimersState;
#endif
  time_t lastSignalDisplay;
  int lastLiveIndicatorY;
  bool lastLiveIndicatorTransferring;
  const cChannel *lastChannel;
  cString lastChannelName;
  const cEvent *lastEvent;
  const cRecording *lastRecording;
  cString lastHeader;
  int lastSeen;
  static cBitmap bmArrowUp, bmArrowDown, bmTransferMode;
  void DrawMainFrameUpper(tColor Color);
  void DrawMainFrameLower(void);
  void DrawMainFrameChannel(void);
  void DrawMainButton(const char *Text, int x0, int x1, int x2, int x3, int y0, int y1, tColor ColorFg, tColor ColorBg, const cFont *Font);
  void DrawMenuFrame(void);
  void DrawMainBracket(void);
  void DrawStatusElbows(void);
  void DrawDate(void);
  void DrawDisk(void);
  void DrawLoad(void);
  void DrawNumRecordingsInPath(void);
  void DrawCountRecordings(void);
  void DrawCountTimers(void);
  void DrawFrameDisplay(void);
  void DrawScrollbar(int Total, int Offset, int Shown, bool CanScrollUp, bool CanScrollDown);
  void DrawTimer(const cTimer *Timer, int y, bool MultiRec);
  void DrawTimers(void);
  void DrawDevice(const cDevice *Device);
  void DrawDevices(void);
  void DrawLiveIndicator(void);
  void DrawSignals(void);
  void DrawLive(const cChannel *Channel);
  void DrawPlay(cControl *Control);
  void DrawInfo(const cEvent *Event, bool WithTime);
  void DrawSeen(int Current, int Total);
  void DrawTextScrollbar(void);
public:
  cLCARSNGDisplayMenu(void);
  virtual ~cLCARSNGDisplayMenu();
  virtual void Scroll(bool Up, bool Page);
  virtual int MaxItems(void);
  virtual void Clear(void);
  virtual void SetMenuCategory(eMenuCategory MenuCategory);
  virtual void SetTitle(const char *Title);
  virtual void SetButtons(const char *Red, const char *Green = NULL, const char *Yellow = NULL, const char *Blue = NULL);
  virtual void SetMessage(eMessageType Type, const char *Text);
  virtual void SetItem(const char *Text, int Index, bool Current, bool Selectable);
  virtual void SetScrollbar(int Total, int Offset);
  virtual void SetEvent(const cEvent *Event);
  virtual void SetRecording(const cRecording *Recording);
  virtual void SetText(const char *Text, bool FixedFont);
  virtual int GetTextAreaWidth(void) const;
  virtual const cFont *GetTextAreaFont(bool FixedFont) const;
  virtual void Flush(void);
  };
