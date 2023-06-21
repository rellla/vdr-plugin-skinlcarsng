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

void DrawRectangleOutline(cOsd *Osd, int x1, int y1, int x2, int y2, tColor Color1, tColor Color2, int Usage, int Margin)
{
  /* Color1 = border color
   * Color2 = inner color
   */

  int margin = (Margin > 0) ? Margin : Config.Margin;
  Osd->DrawRectangle(x1, y1, x2, y2, (margin == 0) ? Color2 : Color1);
  if ((margin == 0) || !Usage || (Color1 == Color2))
     return;

  Osd->DrawRectangle(x1 + ((Usage & muLeft)   ? margin : 0),
                     y1 + ((Usage & muTop)    ? margin : 0),
                     x2 - ((Usage & muRight)  ? margin : 0),
                     y2 - ((Usage & muBottom) ? margin : 0), Color2);
}

void DrawRectangleOutline(cPixmap *Pixmap, int x, int y, int w, int h, tColor Color1, tColor Color2, int Usage, int Margin)
{
  /* Color1 = border color
   * Color2 = inner color
   */

  int margin = (Margin > 0) ? Margin : Config.Margin;
  Pixmap->DrawRectangle(cRect(x, y, w, h), (margin == 0) ? Color2 : Color1);
  if ((margin == 0) || !Usage || (Color1 == Color2))
     return;

  Pixmap->DrawRectangle(cRect(x + ((Usage & muLeft) ? margin : 0), y + ((Usage & muTop)    ? margin : 0),
                              w - ((Usage & muLeft) ? margin : 0) -    ((Usage & muRight)  ? margin : 0),
                              h - ((Usage & muTop)  ? margin : 0) -    ((Usage & muBottom) ? margin : 0)),
                              Color2);
}

void DrawEllipseOutline(cOsd *Osd, int x1, int y1, int x2, int y2, tColor Color1, tColor Color2, int Quadrants, int Margin)
{
  /* Quadrants:
   * 0       draws the entire ellipse
   * 1..4    draws only the first, second, third or fourth quadrant, respectively
   * 5..8    draws the right, top, left or bottom half, respectively
   * -1..-4  draws the inverted part of the given quadrant
   *  Color1 = border color
   *  Color2 = inner color
   */

  int margin = (Margin > 0) ? Margin : Config.Margin;
  Osd->DrawEllipse(x1, y1, x2, y2, (margin == 0) ? Color2 : Color1, Quadrants);
  if (margin == 0)
     return;

  switch (Quadrants) {
             case  0: Osd->DrawEllipse(x1 + margin, y1 + margin, x2 - margin, y2 - margin, Color2, 0);                  // full ellipse
                      break;
             case  1: Osd->DrawEllipse(x1, y1 + margin, x2 - margin, y2, Color2, 1);                                    // top right
                      break;
             case  2: Osd->DrawEllipse(x1 + margin, y1 + margin, x2, y2, Color2, 2);                                    // top left
                      break;
             case  3: Osd->DrawEllipse(x1 + margin, y1, x2, y2 - margin, Color2, 3);                                    // bottom left
                      break;
             case  4: Osd->DrawEllipse(x1, y1, x2 - margin, y2 - margin, Color2, 4);                                    // bottom right
                      break;
             case  5: Osd->DrawEllipse(x1, y1 + margin, x2 - margin, y2 - margin, Color2, 5);                           // right
                      break;
             case  6: Osd->DrawEllipse(x1 + margin, y1 + margin, x2 - margin, y2, Color2, 6);                           // top
                      break;
             case  7: Osd->DrawEllipse(x1 + margin, y1 + margin, x2, y2 - margin, Color2, 7);                           // left
                      break;
             case  8: Osd->DrawEllipse(x1 + margin, y1, x2 - margin, y2 - margin, Color2, 8);                           // bottom
                      break;
             case -1: Osd->DrawEllipse(x1, y1 - margin, x2 + margin, y2, Color2, -1);                                   // top right invers
                      break;
             case -2: Osd->DrawEllipse(x1 - margin, y1 - margin, x2, y2, Color2, -2);                                   // top left invers
                      break;
             case -3: Osd->DrawEllipse(x1 - margin, y1, x2, y2 + margin, Color2, -3);                                   // bottom left invers
                      break;
             case -4: Osd->DrawEllipse(x1, y1, x2 + margin, y2 + margin, Color2, -4);                                   // bottom right invers
                      break;
             default: ;
             }
}

