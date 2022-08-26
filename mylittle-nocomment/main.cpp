#include "z64.h"
#include "m64p_api/m64p_api.h"

extern UINT32 FrameBuffer[];
extern int visiblelines;
extern int oldlowerfield;

UINT32 FinalizedFrameBuffer[PRESCALE_WIDTH * PRESCALE_HEIGHT];

GFX_INFO gfx;

int rdp_init();
int rdp_close();
int rdp_update();
void rdp_process_list(void);
extern INLINE void popmessage(const char* err, ...);
extern INLINE void fatalerror(const char* err, ...);

static BOOL ProcessDListShown = FALSE;

static m64p_handle Config = nullptr;
static BOOL BobDeinterlacer = TRUE;

static ptr_ConfigOpenSection ConfigOpenSectionFunc = nullptr;
static ptr_ConfigSaveSection ConfigSaveSectionFunc = nullptr;
static ptr_ConfigSetDefaultBool ConfigSetDefaultBoolFunc = nullptr;
static ptr_ConfigGetParamBool ConfigGetParamBoolFunc = nullptr;

static m64p_dynlib_handle CoreLibHandle = nullptr;

static BOOL inited = FALSE;

static void (*render_cb)(int) = nullptr;

extern "C"
{

EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle coreLibHandle, void*, void (*)(void*, int, const char*))
{
	if (inited)
	{
		return M64ERR_ALREADY_INIT;
	}

	CoreLibHandle = coreLibHandle;

	ConfigOpenSectionFunc = (ptr_ConfigOpenSection)DLSYM(CoreLibHandle, "ConfigOpenSection");
	ConfigSaveSectionFunc = (ptr_ConfigSaveSection)DLSYM(CoreLibHandle, "ConfigSaveSection");
	ConfigSetDefaultBoolFunc = (ptr_ConfigSetDefaultBool)DLSYM(CoreLibHandle, "ConfigSetDefaultBool");
	ConfigGetParamBoolFunc = (ptr_ConfigGetParamBool)DLSYM(CoreLibHandle, "ConfigGetParamBool");

	ConfigOpenSectionFunc("Video-Angrylion", &Config);
	ConfigSetDefaultBoolFunc(Config, "BobDeinterlacer", TRUE, "Use Bob Deinterlacer if True, Use Weave Deinterlacer if False");
	ConfigSaveSectionFunc("Video-Angrylion");

	inited = TRUE;
	return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
	if (!inited)
	{
		return M64ERR_NOT_INIT;
	}

	inited = false;
	return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type* PluginType, int* PluginVersion, int* APIVersion, const char** PluginNamePtr, int* Capabilities)
{
	#define MAYBE_SET(x, y) do { if (x) *x = y; } while (0)

	MAYBE_SET(PluginType, M64PLUGIN_GFX);
	MAYBE_SET(PluginVersion, 1);
	MAYBE_SET(APIVersion, 0x020100);
	MAYBE_SET(PluginNamePtr, "My little plugin");
	MAYBE_SET(Capabilities, 0);

	#undef MAYBE_SET

	return M64ERR_SUCCESS;
}

EXPORT BOOL CALL InitiateGFX(GFX_INFO Gfx_Info)
{
	gfx = Gfx_Info;
	return TRUE;
}

EXPORT void CALL MoveScreen(int, int)
{
}

EXPORT void CALL ProcessDList(void)
{
	if (!ProcessDListShown)
	{
		popmessage("ProcessDList");
		ProcessDListShown = TRUE;
	}
}

EXPORT void CALL ProcessRDPList(void)
{
	rdp_process_list();
}

EXPORT BOOL CALL RomOpen(void)
{
	BobDeinterlacer = ConfigGetParamBoolFunc(Config, "BobDeinterlacer");
	rdp_init();
	return TRUE;
}

EXPORT void CALL RomClosed(void)
{
	rdp_close();
}

EXPORT void CALL ShowCFB(void)
{
}

EXPORT void CALL UpdateScreen(void)
{
	rdp_update();
	if (render_cb)
	{
		render_cb(1);
	}
}

EXPORT void CALL ViStatusChanged(void)
{
}

EXPORT void CALL ViWidthChanged(void)
{
}

EXPORT void CALL ChangeWindow(void)
{
}

EXPORT void CALL ReadScreen2(void* dest, int* width, int* height, int)
{
	if (!dest)
	{
		*width = PRESCALE_WIDTH;
		switch (visiblelines)
		{
			case 240: case 480: *height = 480; break;
			case 288: case 576: *height = 576; break;
			default: *height = 480; break;
		}
		return;
	}

	UINT32 w = PRESCALE_WIDTH;
	UINT32 h = visiblelines;
	if (h < 480) // progressive; double the height
	{
		UINT32* s = FrameBuffer;
		UINT32* d = FinalizedFrameBuffer;
		UINT32 dw = w * 2;
		for (UINT32 i = 0; i < h; i++)
		{
			memcpy(d, s, w * sizeof(UINT32));
			memcpy(d + w, s, w * sizeof(UINT32));
			s += w;
			d += dw;
		}
		h *= 2;
	}
	else // interlaced; deinterlace
	{
		if (BobDeinterlacer)
		{
			UINT32* s = FrameBuffer + oldlowerfield * w;
			UINT32* d = FinalizedFrameBuffer;
			UINT32 dw = w * 2;
			UINT32 hh = h / 2;
			for (UINT32 i = 0; i < hh; i++)
			{
				memcpy(d, s, w * sizeof(UINT32));
				memcpy(d + w, s, w * sizeof(UINT32));
				s += dw;
				d += dw;
			}
		}
		else // result is already weaved
		{
			UINT32* s = FrameBuffer;
			UINT32* d = FinalizedFrameBuffer;
			for (UINT32 i = 0; i < h; i++)
			{
				memcpy(d, s, w * sizeof(UINT32));
				s += w;
				d += w;
			}
		}
	}

	*width = w;
	*height = h;

	UINT32* s = FinalizedFrameBuffer;
	UINT32* d = ((UINT32*)dest) + w * (h - 1);
	for (UINT32 i = 0; i < h; i++)
	{
		memcpy(d, s, w * sizeof(UINT32));
		s += w;
		d -= w;
	}
}

EXPORT void CALL SetRenderingCallback(void (*callback)(int))
{
	render_cb = callback;
}

EXPORT void CALL ResizeVideoOutput(int, int)
{
}

EXPORT void CALL FBWrite(unsigned int, unsigned int)
{
}

EXPORT void CALL FBRead(unsigned int)
{
}

EXPORT void CALL FBGetFrameBufferInfo(void*)
{
}

}
