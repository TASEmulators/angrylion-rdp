#include "z64.h"
#include "Gfx #1.3.h"
#include <cstring>

namespace angrylion {

extern int SaveLoaded;
extern UINT32 command_counter;

int rdp_init();
int rdp_close();
int rdp_update();
bool rdp_process_list(void);
extern INLINE void popmessage(const char* err, ...);
extern INLINE void fatalerror(const char* err, ...);

extern UINT32 FrameBuffer[];
extern int visiblelines;
extern int oldlowerfield;

UINT32 FinalizedFrameBuffer[640 * 576];

UINT32 * FinalizeFrame(int deinterlacer)
{
	UINT32 w = 640;
	UINT32 h = visiblelines;
	if (h < 480)
	{
		UINT32* s = FrameBuffer;
		UINT32* d = FinalizedFrameBuffer;
		UINT32 doubleW = w * 2;
		for (UINT32 i = 0; i < h; i++)
		{
			memcpy(d, s, w * sizeof (UINT32));
			memcpy(d + w, s, w * sizeof(UINT32));
			s += w;
			d += doubleW;
		}
	}
	else
	{
		if (deinterlacer) // bob
		{
			UINT32* s = FrameBuffer + oldlowerfield * w;
			UINT32* d = FinalizedFrameBuffer;
			UINT32 doubleW = w * 2;
			UINT32 halfH = h / 2;
			for (UINT32 i = 0; i < halfH; i++)
			{
				memcpy(d, s, w * sizeof (UINT32));
				memcpy(d + w, s, w * sizeof (UINT32));
				s += doubleW;
				d += doubleW;
			}
		}
		else // weave
		{
			memcpy(FinalizedFrameBuffer, FrameBuffer, w * h * sizeof (UINT32));
		}
	}
	return FinalizedFrameBuffer;
}

bool ProcessRDPList(void)
{
	return rdp_process_list();
}

void RomClosed(void)
{
	rdp_close();

	SaveLoaded = 1;
	command_counter = 0;
}

void RomOpen(void)
{
	rdp_init();
}

void UpdateScreen(void)
{
	rdp_update();
}

}
