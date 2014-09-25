#ifndef _include_detour_h_
#define _include_detour_h_

#include <DetourDecon.h>
#include <am-vector.h>

class CDetourManager : public IDetourManager
{
	ke::Vector<IDetourCollection*> m_collections;
public:
	~CDetourManager();

	virtual IDetourCollection *GetDetourCollection(byte *addr);
	virtual void Destroy(IDetourCollection *pCollection);
	virtual IDetourCollection *Detour(byte *addr, prototype_t *proto);

	IDetourCollection *CreateDetourCollection(byte *addr, prototype_t *proto);
};

#endif //_include_detour_h_