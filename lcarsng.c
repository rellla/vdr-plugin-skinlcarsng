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
#include "displaychannel.h"
#include "displaymenu.h"
#include "displayreplay.h"
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

cTheme Theme;

bool TwoColors = false;

// --- Helper functions ------------------------------------------------------

cOsd *CreateOsd(int Left, int Top, int x0, int y0, int x1, int y1)
{
  cOsd *Osd = cOsdProvider::NewOsd(Left, Top);
  int Bpp[] = { 32, 8, 4, 2, 1 };
  tArea Area = { x0, y0, x1, y1, 0 };
  for (unsigned int i = 0; i < sizeof(Bpp) / sizeof(int); i++) {
      Area.bpp = Bpp[i];
      if (Osd->CanHandleAreas(&Area, 1) == oeOk) {
         Osd->SetAreas(&Area, 1);
         Osd->SetAntiAliasGranularity(20, 16);
         TwoColors = Area.bpp == 1;
         break;
         }
      }
  return Osd;
}

cFont *CreateTinyFont(int LineHeight)
{
  // Creates a font that is not higher than half of LineHeight.
  LineHeight /= 1.85;
  int Height = LineHeight;
  for (;;) {
      cFont *TinyFont = cFont::CreateFont(Setup.FontOsd, Height);
      if (Height < 2 || TinyFont->Height() <= LineHeight)
         return TinyFont;
      delete TinyFont;
      Height -= 1;
      }
}

bool DrawDeviceData(cOsd *Osd, const cDevice *Device, int x0, int y0, int x1, int y1, int &xs, const cFont *TinyFont, cString &LastDeviceType, cCamSlot *&LastCamSlot, bool Initial)
{
  cString DeviceType = Device->DeviceType();
  cCamSlot *CamSlot = Device->CamSlot();
  if (Initial || strcmp(DeviceType, LastDeviceType) || CamSlot != LastCamSlot) {
     const cFont *font = cFont::GetFont(fontOsd);
     tColor ColorFg = Theme.Color(clrDeviceFg);
     tColor ColorBg = Theme.Color(clrDeviceBg);
     Osd->DrawRectangle(x0, y0, x1 - 1, y1 - 1, ColorBg);
     int x = x0;
     // Device number:
     cString Nr = itoa(Device->DeviceNumber() + 1);
     int w = max(font->Width(Nr), y1 - y0);
     Osd->DrawText(x, y0, Nr, ColorFg, ColorBg, font, w, y1 - y0, taCenter);
     x += w;
     // Device type:
     Osd->DrawText(x, y0, DeviceType, ColorFg, ColorBg, TinyFont);
     xs = max(xs, x + TinyFont->Width(DeviceType));
     LastDeviceType = DeviceType;
     // CAM:
     if (CamSlot) {
#if APIVERSNUM > 20302
        cString s = cString::sprintf("CAM %d", CamSlot->MasterSlotNumber());
#else
        cString s = cString::sprintf("CAM %d", CamSlot->SlotNumber());
#endif
        Osd->DrawText(x, y1 - TinyFont->Height(), s, ColorFg, ColorBg, TinyFont);
        xs = max(xs, x + TinyFont->Width(s));
        }
     LastCamSlot = CamSlot;
     return true;
     }
  return false;
}

void DrawDeviceSignal(cOsd *Osd, const cDevice *Device, int x0, int y0, int x1, int y1, int &LastSignalStrength, int &LastSignalQuality, bool Initial)
{
  int SignalStrength = Device->SignalStrength();
  int SignalQuality = Device->SignalQuality();
  int d = max((y1 - y0) / 10, 1);
  int x00 = x0 + d;
  int x01 = x1 - d;
  int h = (y1 - y0 - 3 * d) / 2;
  int w = x01 - x00;
  int y00 = y0 + d;
  int y01 = y00 + h;
  int y03 = y1 - d;
  int y02 = y03 - h;
  tColor ColorSignalValue, ColorSignalRest;
  if (TwoColors) {
     ColorSignalValue = Theme.Color(clrBackground);
     ColorSignalRest = Theme.Color(clrMenuFrameBg);
     }
  else {
     ColorSignalValue = Theme.Color(clrSignalValue);
     ColorSignalRest = Theme.Color(clrSignalRest);
     }
  if (SignalStrength >= 0 && (Initial || SignalStrength != LastSignalStrength)) {
     int s = SignalStrength * w / 100;
     Osd->DrawRectangle(x00, y00, x00 + s - 1, y01 - 1, ColorSignalValue);
     Osd->DrawRectangle(x00 + s, y00, x01 - 1, y01 - 1, ColorSignalRest);
     LastSignalStrength = SignalStrength;
     }
  if (SignalQuality >= 0 && (Initial || SignalQuality != LastSignalQuality)) {
     int q = SignalQuality * w / 100;
     Osd->DrawRectangle(x00, y02, x00 + q - 1, y03 - 1, ColorSignalValue);
     Osd->DrawRectangle(x00 + q, y02, x01 - 1, y03 - 1, ColorSignalRest);
     LastSignalQuality = SignalQuality;
     }
}

