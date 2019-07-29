/*
 * skinlcars.h: A VDR skin with Star Trek's "LCARS" layout
 *
 * See the main source file 'vdr.c' for copyright information and
 * how to reach the author.
 *
 * $Id: skinlcars.h 3.0 2012/04/15 13:17:35 kls Exp $
 */

#ifndef __SKINLCARS_H
#define __SKINLCARS_H

#include <vdr/font.h>
#include <vdr/menu.h>
#include <vdr/osd.h>
#if APIVERSNUM > 20101
#include <vdr/positioner.h>
#endif
#include <vdr/skins.h>
#include <vdr/themes.h>
#include <vdr/thread.h>
#include <vdr/tools.h>
#include <vdr/videodir.h>
#include <sys/statvfs.h>
#include <string>

#include "symbols/arrowdown.xpm"
#include "symbols/arrowup.xpm"
#include "symbols/audio.xpm"
#include "symbols/audioleft.xpm"
#include "symbols/audioright.xpm"
#include "symbols/audiostereo.xpm"
#include "symbols/dolbydigital.xpm"
#include "symbols/encrypted.xpm"
#include "symbols/ffwd.xpm"
#include "symbols/ffwd1.xpm"
#include "symbols/ffwd2.xpm"
#include "symbols/ffwd3.xpm"
#include "symbols/frew.xpm"
#include "symbols/frew1.xpm"
#include "symbols/frew2.xpm"
#include "symbols/frew3.xpm"
#include "symbols/mute.xpm"
#include "symbols/pause.xpm"
#include "symbols/play.xpm"
#include "symbols/radio.xpm"
#include "symbols/recording.xpm"
#include "symbols/sfwd.xpm"
#include "symbols/sfwd1.xpm"
#include "symbols/sfwd2.xpm"
#include "symbols/sfwd3.xpm"
#include "symbols/srew.xpm"
#include "symbols/srew1.xpm"
#include "symbols/srew2.xpm"
#include "symbols/srew3.xpm"
#include "symbols/teletext.xpm"
#include "symbols/volume.xpm"

#define Gap            (Setup.FontOsdSize / 5 & ~1) // must be even
#define Margin         (Gap / 2)
#define TextFrame      (Setup.FontOsdSize / TEXT_ALIGN_BORDER)
#define TextSpacing    (2 * TextFrame)
#define SymbolSpacing  TextSpacing
#define ShowSeenExtent (Setup.FontOsdSize / 5) // pixels by which the "seen" bar extends out of the frame

#define MB_PER_MINUTE    25.75 // this is just an estimate! (taken over from VDR)
#define DISKSPACECHEK        5 // seconds between disk space checks
#define DISKUSAGEALERTLIMIT 95 // percent of disk usage above which the display goes into alert mode
#define SIGNALDISPLAYDELTA   2 // seconds between subsequent device signal displays

// Color domains:

#define CLR_BACKGROUND      0x99000000
#define CLR_MAIN_FRAME      0xFFFF9966
#define CLR_CHANNEL_FRAME   0xFF8A9EC9
#define CLR_REPLAY_FRAME    0xFFCC6666
#define CLR_DATE            0xFF99CCFF
#define CLR_MENU_ITEMS      0xFF9999FF
#define CLR_TIMER           0xFF99CCFF
#define CLR_DEVICE          0xFFF1B1AF
#define CLR_CHANNEL_NAME    0xFF99CCFF
#define CLR_EVENT_TITLE     0xFF99CCFF
#define CLR_EVENT_TIME      0xFFFFCC66
#define CLR_EVENT_SHORTTEXT 0xFFFFCC66
#define CLR_TEXT            0xFF99CCFF
#define CLR_TRACK           0xFFFFCC66
#define CLR_SEEN            0xFFCC99CC
#define CLR_ALERT           0xFFFF0000
#define CLR_EXPOSED         0xFF990000
#define CLR_WHITE           0xFFFFFFFF
#define CLR_RED             0xFFCC6666
#define CLR_GREEN           0xFFA0FF99
#define CLR_YELLOW          0xFFF1DF60
#define CLR_BLUE            0xFF9A99FF
#define CLR_BLACK           0xFF000000

extern cTheme Theme;

// General colors:

