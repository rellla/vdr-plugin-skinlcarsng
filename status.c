/*
 * status.c: Keeping track of several VDR status settings
 */

#include "status.h"

cLCARSNGStatusMonitor *statusMonitor;

void cLCARSNGStatusMonitor::SetVolume(int volume, bool absolute)
{  // The volume has been set to the given value, either
   // absolutely or relative to the current volume.

   Volume = absolute ? volume : Volume + volume;
}
