#ifndef _ANGRYLION_H_INCLUDED__
#define _ANGRYLION_H_INCLUDED__

#include <stdint.h>

namespace angrylion {

/******************************************************************
  Function: FinalizeFrame
  Purpose:  This function is called to finalize the frame buffer,
            applying doubling or deinterlacing.
  input:    type of deinterlacer (0 = weave, 1 = bob)
  output:   none
*******************************************************************/ 
void FinalizeFrame(int deinterlacer);

/******************************************************************
  Function: ProcessRDPList
  Purpose:  This function is called when there is an LLE RDP list
            to be processed.
  input:    none
  output:   none
*******************************************************************/ 
void ProcessRDPList(void);

/******************************************************************
  Function: Init
  Purpose:  This function is called when the emulator is initializing.
  input:    none
  output:   none
*******************************************************************/ 
void Init(void);

/******************************************************************
  Function: UpdateScreen
  Purpose:  This function is called in response to a vsync of the
            screen were the VI bit in MI_INTR_REG has already been
			set
  input:    do fast VI (no VI filters) or not
  output:   none
*******************************************************************/ 
void UpdateScreen(bool fast);

}

#endif //_ANGRYLION_H_INCLUDED__
