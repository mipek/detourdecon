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
#ifndef _include_detourgen_h_
#define _include_detourgen_h_

#include <DetourDecon.h>

#define DETOURGEN_OK				0	// Generation was successfull.
#define DETOURGEN_TOOFEWBYTES		1	// Target function is not large enough to be detoured. Needs 5 bytes.
#define DETOURGEN_OUTOFMEMORY		2	// Out of memory.
//#define	DETOURGEN_ALREADYDETOURED	3	// Target function is already detoured.

/**
 * Internally manages the detour generation process.
 *
 * Short summary:
 * 1. Check if function is large enough to be detoured(we need atleast 5 bytes for a JMP).
 * 2. Copy the first few instructions of the function to a trampoline so we can overwrite them.
 * 3. Replace original instructions with NOPs to prevent us from struggling with encoding problems later on.
 * 4. Inject a JMP in our trampoline to allow normal execution of the original code.
 * 5. Generate a detour manager that calls all listeners.
 * 6. Patch the original function so it jumps to our detour manager.
 */
class DetourGen
{
	static byte *CopyToTrampoline(byte *targetAddr, size_t copySize);
	static byte *GenerateManager(byte *targetAddr, byte *trampoline, prototype_t *proto);
	static void PatchNOP(byte *targetAddr, size_t size);
	static void PatchJump(byte *targetAddr, byte *genAddr);

public:
	static int Generate(byte *targetAddr, IDetourCollection *pCollection, prototype_t *proto);
	static const char *GetErrorString(int error);
};

#endif //_include_detourgen_h_