#if APIVERSNUM > 20101
void DrawDevicePosition(cOsd *Osd, const cPositioner *Positioner, int x0, int y0, int x1, int y1, int &LastCurrent)
{
  int HorizonLeft = Positioner->HorizonLongitude(cPositioner::pdLeft);
  int HorizonRight = Positioner->HorizonLongitude(cPositioner::pdRight);
  int HardLimitLeft = cPositioner::NormalizeAngle(HorizonLeft - Positioner->HardLimitLongitude(cPositioner::pdLeft));
  int HardLimitRight = cPositioner::NormalizeAngle(Positioner->HardLimitLongitude(cPositioner::pdRight) - HorizonRight);
  int HorizonDelta = cPositioner::NormalizeAngle(HorizonLeft - HorizonRight);
  int Current = cPositioner::NormalizeAngle(HorizonLeft - Positioner->CurrentLongitude());
  int Target = cPositioner::NormalizeAngle(HorizonLeft - Positioner->TargetLongitude());
  int d = (y1 - y0) / 2;
  int w = x1 - x0 - 2 * d;
  int l = max(x0 + d, x0 + d + w * HardLimitLeft / HorizonDelta);
  int r = min(x1 - d, x1 - d - w * HardLimitRight / HorizonDelta) - 1;
  int c = constrain(x0 + d + w * Current / HorizonDelta, l, r);
  int t = constrain(x0 + d + w * Target / HorizonDelta, l, r);
  if (c == LastCurrent)
     return;
  if (c > t)
     swap(c, t);
  tColor ColorRange, ColorMove;
  if (TwoColors) {
     ColorRange = Theme.Color(clrChannelFrameBg);
     ColorMove = Theme.Color(clrBackground);
     }
  else {
     ColorRange = Theme.Color(clrChannelFrameBg);
     ColorMove = Theme.Color(clrDeviceBg);
     }
  Osd->DrawRectangle(x0, y0, x1 - 1, y1 - 1, Theme.Color(clrBackground));
  Osd->DrawEllipse(l - d, y0, l, y1 - 1, ColorRange, 7);
  Osd->DrawRectangle(l, y0, r, y1 - 1, ColorRange);
  Osd->DrawEllipse(r, y0, r + d, y1 - 1, ColorRange, 5);
  Osd->DrawEllipse(c - d, y0, c, y1 - 1, ColorMove, 7);
  Osd->DrawRectangle(c, y0, t, y1 - 1, ColorMove);
  Osd->DrawEllipse(t, y0, t + d, y1 - 1, ColorMove, 5);
  LastCurrent = c;
}
#endif

static time_t lastDiskSpaceCheck = 0;
static int lastFreeMB = -1;

