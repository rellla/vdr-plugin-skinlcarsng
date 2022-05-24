/*
 * status.h: Keeping track of several VDR status settings
 */

#ifndef __STATUS_H_
#define __STATUS_H_

#include <vdr/status.h>

class cLCARSNGStatusMonitor : public cStatus
{
private:
   int Volume = 0;
protected:
   virtual void SetVolume(int Volume, bool Absolute);
public:
   cLCARSNGStatusMonitor(void) {};
   ~cLCARSNGStatusMonitor(void) {};
   int GetVolume(void) { return Volume; };
};

extern cLCARSNGStatusMonitor *statusMonitor;

#endif
