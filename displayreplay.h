/*
 * skinlcars.c: A VDR skin with Star Trek's "LCARS" layout
 *
 * See the main source file 'vdr.c' for copyright information and
 * how to reach the author.
 *
 * $Id: skinlcars.c 4.1 2015/09/01 10:07:07 kls Exp $
 */

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

// --- cLCARSNGDisplayReplay -----------------------------------------------

class cLCARSNGDisplayReplay : public cSkinDisplayReplay, cThread {
private:
  cOsd *osd;
  const cFont *font;
  int xp00, xp01, xp02, xp03, xp04, xp05, xp06, xp07, xp08, xp09, xp10, xp11, xp12, xp13, xp14, xp15;
  int yp00, yp01, yp02, yp03, yp04, yp05, yp06, yp07, yp08, yp09;
  bool modeOnly;
  int iconHeight;
  int lineHeight;
  tColor frameColorFg;
  tColor frameColorBg;
  tColor frameColorMg;
  int lastCurrentWidth;
  int lastTotalWidth;
  cString lastDate;
  tTrackId lastTrackId;
  bool isRecording;
  int framesTotal;
  double fps;
  cString endTime;
  static cBitmap bmTeletext, bmRadio, bmAudio, bmDolbyDigital, bmEncrypted, bmRecording;
  void Action(void);
  void DrawDate(void);
  void DrawTrack(void);
  void DrawBlinkingRec(void);
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