int FreeMB(const char *Base, bool Initial)
{
  bool Directory = false;
  char *currentBase = NULL;
  if (Base) {
     size_t Length = strlen(Base);
     const char *p = strchr(Base, ' ');
     int l = p - Base;
     if (l < 0)
        return 0;
     currentBase = MALLOC(char, Length - l);
     strncpy(currentBase, &Base[l + 1], Length - l -1);
     currentBase[Length - l -1] = '\0';
     Directory = (strcmp(currentBase, cString::sprintf("%s", trVDR("Recordings"))) && strcmp(currentBase, cString::sprintf("%s", trVDR("Deleted Recordings")))) ? true : false;
//     free(p);
     }
  if (Initial || lastFreeMB <= 0 || (time(NULL) - lastDiskSpaceCheck) > DISKSPACECHEK) {
     dev_t fsid = 0;
     int freediskspace = 0;
     std::string path = cVideoDirectory::Name();
     path += "/";
     char *tmpbase = Directory ? ExchangeChars(strdup(currentBase), true) : NULL;
     dsyslog ("%s %s %d %s\n", __FILE__, __func__,  __LINE__, (const char *)tmpbase);
     if (tmpbase)
        path += tmpbase;
     struct stat statdir;
     if (!stat(path.c_str(), &statdir)) {
        if (statdir.st_dev != fsid) {
           fsid = statdir.st_dev;
           struct statvfs fsstat;
           if (!statvfs(path.c_str(), &fsstat)) {
              freediskspace = int((double)fsstat.f_bavail / (double)(1024.0 * 1024.0 / fsstat.f_bsize));
              LOCK_DELETEDRECORDINGS_READ;
              for (const cRecording *rec = DeletedRecordings->First(); rec; rec = DeletedRecordings->Next(rec)) {
                 if (!stat(rec->FileName(), &statdir)) {
                    if (statdir.st_dev == fsid) {
                       int ds = DirSizeMB(rec->FileName());
                       if (ds > 0)
                          freediskspace += ds;
                       else
                          esyslog("DirSizeMB(%s) failed!", rec->FileName());
                       }
                    }
                 }
              }
           else {
              dsyslog("Error while getting filesystem size - statvfs (%s): %s", path.c_str(), strerror(errno));
              freediskspace = 0;
              }
           }
        else {
           freediskspace = lastFreeMB;
           }
        }
     else {
        dsyslog("Error while getting filesystem size - stat (%s): %s", path.c_str(), strerror(errno));
        freediskspace = 0;
        }
     free(tmpbase);
     lastFreeMB = freediskspace;
     lastDiskSpaceCheck = time(NULL);
     }
  free(currentBase);
  return lastFreeMB;
}

// --- cLCARSNGDisplayVolume -----------------------------------------------

class cLCARSNGDisplayVolume : public cSkinDisplayVolume {
private:
  cOsd *osd;
  int x0, x1, x2, x3, x4, x5, x6, x7;
  int y0, y1;
  tColor frameColor;
  int mute;
public:
  cLCARSNGDisplayVolume(void);
  virtual ~cLCARSNGDisplayVolume();
  virtual void SetVolume(int Current, int Total, bool Mute);
  virtual void Flush(void);
  };

cLCARSNGDisplayVolume::cLCARSNGDisplayVolume(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  int lineHeight = font->Height();
  frameColor = Theme.Color(clrVolumeFrame);
  mute = -1;
  x0 = 0;
  x1 = lineHeight / 2;
  x2 = lineHeight;
  x3 = x2 + Gap;
  x7 = cOsd::OsdWidth();
  x6 = x7 - lineHeight / 2;
  x5 = x6 - lineHeight / 2;
  x4 = x5 - Gap;
  y0 = 0;
  y1 = lineHeight;
  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - y1, x0, y0, x7 - 1, y1 - 1);
  osd->DrawRectangle(x0, y0, x7 - 1, y1 - 1, Theme.Color(clrBackground));
  osd->DrawRectangle(x0, y0, x1 - 1, y1 - 1, clrTransparent);
  osd->DrawEllipse  (x0, y0, x1 - 1, y1 - 1, frameColor, 7);
  osd->DrawRectangle(x1, y0, x2 - 1, y1 - 1, frameColor);
  osd->DrawRectangle(x3, y0, x4 - 1, y1 - 1, frameColor);
  osd->DrawRectangle(x5, y0, x6 - 1, y1 - 1, frameColor);
  osd->DrawRectangle(x6, y0, x7 - 1, y1 - 1, clrTransparent);
  osd->DrawEllipse  (x6, y0, x7 - 1, y1 - 1, frameColor, 5);
}