THEME_CLR(Theme, clrBackground,             CLR_BACKGROUND);
THEME_CLR(Theme, clrDateFg,                 CLR_BLACK);
THEME_CLR(Theme, clrDateBg,                 CLR_DATE);
THEME_CLR(Theme, clrTimerFg,                CLR_BLACK);
THEME_CLR(Theme, clrTimerBg,                CLR_TIMER);
THEME_CLR(Theme, clrDeviceFg,               CLR_BLACK);
THEME_CLR(Theme, clrDeviceBg,               CLR_DEVICE);
THEME_CLR(Theme, clrSignalValue,            CLR_GREEN);
THEME_CLR(Theme, clrSignalRest,             CLR_RED);
THEME_CLR(Theme, clrSeen,                   CLR_SEEN);
THEME_CLR(Theme, clrTrackName,              CLR_TRACK);
THEME_CLR(Theme, clrAlertFg,                CLR_WHITE);
THEME_CLR(Theme, clrAlertBg,                CLR_ALERT);
THEME_CLR(Theme, clrChannelName,            CLR_CHANNEL_NAME);
THEME_CLR(Theme, clrEventTitle,             CLR_EVENT_TITLE);
THEME_CLR(Theme, clrEventTime,              CLR_EVENT_TIME);
THEME_CLR(Theme, clrEventShortText,         CLR_EVENT_SHORTTEXT);
THEME_CLR(Theme, clrEventDescription,       CLR_TEXT);

// Buttons:

THEME_CLR(Theme, clrButtonRedFg,            CLR_BLACK);
THEME_CLR(Theme, clrButtonRedBg,            CLR_RED);
THEME_CLR(Theme, clrButtonGreenFg,          CLR_BLACK);
THEME_CLR(Theme, clrButtonGreenBg,          CLR_GREEN);
THEME_CLR(Theme, clrButtonYellowFg,         CLR_BLACK);
THEME_CLR(Theme, clrButtonYellowBg,         CLR_YELLOW);
THEME_CLR(Theme, clrButtonBlueFg,           CLR_BLACK);
THEME_CLR(Theme, clrButtonBlueBg,           CLR_BLUE);

// Messages:

THEME_CLR(Theme, clrMessageStatusFg,        CLR_BLACK);
THEME_CLR(Theme, clrMessageStatusBg,        CLR_BLUE);
THEME_CLR(Theme, clrMessageInfoFg,          CLR_BLACK);
THEME_CLR(Theme, clrMessageInfoBg,          CLR_GREEN);
THEME_CLR(Theme, clrMessageWarningFg,       CLR_BLACK);
THEME_CLR(Theme, clrMessageWarningBg,       CLR_YELLOW);
THEME_CLR(Theme, clrMessageErrorFg,         CLR_BLACK);
THEME_CLR(Theme, clrMessageErrorBg,         CLR_RED);

// Volume:

THEME_CLR(Theme, clrVolumeFrame,            CLR_MAIN_FRAME);
THEME_CLR(Theme, clrVolumeSymbol,           CLR_BLACK);
THEME_CLR(Theme, clrVolumeBarUpper,         RgbShade(CLR_MAIN_FRAME, -0.2));
THEME_CLR(Theme, clrVolumeBarLower,         CLR_GREEN);

// Channel display:

THEME_CLR(Theme, clrChannelFrameFg,         CLR_BLACK);
THEME_CLR(Theme, clrChannelFrameBg,         CLR_CHANNEL_FRAME);
THEME_CLR(Theme, clrChannelSymbolOn,        CLR_BLACK);
THEME_CLR(Theme, clrChannelSymbolOff,       RgbShade(CLR_CHANNEL_FRAME, -0.2));
THEME_CLR(Theme, clrChannelSymbolRecFg,     CLR_WHITE);
THEME_CLR(Theme, clrChannelSymbolRecBg,     CLR_RED);
THEME_CLR(Theme, clrSwitchTimer,            CLR_GREEN);

// Menu:

