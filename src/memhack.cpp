#include "memhack.h"

bool CMemHack::IsPatched(const void *addr, size_t len)
{
	return false;
}

void CMemHack::Patch(const void *addr, const void *bytes, size_t len)
{
}

int CMemHack::AddPattern(const char *pattern, size_t len, OnPatternMatch_t cb)
{
}

void CMemHack::Scan(const void *module, size_t len)
{
}