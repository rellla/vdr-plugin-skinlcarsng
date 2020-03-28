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
#include "config.h"
#include "displaychannel.h"
#include "displaymenu.h"
#include "displaymessage.h"
#include "displayreplay.h"
#include "displaytracks.h"
#include "displayvolume.h"

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

void DrawRectangleOutline(cOsd *Osd, int x1, int y1, int x2, int y2, tColor Color1, tColor Color2, int usage)
{
   Osd->DrawRectangle(x1, y1, x2, y2, Color1);
   Osd->DrawRectangle(x1 + ((usage & muLeft) ? Config.Margin : 0), y1 + ((usage & muTop) ? Config.Margin : 0), x2 - ((usage & muRight) ? Config.Margin : 0), y2 - ((usage & muBottom) ? Config.Margin : 0), Color2);
}

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
//     dsyslog ("%s %s %d %s\n", __FILE__, __func__,  __LINE__, (const char *)tmpbase);
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
