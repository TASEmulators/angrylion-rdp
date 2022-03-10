/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Deinterlacer.h"

Deinterlacer::Deinterlacer() : FieldBuffer(NULL), StateValid(false), DeintType(DEINT_WEAVE)
{
 PrevDRect.x = 0;
 PrevDRect.y = 0;

 PrevDRect.w = 0;
 PrevDRect.h = 0;

 for (int i = 0; i < 576 + 1; i++)
 {
     FieldLineWidths[0][i] = (i & 1) ? 0 : 640;
     FieldLineWidths[1][i] = (i & 1) ? 640 : 0;
 }
}

Deinterlacer::~Deinterlacer()
{
 if(FieldBuffer)
 {
  delete FieldBuffer;
  FieldBuffer = NULL;
 }
}

void Deinterlacer::SetType(unsigned dt)
{
 if(DeintType != dt)
 {
  DeintType = dt;

  LWBuffer.resize(0);
  if(FieldBuffer)
  {
   delete FieldBuffer;
   FieldBuffer = NULL;
  }
  StateValid = false;
 }
}

void Deinterlacer::InternalProcess(MDFN_Surface *surface, MDFN_Rect &DisplayRect, int32 *LineWidths, const bool field)
{
 //
 // We need to output with LineWidths as always being valid to handle the case of horizontal resolution change between fields
 // while in interlace mode, so clear the first LineWidths entry if it's == ~0, and
 // [...]
 const bool LineWidths_In_Valid = (LineWidths[0] != ~0);
 const bool WeaveGood = (StateValid && PrevDRect.h == DisplayRect.h && DeintType == DEINT_WEAVE);
 //
 // XReposition stuff is to prevent exceeding the dimensions of the video surface under certain conditions(weave deinterlacer, previous field has higher
 // horizontal resolution than current field, and current field's rectangle has an x offset that's too large when taking into consideration the previous field's
 // width; for simplicity, we don't check widths, but just assume that the previous field's maximum width is >= than the current field's maximum width).
 //
 const int32 XReposition = ((WeaveGood && DisplayRect.x > PrevDRect.x) ? DisplayRect.x : 0);

 //printf("%2d %2d, %d\n", DisplayRect.x, PrevDRect.x, XReposition);

 if(XReposition)
  DisplayRect.x = 0;

 if(surface->h && !LineWidths_In_Valid)
 {
  LineWidths[0] = 0;
 }

 for(int y = 0; y < DisplayRect.h / 2; y++)
 {
  // [...]
  // set all relevant source line widths to the contents of DisplayRect(also simplifies the src_lw and related pointer calculation code
  // farther below.
  if(!LineWidths_In_Valid)
   LineWidths[(y * 2) + field + DisplayRect.y] = DisplayRect.w;

  if(XReposition)
  {
    memmove(surface->pix + ((y * 2) + field + DisplayRect.y) * surface->pitchinpix,
	    surface->pix + ((y * 2) + field + DisplayRect.y) * surface->pitchinpix + XReposition,
	    LineWidths[(y * 2) + field + DisplayRect.y] * sizeof(uint32));
  }

  if(WeaveGood)
  {
   const uint32* src = FieldBuffer->pix + y * FieldBuffer->pitchinpix;
   uint32* dest = surface->pix + ((y * 2) + (field ^ 1) + DisplayRect.y) * surface->pitchinpix + DisplayRect.x;
   int32 *dest_lw = &LineWidths[(y * 2) + (field ^ 1) + DisplayRect.y];

   *dest_lw = LWBuffer[y];

   memcpy(dest, src, LWBuffer[y] * sizeof(uint32));
  }
  else if(DeintType == DEINT_BOB)
  {
   const uint32* src = surface->pix + ((y * 2) + field + DisplayRect.y) * surface->pitchinpix + DisplayRect.x;
   uint32* dest = surface->pix + ((y * 2) + (field ^ 1) + DisplayRect.y) * surface->pitchinpix + DisplayRect.x;
   const int32 *src_lw = &LineWidths[(y * 2) + field + DisplayRect.y];
   int32 *dest_lw = &LineWidths[(y * 2) + (field ^ 1) + DisplayRect.y];

   *dest_lw = *src_lw;

   memcpy(dest, src, *src_lw * sizeof(uint32));
  }
  else
  {
   const int32 *src_lw = &LineWidths[(y * 2) + field + DisplayRect.y];
   const uint32* src = surface->pix + ((y * 2) + field + DisplayRect.y) * surface->pitchinpix + DisplayRect.x;
   const int32 dly = ((y * 2) + (field + 1) + DisplayRect.y);
   uint32* dest = surface->pix + dly * surface->pitchinpix + DisplayRect.x;

   if(y == 0 && field)
   {
    uint32 black = 0;
    uint32* dm2 = surface->pix + (dly - 2) * surface->pitchinpix;

    LineWidths[dly - 2] = *src_lw;

    for(int x = 0; x < *src_lw; x++)
     dm2[x] = black;
   }

   if(dly < (DisplayRect.y + DisplayRect.h))
   {
    LineWidths[dly] = *src_lw;
    memcpy(dest, src, *src_lw * sizeof(uint32));
   }
  }

  //
  //
  //
  //
  //
  //
  if(DeintType == DEINT_WEAVE)
  {
   const int32 *src_lw = &LineWidths[(y * 2) + field + DisplayRect.y];
   const uint32* src = surface->pix + ((y * 2) + field + DisplayRect.y) * surface->pitchinpix + DisplayRect.x;
   uint32* dest = FieldBuffer->pix + y * FieldBuffer->pitchinpix;

   memcpy(dest, src, *src_lw * sizeof(uint32));
   LWBuffer[y] = *src_lw;

   StateValid = true;
  }
 }
}

void Deinterlacer::Process(MDFN_Surface *surface, MDFN_Rect &DisplayRect, const bool field)
{
 const MDFN_Rect DisplayRect_Original = DisplayRect;

 if(DeintType == DEINT_WEAVE)
 {
  if(!FieldBuffer || FieldBuffer->w < surface->w || FieldBuffer->h < (surface->h / 2))
  {
   if(FieldBuffer)
    delete FieldBuffer;

   FieldBuffer = new MDFN_Surface(NULL, surface->w, surface->h / 2, surface->w);
   LWBuffer.resize(FieldBuffer->h);
  }
 }

 int32 LineWidths[576 + 1];
 memcpy(LineWidths, FieldLineWidths[field], sizeof(LineWidths));
 InternalProcess(surface, DisplayRect, LineWidths, field);

 PrevDRect = DisplayRect_Original;
}

void Deinterlacer::ClearState(void)
{
 StateValid = false;

 PrevDRect.x = 0;
 PrevDRect.y = 0;

 PrevDRect.w = 0;
 PrevDRect.h = 0;
}