cLCARSNGDisplayVolume::~cLCARSNGDisplayVolume()
{
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayVolume::SetVolume(int Current, int Total, bool Mute)
{
  int xl = x3 + TextSpacing;
  int xr = x4 - TextSpacing;
  int yt = y0 + TextFrame;
  int yb = y1 - TextFrame;
  if (mute != Mute) {
     osd->DrawRectangle(x3, y0, x4 - 1, y1 - 1, frameColor);
     mute = Mute;
     }
  cBitmap bm(Mute ? mute_xpm : volume_xpm);
  osd->DrawBitmap(xl, y0 + (y1 - y0 - bm.Height()) / 2, bm, Theme.Color(clrVolumeSymbol), frameColor);
  if (!Mute) {
     xl += bm.Width() + TextSpacing;
     int w = (y1 - y0) / 3;
     int d = TextFrame;
     int n = (xr - xl + d) / (w + d);
     int x = xr - n * (w + d);
     tColor Color = Theme.Color(clrVolumeBarLower);
     for (int i = 0; i < n; i++) {
         if (Total * i >= Current * n)
            Color = Theme.Color(clrVolumeBarUpper);
         osd->DrawRectangle(x, yt, x + w - 1, yb - 1, Color);
         x += w + d;
         }
     }
}

void cLCARSNGDisplayVolume::Flush(void)
{
  osd->Flush();
}

// --- cLCARSNGDisplayTracks -----------------------------------------------

class cLCARSNGDisplayTracks : public cSkinDisplayTracks {
private:
  cOsd *osd;
  int xt00, xt01, xt02, xt03, xt04, xt05, xt06, xt07, xt08, xt09, xt10, xt11, xt12;
  int yt00, yt01, yt02, yt03, yt04, yt05, yt06, yt07;
  int lineHeight;
  tColor frameColor;
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

cBitmap cLCARSNGDisplayTracks::bmAudioLeft(audioleft_xpm);
cBitmap cLCARSNGDisplayTracks::bmAudioRight(audioright_xpm);
cBitmap cLCARSNGDisplayTracks::bmAudioStereo(audiostereo_xpm);

cLCARSNGDisplayTracks::cLCARSNGDisplayTracks(const char *Title, int NumTracks, const char * const *Tracks)
{
  const cFont *font = cFont::GetFont(fontOsd);
  lineHeight = font->Height();
  frameColor = Theme.Color(clrTrackFrameBg);
  currentIndex = -1;
  xt00 = 0;
  xt01 = xt00 + lineHeight / 2;
  xt02 = xt01 + Gap;
  xt03 = xt00 + 2 * lineHeight;
  int ItemsWidth = font->Width(Title) + xt03 - xt02;
  for (int i = 0; i < NumTracks; i++)
      ItemsWidth = max(ItemsWidth, font->Width(Tracks[i]) + 2 * TextFrame);
  xt04 = xt02 + ItemsWidth;
  xt05 = xt04 + Gap;
  xt06 = xt04 + lineHeight;
  xt07 = xt05 + lineHeight;
  xt08 = xt07 + lineHeight;
  xt09 = xt08 + Gap;
  xt10 = xt09 + lineHeight / 2;
  xt11 = xt10 + Gap;
  xt12 = xt11 + lineHeight;
  yt00 = 0;
  yt01 = yt00 + lineHeight;
  yt02 = yt01 + lineHeight;
  yt03 = yt02 + Gap;
  yt04 = yt03 + NumTracks * lineHeight + (NumTracks - 1) * Gap;
  yt05 = yt04 + Gap;
  yt06 = yt05 + lineHeight;
  yt07 = yt06 + lineHeight;
  while (yt07 > cOsd::OsdHeight()) {
        yt04 -= lineHeight + Gap;
        yt05 = yt04 + Gap;
        yt06 = yt05 + lineHeight;
        yt07 = yt06 + lineHeight;
        }
  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - yt07, xt00, yt00, xt12 - 1, yt07 - 1);
  // The upper elbow:
  osd->DrawRectangle(xt00, yt00, xt12 - 1, yt07 - 1, Theme.Color(clrBackground));
  osd->DrawRectangle(xt00, yt00, xt03 - 1, yt02 - 1, clrTransparent);
  osd->DrawEllipse  (xt00, yt00, xt03 - 1, yt02 - 1, frameColor, 2);
  osd->DrawRectangle(xt03, yt00, xt04 - 1, yt02 - 1, frameColor);
  osd->DrawRectangle(xt04, yt00, xt08 - 1, yt01 - 1, frameColor);
  osd->DrawEllipse  (xt04, yt01, xt06 - 1, yt02 - 1, frameColor, -2);
  osd->DrawRectangle(xt09, yt00, xt10 - 1, yt01 - 1, frameColor);
  osd->DrawRectangle(xt11, yt00, xt11 + lineHeight / 2 - 1, yt01 - 1, frameColor);
  osd->DrawRectangle(xt11 + lineHeight / 2, yt00, xt12 - 1, yt00 + lineHeight / 2 - 1, clrTransparent);
  osd->DrawEllipse  (xt11 + lineHeight / 2, yt00, xt12 - 1, yt01 - 1, frameColor, 5);
  osd->DrawText(xt03, yt00, Title, Theme.Color(clrTrackFrameFg), frameColor, font, xt04 - xt03, 0, taTop | taRight);
  // The items:
  for (int i = 0; i < NumTracks; i++)
      SetItem(Tracks[i], i, false);
  // The lower elbow:
  osd->DrawRectangle(xt00, yt05, xt03 - 1, yt07 - 1, clrTransparent);
  osd->DrawEllipse  (xt00, yt05, xt03 - 1, yt07 - 1, frameColor, 3);
  osd->DrawRectangle(xt03, yt05, xt04 - 1, yt07 - 1, frameColor);
  osd->DrawRectangle(xt04, yt06, xt08 - 1, yt07 - 1, frameColor);
  osd->DrawEllipse  (xt04, yt05, xt06 - 1, yt06 - 1, frameColor, -3);
  osd->DrawRectangle(xt09, yt06, xt10 - 1, yt07 - 1, frameColor);
  osd->DrawRectangle(xt11, yt06, xt11 + lineHeight / 2 - 1, yt07 - 1, frameColor);
  osd->DrawRectangle(xt11 + lineHeight / 2, yt06 + lineHeight / 2, xt12 - 1, yt07 - 1, clrTransparent);
  osd->DrawEllipse  (xt11 + lineHeight / 2, yt06, xt12 - 1, yt07 - 1, frameColor, 5);
}

cLCARSNGDisplayTracks::~cLCARSNGDisplayTracks()
{
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayTracks::SetItem(const char *Text, int Index, bool Current)
{
  int y0 = yt03 + Index * (lineHeight + Gap);
  int y1 = y0 + lineHeight;
  if (y1 > yt04)
     return;
  tColor ColorFg, ColorBg;
  if (Current) {
     ColorFg = Theme.Color(clrTrackItemCurrentFg);
     ColorBg = Theme.Color(clrTrackItemCurrentBg);
     osd->DrawRectangle(xt00, y0, xt01 - 1, y1 - 1, frameColor);
     osd->DrawRectangle(xt02, y0, xt04 - 1, y1 - 1, ColorBg);
     osd->DrawRectangle(xt05, y0, xt05 + lineHeight / 2 - 1, y1 - 1, ColorBg);
     osd->DrawEllipse  (xt05 + lineHeight / 2, y0, xt07 - 1, y1 - 1, ColorBg, 5);
     currentIndex = Index;
     }
  else {
     ColorFg = Theme.Color(clrTrackItemFg);
     ColorBg = Theme.Color(clrTrackItemBg);
     osd->DrawRectangle(xt00, y0, xt01 - 1, y1 - 1, frameColor);
     osd->DrawRectangle(xt02, y0, xt04 - 1, y1 - 1, ColorBg);
     if (currentIndex == Index)
        osd->DrawRectangle(xt05, y0, xt07 - 1, y1 - 1, Theme.Color(clrBackground));
     }
  const cFont *font = cFont::GetFont(fontOsd);
  osd->DrawText(xt02, y0, Text, ColorFg, ColorBg, font, xt04 - xt02, y1 - y0, taTop | taLeft | taBorder);
}

void cLCARSNGDisplayTracks::SetTrack(int Index, const char * const *Tracks)
{
  if (currentIndex >= 0)
     SetItem(Tracks[currentIndex], currentIndex, false);
  SetItem(Tracks[Index], Index, true);
}

void cLCARSNGDisplayTracks::SetAudioChannel(int AudioChannel)
{
  cBitmap *bm = NULL;
  switch (AudioChannel) {
    case 0: bm = &bmAudioStereo; break;
    case 1: bm = &bmAudioLeft;   break;
    case 2: bm = &bmAudioRight;  break;
    default: ;
    }
  if (bm)
     osd->DrawBitmap(xt04 - bm->Width(), (yt06 + yt07 - bm->Height()) / 2, *bm, Theme.Color(clrTrackFrameFg), frameColor);
  else
     osd->DrawRectangle(xt03, yt06, xt04 - 1, yt07 - 1, frameColor);
}

void cLCARSNGDisplayTracks::Flush(void)
{
  osd->Flush();
}

// --- cLCARSNGDisplayMessage ----------------------------------------------

class cLCARSNGDisplayMessage : public cSkinDisplayMessage {
private:
  cOsd *osd;
  int x0, x1, x2, x3, x4, x5, x6, x7;
  int y0, y1;
public:
  cLCARSNGDisplayMessage(void);
  virtual ~cLCARSNGDisplayMessage();
  virtual void SetMessage(eMessageType Type, const char *Text);
  virtual void Flush(void);
  };

cLCARSNGDisplayMessage::cLCARSNGDisplayMessage(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  int lineHeight = font->Height();
  x0 = 0;
  x1 = lineHeight / 2;
  x2 = lineHeight;
  x3 = x2 + Gap;
  x7 = cOsd::OsdWidth();
  x6 = x7 - lineHeight / 2;
  x5 = x6 - lineHeight / 2;
  x4 = x5 - Gap;
  y0 = 0;
  y1 = lineHeight;
  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - y1, x0, y0, x7 - 1, y1 - 1);
}

cLCARSNGDisplayMessage::~cLCARSNGDisplayMessage()
{
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayMessage::SetMessage(eMessageType Type, const char *Text)
{
  tColor ColorFg = Theme.Color(clrMessageStatusFg + 2 * Type);
  tColor ColorBg = Theme.Color(clrMessageStatusBg + 2 * Type);
  osd->DrawRectangle(x0, y0, x7 - 1, y1 - 1, Theme.Color(clrBackground));
  osd->DrawRectangle(x0, y0, x1 - 1, y1 - 1, clrTransparent);
  osd->DrawEllipse  (x0, y0, x1 - 1, y1 - 1, ColorBg, 7);
  osd->DrawRectangle(x1, y0, x2 - 1, y1 - 1, ColorBg);
  osd->DrawText(x3, y0, Text, ColorFg, ColorBg, cFont::GetFont(fontSml), x4 - x3, y1 - y0, taCenter);
  osd->DrawRectangle(x5, y0, x6 - 1, y1 - 1, ColorBg);
  osd->DrawRectangle(x6, y0, x7 - 1, y1 - 1, clrTransparent);
  osd->DrawEllipse  (x6, y0, x7 - 1, y1 - 1, ColorBg, 5);
}

void cLCARSNGDisplayMessage::Flush(void)
{
  osd->Flush();
}

// --- cLCARSNG ------------------------------------------------------------

cLCARSNG::cLCARSNG(void)
:cSkin("lcarsng", &::Theme)
{
}

const char *cLCARSNG::Description(void)
{
  return "LCARSNG";
}

cSkinDisplayChannel *cLCARSNG::DisplayChannel(bool WithInfo)
{
  return new cLCARSNGDisplayChannel(WithInfo);
}

cSkinDisplayMenu *cLCARSNG::DisplayMenu(void)
{
  return new cLCARSNGDisplayMenu;
}

cSkinDisplayReplay *cLCARSNG::DisplayReplay(bool ModeOnly)
{
  return new cLCARSNGDisplayReplay(ModeOnly);
}

cSkinDisplayVolume *cLCARSNG::DisplayVolume(void)
{
  return new cLCARSNGDisplayVolume;
}

cSkinDisplayTracks *cLCARSNG::DisplayTracks(const char *Title, int NumTracks, const char * const *Tracks)
{
  return new cLCARSNGDisplayTracks(Title, NumTracks, Tracks);
}

cSkinDisplayMessage *cLCARSNG::DisplayMessage(void)
{
  return new cLCARSNGDisplayMessage;
}
