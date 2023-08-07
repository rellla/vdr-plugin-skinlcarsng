#include "config.h"
#include "lcarsng.h"
#include "displaymessage.h"

// --- cLCARSNGDisplayMessage ----------------------------------------------

cLCARSNGDisplayMessage::cLCARSNGDisplayMessage(void)
{
  int lineHeight = cFont::GetFont(fontOsd)->Height();

  osd = CreateOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - lineHeight, 0, 0, cOsd::OsdWidth() - 1, lineHeight - 1);
  messageBox = new cLCARSNGMessageBox(osd, cRect(0, 0, cOsd::OsdWidth(), lineHeight));
}

cLCARSNGDisplayMessage::~cLCARSNGDisplayMessage()
{
  delete messageBox;
  delete osd;
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
