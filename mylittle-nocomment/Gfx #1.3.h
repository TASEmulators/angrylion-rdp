/**********************************************************************************
Common gfx plugin spec, version #1.3 maintained by zilmar (zilmar@emulation64.com)

All questions or suggestions should go through the mailing list.
http://www.egroups.com/group/Plugin64-Dev
***********************************************************************************

Notes:
------

Setting the approprate bits in the MI_INTR_REG and calling CheckInterrupts which 
are both passed to the DLL in InitiateGFX will generate an Interrupt from with in 
the plugin.

The Setting of the RSP flags and generating an SP interrupt  should not be done in
the plugin

**********************************************************************************/
#ifndef _GFX_H_INCLUDED__
#define _GFX_H_INCLUDED__

#include <stdint.h>

namespace angrylion {

/******************************************************************
  Function: FinalizeFrame
  Purpose:  This function is called to finalize the frame buffer,
            applying doubling or deinterlacing. This function
            will return the buffer to the finalized frame.
  input:    type of deinterlacer (0 = weave, 1 = bob)
  output:   pointer to finalized frame buffer
*******************************************************************/ 
uint32_t * FinalizeFrame(int deinterlacer);

/******************************************************************
  Function: ProcessRDPList
  Purpose:  This function is called when there is a Dlist to be
            processed. (Low level GFX list)
  input:    none
  output:   true to set current and start to end, false does nothing
*******************************************************************/ 
bool ProcessRDPList(void);

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/ 
void RomClosed(void);

/******************************************************************
  Function: RomOpen
  Purpose:  This function is called when a rom is open. (from the 
            emulation thread)
  input:    none
  output:   none
*******************************************************************/ 
void RomOpen(void);

/******************************************************************
  Function: UpdateScreen
  Purpose:  This function is called in response to a vsync of the
            screen were the VI bit in MI_INTR_REG has already been
			set
  input:    none
  output:   none
*******************************************************************/ 
void UpdateScreen(void);

}

#endif //_GFX_H_INCLUDED__
