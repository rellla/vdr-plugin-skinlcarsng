#include "config.h"
#include "lcarsng.h"
#include "displaymessage.h"

// --- cLCARSNGDisplayMessage ----------------------------------------------

cLCARSNGDisplayMessage::cLCARSNGDisplayMessage(void)
{
  int lineHeight = cFont::GetFont(fontOsd)->Height();

  // message and volume box
  int xv00 = 0;
  int xv01 = cOsd::OsdWidth();
  int yv00 = cOsd::OsdHeight() - lineHeight;
  int yv01 = yv00 + lineHeight;

  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop(), xv00, yv00, xv01, yv01);
  messageBox = new cLCARSNGMessageBox(osd, cRect(xv00, yv00, xv01 - xv00, yv01 - yv00));
}

cLCARSNGDisplayMessage::~cLCARSNGDisplayMessage()
{
  delete messageBox;
  delete osd;
  cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
}

void cLCARSNGDisplayMessage::SetMessage(eMessageType Type, const char *Text)
{
  if (messageBox)
     messageBox->SetMessage(Type, Text);
}

void cLCARSNGDisplayMessage::Flush(void)
{
  osd->Flush();
}
