#include "lcarsng.h"
#include "config.h"
#include "volumebox.h"
#include "displayvolume.h"

// --- cLCARSNGDisplayVolume -----------------------------------------------

cLCARSNGDisplayVolume::cLCARSNGDisplayVolume(void)
{
  const cFont *font = cFont::GetFont(fontOsd);
  int lineHeight = font->Height();

  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - lineHeight, 0, 0, cOsd::OsdWidth() - 1, lineHeight - 1);
  volumeBox = new cLCARSNGVolumeBox(osd, cRect(0, 0, cOsd::OsdWidth(), lineHeight));
}

cLCARSNGDisplayVolume::~cLCARSNGDisplayVolume()
{
  delete volumeBox;
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayVolume::SetVolume(int Current, int Total, bool Mute)
{
  volumeBox->SetVolume(Current, Total, Mute);
}

void cLCARSNGDisplayVolume::Flush(void)
{
  osd->Flush();
}
