#include "z64.h"
#include "Gfx #1.3.h"

extern const int screen_width = 1024, screen_height = 768;

LPDIRECTDRAW7 lpdd = 0;
LPDIRECTDRAWSURFACE7 lpddsprimary;
LPDIRECTDRAWSURFACE7 lpddsback;
DDSURFACEDESC2 ddsd;
HRESULT res;
RECT dst, src;
INT32 pitchindwords;

alignas(UINT32) UINT32 FrameBuffer[PRESCALE_WIDTH * PRESCALE_HEIGHT];
extern int oldlowerfield;

FILE* zeldainfo = 0;
int ProcessDListShown = 0;
extern int SaveLoaded;
extern UINT32 command_counter;

extern GFX_INFO gfx;

int rdp_init();
int rdp_close();
int rdp_update();
void rdp_process_list(void);
extern INLINE void popmessage(const char* err, ...);
extern INLINE void fatalerror(const char* err, ...);


EXPORT void CALL CaptureScreen(char* Directory)
{
}


EXPORT void CALL ChangeWindow(void)
{
}


EXPORT void CALL CloseDLL(void)
{
}


EXPORT void CALL DllAbout(HWND hParent)
{
	popmessage("angrylion's RDP, unpublished beta. MESS source code used.");
}


EXPORT void CALL DllConfig(HWND hParent)
{
	popmessage("Nothing to configure");
}


EXPORT void CALL DllTest(HWND hParent)
{
}

EXPORT void CALL ReadScreen(void** dest, long* width, long* height)
{
	UINT32 w = PRESCALE_WIDTH;
	UINT32 h = src.bottom;
	if (h < 480) // progressive; double the height
	{
		UINT32* s = FrameBuffer + (h - 1) * w;
		UINT32* d = FrameBuffer + ((h * 2) - 1) * w;
		for (UINT32 i = 0; i < h; i++)
		{
			memcpy(d, s, w * sizeof(UINT32));
			memcpy(d - w, s, w * sizeof(UINT32));
			d -= w * 2;
			s -= w;
		}
		h *= 2;
	}
	else // interlaced; deinterlace
	{
		UINT32 buf[640 * (288 + 1)];
		memset(buf, 0, w + sizeof(UINT32));
		UINT32* s = FrameBuffer + oldlowerfield * w;
		UINT32* d = buf + oldlowerfield * w;
		UINT32 halfh = h / 2;
		for (UINT32 i = 0; i < halfh; i++)
		{
			memcpy(d, s, w * sizeof(UINT32));
			d += w;
			s += w * 2;
		}
		s = buf;
		d = FrameBuffer;
		for (UINT32 i = 0; i < halfh; i++)
		{
			memcpy(d, s, w * sizeof(UINT32));
			memcpy(d + w, s, w * sizeof(UINT32));
			d += w * 2;
			s += w;
		}
	}

	*width = w;
	*height = h;
	*dest = malloc(w * h * 3);
	if (!*dest)
	{
		fatalerror("Could not allocate memory for ReadScreen()!");
		return;
	}

	UINT32* fb = FrameBuffer + (h - 1) * w;
	UINT8* ret = (UINT8*)(*dest);
	for (UINT32 i = 0; i < h; i++)
	{
		for (UINT32 j = 0; j < w; j++)
		{
			UINT8* d = &ret[j * 3];
			UINT32 p = fb[j];
			d[0] = p >> 0 & 0xFF;
			d[1] = p >> 8 & 0xFF;
			d[2] = p >> 16 & 0xFF;
		}
		fb -= w;
		ret += w * 3;
	}
}

// for mupen to free what's returned from ReadScreen
EXPORT void CALL DllCrtFree(void* p)
{
	free(p);
}


EXPORT void CALL DrawScreen(void)
{
}


EXPORT void CALL GetDllInfo(PLUGIN_INFO* PluginInfo)
{
	PluginInfo->Version = 0x0103;
	PluginInfo->Type = PLUGIN_TYPE_GFX;
	sprintf(PluginInfo->Name, "My little plugin");
	PluginInfo->NormalMemory = TRUE;
	PluginInfo->MemoryBswaped = TRUE;
}


GFX_INFO gfx;

EXPORT BOOL CALL InitiateGFX(GFX_INFO Gfx_Info)
{
	gfx = Gfx_Info;

	return TRUE;
}


EXPORT void CALL MoveScreen(int xpos, int ypos)
{
	RECT statusrect;
	POINT p;
	p.x = p.y = 0;
	GetClientRect(gfx.hWnd, &dst);
	ClientToScreen(gfx.hWnd, &p);
	OffsetRect(&dst, p.x, p.y);
	GetClientRect(gfx.hStatusBar, &statusrect);
	dst.bottom -= statusrect.bottom;
}


EXPORT void CALL ProcessDList(void)
{
	if (!ProcessDListShown)
	{
		popmessage("ProcessDList");
		ProcessDListShown = 1;
	}
}


EXPORT void CALL ProcessRDPList(void)
{
	rdp_process_list();
	return;
}