THEME_CLR(Theme, clrMenuFrameFg,            CLR_BLACK);
THEME_CLR(Theme, clrMenuFrameBg,            CLR_MAIN_FRAME);
THEME_CLR(Theme, clrMenuTitle,              CLR_MAIN_FRAME);
THEME_CLR(Theme, clrMenuMainBracket,        CLR_MENU_ITEMS);
THEME_CLR(Theme, clrMenuTimerRecording,     CLR_DEVICE);
THEME_CLR(Theme, clrMenuDeviceRecording,    CLR_TIMER);
THEME_CLR(Theme, clrMenuItemCurrentFg,      CLR_MAIN_FRAME);
THEME_CLR(Theme, clrMenuItemCurrentBg,      RgbShade(CLR_MENU_ITEMS, -0.5));
THEME_CLR(Theme, clrMenuItemSelectable,     CLR_MENU_ITEMS);
THEME_CLR(Theme, clrMenuItemNonSelectable,  CLR_TEXT);
THEME_CLR(Theme, clrMenuScrollbarTotal,     RgbShade(CLR_MAIN_FRAME, 0.2));
THEME_CLR(Theme, clrMenuScrollbarShown,     CLR_SEEN);
THEME_CLR(Theme, clrMenuScrollbarArrow,     CLR_BLACK);
THEME_CLR(Theme, clrMenuText,               CLR_TEXT);

// Replay display:

THEME_CLR(Theme, clrReplayFrameFg,          CLR_BLACK);
THEME_CLR(Theme, clrReplayFrameBg,          CLR_REPLAY_FRAME);
THEME_CLR(Theme, clrReplayFrameMg,          CLR_WHITE);
THEME_CLR(Theme, clrReplayPosition,         CLR_SEEN);
THEME_CLR(Theme, clrReplayJumpFg,           CLR_BLACK);
THEME_CLR(Theme, clrReplayJumpBg,           CLR_SEEN);
THEME_CLR(Theme, clrReplayProgressSeen,     CLR_SEEN);
THEME_CLR(Theme, clrReplayProgressRest,     RgbShade(CLR_WHITE, -0.2));
THEME_CLR(Theme, clrReplayProgressSelected, CLR_EXPOSED);
THEME_CLR(Theme, clrReplayProgressMark,     CLR_BLACK);
THEME_CLR(Theme, clrReplayProgressCurrent,  CLR_EXPOSED);

// Track display:

THEME_CLR(Theme, clrTrackFrameFg,           CLR_BLACK);
THEME_CLR(Theme, clrTrackFrameBg,           CLR_TRACK);
THEME_CLR(Theme, clrTrackItemFg,            CLR_BLACK);
THEME_CLR(Theme, clrTrackItemBg,            RgbShade(CLR_TRACK, 0.5));
THEME_CLR(Theme, clrTrackItemCurrentFg,     CLR_BLACK);
THEME_CLR(Theme, clrTrackItemCurrentBg,     CLR_TRACK);

extern bool TwoColors;

enum eMarginUsage {
  muNone     = 0x0000,
  muLeft     = 0x0001,
  muTop      = 0x0002,
  muRight    = 0x0004,
  muBottom   = 0x0008,
  };

cOsd *CreateOsd(int Left, int Top, int x0, int y0, int x1, int y1);
cFont *CreateTinyFont(int LineHeight);
bool DrawDeviceData(cOsd *Osd, const cDevice *Device, int x0, int y0, int x1, int y1, int &xs, const cFont *TinyFont, cString &LastDeviceType, cCamSlot *&LastCamSlot, bool Initial);
void DrawDeviceSignal(cOsd *Osd, const cDevice *Device, int x0, int y0, int x1, int y1, int &LastSignalStrength, int &LastSignalQuality, bool Initial);
void DrawDevicePosition(cOsd *Osd, const cPositioner *Positioner, int x0, int y0, int x1, int y1, int &LastCurrent);
void DrawRectangleOutline(cOsd *Osd, int x1, int y1, int x2, int y2, tColor Color1, tColor Color2, int Usage = 0);
int FreeMB(const char *Base, bool Initial);

class cLCARSNG : public cSkin {
public:
  cLCARSNG(void);
  virtual const char *Description(void);
  virtual cSkinDisplayChannel *DisplayChannel(bool WithInfo);
  virtual cSkinDisplayMenu *DisplayMenu(void);
  virtual cSkinDisplayReplay *DisplayReplay(bool ModeOnly);
  virtual cSkinDisplayVolume *DisplayVolume(void);
  virtual cSkinDisplayTracks *DisplayTracks(const char *Title, int NumTracks, const char * const *Tracks);
  virtual cSkinDisplayMessage *DisplayMessage(void);
  };

#endif //__SKINLCARS_H
