#include <emulibc.h>

#include "z64.h"
#include "angrylion.h"
#include <cstring>

namespace angrylion {

void rdp_init();
int rdp_update();
int rdp_update_fast();
void rdp_process_list(void);

extern UINT32 FrameBuffer[];
extern int visiblelines;
extern int oldlowerfield;

UINT32* OutFrameBuffer;
UINT32 OutHeight;

static bool skipped_draw;

void FinalizeFrame(int deinterlacer)
{
	if (skipped_draw || !OutFrameBuffer) return;

	UINT32 w = 640;
	UINT32 h = visiblelines;
	if (h < 480)
	{
		UINT32 * src = FrameBuffer;
		UINT32 * dst = OutFrameBuffer;
		UINT32 doubleW = w * 2;
		for (UINT32 i = 0; i < h; i++)
		{
			memcpy(dst, src, w * sizeof(UINT32));
			memcpy(dst + w, src, w * sizeof(UINT32));
			src += w;
			dst += doubleW;
		}

		OutHeight = h * 2;
	}
	else
	{
		if (deinterlacer) // bob
		{
			UINT32 * src = FrameBuffer + oldlowerfield * w;
			UINT32 * dst = OutFrameBuffer;
			UINT32 doubleW = w * 2;
			UINT32 halfH = h / 2;
			for (UINT32 i = 0; i < halfH; i++)
			{
				memcpy(dst, src, w * sizeof(UINT32));
				memcpy(dst + w, src, w * sizeof(UINT32));
				src += doubleW;
				dst += doubleW;
			}
		}
		else // weave
		{
			memcpy(OutFrameBuffer, FrameBuffer, w * h * sizeof(UINT32));
		}

		OutHeight = h;
	}
}

void ProcessRDPList(void)
{
	rdp_process_list();
}

void Init(void)
{
	rdp_init();
}

void UpdateScreen(bool fast)
{
	if (fast)
	{
		if (!OutFrameBuffer)
		{
			skipped_draw = 1;
			return;
		}

		skipped_draw = rdp_update_fast();
	}
	else
		skipped_draw = rdp_update();
}

}