EXPORT void CALL RomClosed(void)
{
	rdp_close();
	if (lpddsback)
	{
		IDirectDrawSurface_Release(lpddsback);
		lpddsback = 0;
	}
	if (lpddsprimary)
	{
		IDirectDrawSurface_Release(lpddsprimary);
		lpddsprimary = 0;
	}
	if (lpdd)
	{
		IDirectDraw_Release(lpdd);
		lpdd = 0;
	}

	SaveLoaded = 1;
	command_counter = 0;
}


EXPORT void CALL RomOpen(void)
{
	RECT bigrect, smallrect, statusrect;

	GetWindowRect(gfx.hWnd, &bigrect);
	GetClientRect(gfx.hWnd, &smallrect);
	int rightdiff = screen_width - smallrect.right;
	int bottomdiff = screen_height - smallrect.bottom;
	if (gfx.hStatusBar)
	{
		GetClientRect(gfx.hStatusBar, &statusrect);
		bottomdiff += statusrect.bottom;
	}
	MoveWindow(gfx.hWnd, bigrect.left, bigrect.top, bigrect.right - bigrect.left + rightdiff, bigrect.bottom - bigrect.top + bottomdiff, TRUE);

	DDPIXELFORMAT ftpixel;
	LPDIRECTDRAWCLIPPER lpddcl;

	res = DirectDrawCreateEx(0, (LPVOID*)&lpdd, IID_IDirectDraw7, 0);
	if (res != DD_OK)
		fatalerror("Couldn't create a DirectDraw object");
	res = IDirectDraw_SetCooperativeLevel(lpdd, gfx.hWnd, DDSCL_NORMAL);
	if (res != DD_OK)
		fatalerror("Couldn't set a cooperative level. Error code %x", res);

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	res = IDirectDraw_CreateSurface(lpdd, &ddsd, &lpddsprimary, 0);
	if (res != DD_OK)
		fatalerror("CreateSurface for a primary surface failed. Error code %x", res);

	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = PRESCALE_WIDTH;
	ddsd.dwHeight = PRESCALE_HEIGHT;
	memset(&ftpixel, 0, sizeof(ftpixel));
	ftpixel.dwSize = sizeof(ftpixel);
	ftpixel.dwFlags = DDPF_RGB;
	ftpixel.dwRGBBitCount = 32;
	ftpixel.dwRBitMask = 0xff0000;
	ftpixel.dwGBitMask = 0xff00;
	ftpixel.dwBBitMask = 0xff;
	ddsd.ddpfPixelFormat = ftpixel;
	res = IDirectDraw_CreateSurface(lpdd, &ddsd, &lpddsback, 0);
	if (res == DDERR_INVALIDPIXELFORMAT)
		fatalerror("ARGB8888 is not supported. You can try changing desktop color depth to 32-bit, but most likely that won't help.");
	else if (res != DD_OK)
		fatalerror("CreateSurface for a secondary surface failed. Error code %x", res);

	res = IDirectDrawSurface_GetSurfaceDesc(lpddsback, &ddsd);
	if (res != DD_OK)
		fatalerror("GetSurfaceDesc failed.");
	if ((ddsd.lPitch & 3) || ddsd.lPitch < (PRESCALE_WIDTH << 2))
		fatalerror("Pitch of a secondary surface is either not 32 bit aligned or two small.");
	pitchindwords = ddsd.lPitch >> 2;

	res = IDirectDraw_CreateClipper(lpdd, 0, &lpddcl, 0);
	if (res != DD_OK)
		fatalerror("Couldn't create a clipper.");
	res = IDirectDrawClipper_SetHWnd(lpddcl, 0, gfx.hWnd);
	if (res != DD_OK)
		fatalerror("Couldn't register a windows handle as a clipper.");
	res = IDirectDrawSurface_SetClipper(lpddsprimary, lpddcl);
	if (res != DD_OK)
		fatalerror("Couldn't attach a clipper to a surface.");

	src.top = src.left = 0;
	src.bottom = 0;
	src.right = PRESCALE_WIDTH;

	POINT p;
	p.x = p.y = 0;
	GetClientRect(gfx.hWnd, &dst);
	ClientToScreen(gfx.hWnd, &p);
	OffsetRect(&dst, p.x, p.y);
	GetClientRect(gfx.hStatusBar, &statusrect);
	dst.bottom -= statusrect.bottom;

	DDBLTFX ddbltfx;
	ddbltfx.dwSize = sizeof(DDBLTFX);
	ddbltfx.dwFillColor = 0;
	res = IDirectDrawSurface_Blt(lpddsprimary, &dst, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	src.bottom = PRESCALE_HEIGHT;
	res = IDirectDrawSurface_Blt(lpddsback, &src, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

	rdp_init();
}


EXPORT void CALL ShowCFB(void)
{
	rdp_update();
}



EXPORT void CALL UpdateScreen(void)
{
	rdp_update();
}


EXPORT void CALL ViStatusChanged(void)
{
}


EXPORT void CALL ViWidthChanged(void)
{
}



EXPORT void CALL FBWrite(DWORD, DWORD)
{
}

EXPORT void CALL FBWList(FrameBufferModifyEntry* plist, DWORD size)
{
}


EXPORT void CALL FBRead(DWORD addr)
{
}


EXPORT void CALL FBGetFrameBufferInfo(void* pinfo)
{
}

