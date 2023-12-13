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
