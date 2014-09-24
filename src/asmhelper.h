/**
 * Copyright (c) 2014, Michael Pekar
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *	  
 *   * Neither the name of the copyright holder nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _include_asmhelper_h_
#define _include_asmhelper_h_

#include "platform_compat.h"
#include <stdint.h>
#include <assert.h>

/**
 * Follows a jump (if any).
 */
inline byte *FollowJump(byte *addr)
{
	if(addr[0] == 0xE9) /* jmp rel32 */
	{
		return addr + 5 + *(unsigned long*)(addr+1);
	}
	return addr;
}

/**
 * Get virtual function offset from thunk.
 */
template<class T>
size_t GetVFuncOffset(T fn)
{
#if DD_COMPILER == COMPILER_MSVC
	/* credits: learn_more */
	union {
		T vfn;
		uint8_t *addr;
	};
	vfn = fn;
	if(*addr == 0xE9)
	{
		int offs;
		addr = FollowJump(addr);

		if(addr[0] == 0x8B && addr[1] == 0x01) /* mov eax, dword ptr [ecx] */
		{
			offs = 2;
		} else if(addr[0] == 0x8B && addr[1] == 0x44 && addr[2] == 0x24 && /* mov eax,dword ptr [esp+4] */
				  addr[3] == 0x04 && addr[4] == 0x8B && addr[5] == 0x00)   /* mov eax,dword ptr [eax] */
		{
			offs = 6;
		}
		else {
			assert(!"unknown thunk code");
		}

		/* examine the jump. */
		if(addr[offs] == 0xff)
		{
			if(addr[offs+1] == 0x20) // no disp
			{
				/* FF 20 | jmp dword ptr [eax] */
				return 0;
			} else if(addr[offs+1] == 0x60) // 8 byte disp
			{
				/* FF 60 04 | jmp dword ptr [eax+4] */
				return addr[offs+2];
			} else if(addr[offs+1] == 0xA0) // 32 byte disp
			{
				/* FF A0 38 04 00 00 | jmp dword ptr [eax+438h] */
				return *(int32_t*)(addr + offs + 2);
			}
		}
		assert(!"unknown thunk branch");
	}
#else
#		error add support pl0x
#endif
	assert(!"couldnt get vfunc offs");
	return 0;
}

#endif //_include_asmhelper_h_