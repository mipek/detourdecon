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
#ifndef _include_detourdecon_h_
#define _include_detourdecon_h_

typedef unsigned char byte;

#define DD_SYS_WINDOWS 1
#define DD_SYS_LINUX 2

#define DD_VERSION_MAJOR 1
#define DD_VERSION_MINOR 0

#if defined(_WIN32)
#	define	DD_SYS DD_SYS_WINDOWS
#else
#	error	Unknown platform
#endif

// Macros
#define DETOUR_SET_STATUS(ctx, x) \
	ctx->status = x

#define DETOUR_RETURN(ret, ctx, stat) \
	DETOUR_SET_STATUS(ctx, stat); \
	return ret

#define DETOUR_RETURN_VOID(ctx, stat) \
	DETOUR_SET_STATUS(ctx, stat); \
	return

#include <cstddef>
#include <IDetour.h>
#include <IMemHack.h>

/**
 * Interface to the detour deconfliction layer.
 */
class IDetourDecon
{
public:
	virtual IDetourManager *DetourManager() =0;
	virtual IMemHack *MemHack() =0;

	static IDetourDecon *Singleton();
};

#endif //_include_detourdecon_h_
