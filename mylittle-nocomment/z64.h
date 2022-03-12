#ifndef __Z64_H__
#define __Z64_H__

#include <stdio.h>
#include <stdint.h>

typedef uint64_t UINT64;
typedef int64_t INT64;
typedef uint32_t UINT32;
typedef int32_t INT32;
typedef uint16_t UINT16;
typedef int16_t INT16;
typedef uint8_t UINT8;
typedef int8_t INT8;

#define SP_INTERRUPT	0x1
#define SI_INTERRUPT	0x2
#define AI_INTERRUPT	0x4
#define VI_INTERRUPT	0x8
#define PI_INTERRUPT	0x10
#define DP_INTERRUPT	0x20

#define SP_STATUS_HALT			0x0001
#define SP_STATUS_BROKE			0x0002
#define SP_STATUS_DMABUSY		0x0004
#define SP_STATUS_DMAFULL		0x0008
#define SP_STATUS_IOFULL		0x0010
#define SP_STATUS_SSTEP			0x0020
#define SP_STATUS_INTR_BREAK	0x0040
#define SP_STATUS_SIGNAL0		0x0080
#define SP_STATUS_SIGNAL1		0x0100
#define SP_STATUS_SIGNAL2		0x0200
#define SP_STATUS_SIGNAL3		0x0400
#define SP_STATUS_SIGNAL4		0x0800
#define SP_STATUS_SIGNAL5		0x1000
#define SP_STATUS_SIGNAL6		0x2000
#define SP_STATUS_SIGNAL7		0x4000

#define DP_STATUS_XBUS_DMA		0x01
#define DP_STATUS_FREEZE		0x02
#define DP_STATUS_FLUSH			0x04
#define DP_STATUS_START_GCLK		0x008
#define DP_STATUS_TMEM_BUSY		0x010
#define DP_STATUS_PIPE_BUSY		0x020
#define DP_STATUS_CMD_BUSY			0x040
#define DP_STATUS_CBUF_READY		0x080
#define DP_STATUS_DMA_BUSY			0x100
#define DP_STATUS_END_VALID		0x200
#define DP_STATUS_START_VALID		0x400

#define R4300i_SP_Intr 1


#define LSB_FIRST 1 
#ifdef LSB_FIRST
	#define BYTE_ADDR_XOR		3
	#define WORD_ADDR_XOR		1
	#define BYTE4_XOR_BE(a) 	((a) ^ 3)				
#else
	#define BYTE_ADDR_XOR		0
	#define WORD_ADDR_XOR		0
	#define BYTE4_XOR_BE(a) 	(a)
#endif

#ifdef LSB_FIRST
#define BYTE_XOR_DWORD_SWAP 7
#define WORD_XOR_DWORD_SWAP 3
#else
#define BYTE_XOR_DWORD_SWAP 4
#define WORD_XOR_DWORD_SWAP 2
#endif
#define DWORD_XOR_DWORD_SWAP 1

#define INLINE
#define STRICTINLINE	inline

#define PRESCALE_WIDTH 640
#define PRESCALE_HEIGHT 625

typedef unsigned int offs_t;

#define rdram ((UINT32*)ares::Nintendo64::rdram.ram.data)
#define rsp_dmem ((UINT32*)ares::Nintendo64::rsp.dmem.data)

#define rdram16 ((UINT16*)ares::Nintendo64::rdram.ram.data)
#define rdram8 ((UINT8*)ares::Nintendo64::rdram.ram.data)

#define vi_control (ares::Nintendo64::vi.readWord(0 << 2))
#define vi_origin (ares::Nintendo64::vi.readWord(1 << 2))
#define vi_width (ares::Nintendo64::vi.readWord(2 << 2))

#define vi_v_current_line (ares::Nintendo64::vi.readWord(4 << 2))

#define vi_v_sync (ares::Nintendo64::vi.readWord(6 << 2))

#define vi_h_start (ares::Nintendo64::vi.readWord(9 << 2))
#define vi_v_start (ares::Nintendo64::vi.readWord(10 << 2))

#define vi_x_scale (ares::Nintendo64::vi.readWord(12 << 2))
#define vi_y_scale (ares::Nintendo64::vi.readWord(13 << 2))

#define dp_start (ares::Nintendo64::rdp.readWord(0 << 2))
#define dp_end (ares::Nintendo64::rdp.readWord(1 << 2))
#define dp_current (ares::Nintendo64::rdp.readWord(2 << 2))
#define dp_status (ares::Nintendo64::rdp.readWord(3 << 2))

#endif
