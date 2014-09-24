#include <DetourDecon.h>
#include "detour.h"
#include "memhack.h"

class CDetourDecon: public IDetourDecon
{
	CDetourManager detourMan_;
	CMemHack memHack_;
public:
	virtual IDetourManager *DetourManager()
	{
		return &detourMan_;
	}
	virtual IMemHack *MemHack()
	{
		return &memHack_;
	}
} sDetourDecon;

IDetourDecon *IDetourDecon::Singleton()
{
	return &sDetourDecon;
}