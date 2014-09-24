#ifndef _include_memhack_h_
#define _include_memhack_h_

#include <IMemHack.h>

class CMemHack: public IMemHack
{
public:
	virtual bool IsPatched(const void *addr, size_t len);
	virtual void Patch(const void *addr, const void *bytes, size_t len);
	virtual int AddPattern(const char *pattern, size_t len, OnPatternMatch_t cb);
	virtual void Scan(const void *module, size_t len);
};

#endif //_include_memhack_h_