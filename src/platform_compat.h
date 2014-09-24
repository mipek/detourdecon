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
#ifndef _include_platform_compat_h_
#define _include_platform_compat_h_

#include <stddef.h>

#define	COMPILER_MSVC	1	/* Microsoft Visual Studio. */
#define COMPILER_GNUC	2	/* GNU GCC/GPP */	

#if defined(_MSC_VER)
#	define DD_COMPILER	COMPILER_MSVC
#elif defined(__GNUC__)
#	define DD_COMPILER	COMPILER_GNUC
#endif

#if DD_SYS == DD_SYS_WINDOWS
#	define	WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#	define	PAGEPROT_READ	0x1
#	define	PAGEPROT_RW		0x2
#	define	PAGEPROT_RWEXEC	0x4
#elif DD_SYS == DD_SYS_LINUX
#	define	PAGEPROT_READ	PROT_READ
#	define	PAGEPROT_RW		PROT_READ|PROT_WRITE
#	define	PAGEPROT_RWEXEC	PROT_READ|PROT_WRITE|PROT_EXEC
#endif

static bool SetPageProtection(byte *addr, size_t len, int prot)
{
#if DD_SYS == DD_SYS_WINDOWS
	DWORD dwProt;
	switch(prot)
	{
	case PAGEPROT_READ:
		dwProt = PAGE_READONLY;
		break;
	case PAGEPROT_RW:
		dwProt = PAGE_READWRITE;
		break;
	case PAGEPROT_RWEXEC:
		dwProt = PAGE_EXECUTE_READWRITE;
	}
	return VirtualProtect((LPVOID)addr, len, dwProt, &dwProt) != FALSE;
#elif DD_SYS == DD_SYS_LINUX
#	error impl me
#endif
}

static bool SetPageRWExec(byte *addr, size_t len)
{
	return SetPageProtection(addr, len, PAGEPROT_RWEXEC);
}

static byte* AllocExecPage(size_t sz)
{
#if DD_SYS == DD_SYS_WINDOWS
	return (byte*)VirtualAlloc(NULL, sz, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
#elif DD_SYS == DD_SYS_LINUX
#	error impl me
#endif
}

static void FreeExecPage(byte *addr)
{
#if DD_SYS == DD_SYS_WINDOWS
	VirtualFree(addr, 0, MEM_RELEASE);
#elif DD_SYS == DD_SYS_LINUX
#	error impl me
#endif
}

#endif //_include_platform_compat_h_