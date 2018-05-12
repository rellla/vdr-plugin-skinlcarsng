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

// --- cLCARSNGDisplayChannel ----------------------------------------------

class cLCARSNGDisplayChannel : public cSkinDisplayChannel, cThread {
private:
  cOsd *osd;
  int xc00, xc00m, xc01, xc02, xc02m, xc03, xc04, xc05, xc06m, xc06n, xc06, xc07, xc08, xc09, xc10, xc10m, xc10n, xc11, xc12, xc13, xc14, xc15;
  int yc0B, yc0A, yc00, yc00m, yc01, yc02, yc03, yc04, yc05, yc06, yc07, yc08, yc09, yc10, yc11, yc12;
  int xs; // starting column for signal display
  bool withInfo;
  int iconHeight;
  int lineHeight;
  bool lastOn;
  bool On;
  cFont *tinyFont;
  cFont *tallFont;
  tColor frameColor;
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
  void Action(void);
  void DrawDate(void);
  void DrawTrack(void);
  void DrawSeen(int Current, int Total);
  void DrawDevice(void);
  void DrawSignal(void);
  void DrawBlinkingRec(void);
  void DrawEventRec(const cEvent *Present, const cEvent *Following);
public:
  cLCARSNGDisplayChannel(bool WithInfo);
  virtual ~cLCARSNGDisplayChannel();
  virtual void SetChannel(const cChannel *Channel, int Number);
  virtual void SetEvents(const cEvent *Present, const cEvent *Following);
  virtual void SetMessage(eMessageType Type, const char *Text);
#if APIVERSNUM > 20101
  virtual void SetPositioner(const cPositioner *Positioner);
#endif
  virtual void Flush(void);
  };