void DrawEllipseOutline(cPixmap *Pixmap, int x1, int y1, int w, int h, tColor Color1, tColor Color2, int Quadrants, int Margin)
{
  /* Quadrants:
   * 0       draws the entire ellipse
   * 1..4    draws only the first, second, third or fourth quadrant, respectively
   * 5..8    draws the right, top, left or bottom half, respectively
   * -1..-4  draws the inverted part of the given quadrant
   *  Color1 = border color
   *  Color2 = inner color
   */

  int margin = (Margin > 0) ? Margin : Config.Margin;
  Pixmap->DrawEllipse(cRect(x1, y1, w, h), (margin == 0) ? Color2 : Color1, Quadrants);
  if (margin == 0)
     return;

  switch (Quadrants) {
             case  0: Pixmap->DrawEllipse(cRect(x1 + margin, y1 + margin, w - 2 * margin, h - 2 * margin), Color2, 0);  // full ellipse
                      break;
             case  1: Pixmap->DrawEllipse(cRect(x1, y1 + margin, w - margin, h - margin), Color2, 1);                   // top right
                      break;
             case  2: Pixmap->DrawEllipse(cRect(x1 + margin, y1 + margin, w - margin, h - margin), Color2, 2);          // top left
                      break;
             case  3: Pixmap->DrawEllipse(cRect(x1 + margin, y1, w - margin, h - margin), Color2, 3);                   // bottom left
                      break;
             case  4: Pixmap->DrawEllipse(cRect(x1, y1, w - margin, h - margin), Color2, 4);                            // bottom right
                      break;
             case  5: Pixmap->DrawEllipse(cRect(x1, y1 + margin, w - margin, h - 2 * margin), Color2, 5);               // right
                      break;
             case  6: Pixmap->DrawEllipse(cRect(x1 + margin, y1 + margin, w - 2 * margin, h - margin), Color2, 6);      // top
                      break;
             case  7: Pixmap->DrawEllipse(cRect(x1 + margin, y1 + margin, w - margin, h - 2 * margin), Color2, 7);      // left
                      break;
             case  8: Pixmap->DrawEllipse(cRect(x1 + margin, y1, w - 2 * margin, h - margin), Color2, 8);               // bottom
                      break;
             case -1: Pixmap->DrawEllipse(cRect(x1, y1 - margin, w + margin, h + margin), Color2, -1);                  // top right invers
                      break;
             case -2: Pixmap->DrawEllipse(cRect(x1 - margin, y1 - margin, w + margin, h + margin), Color2, -2);         // top left invers
                      break;
             case -3: Pixmap->DrawEllipse(cRect(x1 - margin, y1, w + margin, h + margin), Color2, -3);                  // bottom left invers
                      break;
             case -4: Pixmap->DrawEllipse(cRect(x1, y1, w + margin, h + margin), Color2, -4);                           // bottom right invers
                      break;
             default: ;
             }
}

static time_t lastDiskSpaceCheck = 0;
static int lastFreeMB = -1;

int FreeMB(const char *Base, bool menurecording)
{
  if (!menurecording)
     return cVideoDiskUsage::FreeMinutes();

  bool Directory = false;
  char *currentBase = Base ? strdup(Base) : NULL;

  if (currentBase) {
     const char *p = strchr(currentBase, ' ');
     if (p) {
        int n = p - currentBase;
        if (n == 3 || n == 6) {
           strshift(currentBase, n + 1);
           }
        }
     Directory = (strcmp(currentBase, cString::sprintf("%s", trVDR("Recordings"))) && strcmp(currentBase, cString::sprintf("%s", trVDR("Deleted Recordings")))) ? true : false;
     }
  if (!Directory)
     return cVideoDiskUsage::FreeMinutes();

  cStateKey recordingsStateKey;
  if (lastFreeMB <= 0 || (time(NULL) - lastDiskSpaceCheck) > DISKSPACECHEK) {
     dev_t fsid = 0;
     int freediskspace = 0;
     std::string path = cVideoDirectory::Name();
     path += "/";
     char *tmpbase = Directory ? ExchangeChars(strdup(currentBase), true) : NULL;
     if (tmpbase)
        path += tmpbase;
     struct stat statdir;
     if (!stat(path.c_str(), &statdir)) {
        if (statdir.st_dev != fsid) {
           fsid = statdir.st_dev;
           struct statvfs fsstat;
           if (!statvfs(path.c_str(), &fsstat)) {
              freediskspace = int((double)fsstat.f_bavail / (double)(1024.0 * 1024.0 / fsstat.f_bsize));
              if (const cRecordings *DeletedRecordings = cRecordings::GetDeletedRecordingsRead(recordingsStateKey)) {
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
                 recordingsStateKey.Remove();
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
  if (lastFreeMB == 0)
     return cVideoDiskUsage::FreeMinutes();
  LOCK_RECORDINGS_READ;
  double MBperMinute = Recordings->MBperMinute();
  return int(double(lastFreeMB) / (MBperMinute > 0 ? MBperMinute : MB_PER_MINUTE));
}

std::string StripXmlTag(std::string &Line, const char *Tag) {
  // set the search strings
  std::stringstream strStart, strStop;
  strStart << "<" << Tag << ">";
  strStop << "</" << Tag << ">";
  // find the strings
  std::string::size_type locStart = Line.find(strStart.str());
  std::string::size_type locStop = Line.find(strStop.str());
  if (locStart == std::string::npos || locStop == std::string::npos)
     return "";
  // extract relevant text
  int pos = locStart + strStart.str().size();
  int len = locStop - pos;
  return len < 0 ? "" : Line.substr(pos, len);
}

cString GetScreenResolutionIcon(void) {
  int screenWidth = 0;
  int screenHeight = 0;
  double aspect = 0;
  cDevice::PrimaryDevice()->GetVideoSize(screenWidth, screenHeight, aspect);
  cString iconName("");
  switch (screenHeight) {
     case 4320: // 7680 x 4320 = 8K UHD
     case 2160: // 3840 x 2160 = 4K UHD
                iconName = "UHD4k";
                break;
     case 1440: // 2560 x 1440 = QHD
     case 1080: // "HD1080i"; // 'i' is default, 'p' can't be detected currently
                iconName = "HD1080";
                break;
     case 720:  // "HD720p"; // 'i' is not defined in standards
                iconName = "HD720p";
                break;
     case 576:  // "SD576i"; // assumed 'i'
                iconName = "SD576i";
                break;
     case 480:  // "SD480i"; // assumed 'i'
                iconName = "SD480i";
                break;
     default:   break;
     }
  return iconName;
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
