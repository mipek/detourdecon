#include "detour.h"
#include "detourgen.h"
#include "patch.h"
#include "asmhelper.h"
using namespace ke;

class CDetour : public IDetour
{
	byte *m_pDetourFunc;
	detourtype_e m_eType;
public:
	CDetour(byte *addr, detourtype_e type):
	  m_pDetourFunc(addr), m_eType(type)
	{
	}

	virtual byte *CDetour::Callback() const
	{
		return m_pDetourFunc;
	}

	virtual detourtype_e Type() const
	{
		return m_eType;
	}
};

class CDetourCollection : public IDetourCollection
{
	byte *m_pOrigFunc;
	Vector<IDetour*> m_detours;
	jmppatch_t m_patch;

	CDetourCollection(byte *func):
	  m_pOrigFunc(func)
	{
	}

public:
	~CDetourCollection()
	{
		Patch_Restore(&m_patch);
		for(size_t i=0; i<m_detours.length(); ++i)
		{
			delete m_detours[i];
		}
	}

	virtual byte *Function() const
	{
		return m_pOrigFunc;
	}

	virtual void AddDetour(byte *cb, detourtype_e type)
	{
		m_detours.append(new CDetour(cb, type));
	}

	virtual bool RemoveDetour(IDetour *pDetour)
	{
		for(size_t i=0; i<m_detours.length(); ++i)
		{
			if(m_detours[i] == pDetour)
			{
				CDetour *pDetour = static_cast<CDetour*>(m_detours[i]);
				m_detours.remove(i);
				delete pDetour;
				return true;
			}
		}
		return false;
	}

	virtual bool RemoveDetour(int idx)
	{
		if(idx < m_detours.length())
		{
			CDetour *pDetour = static_cast<CDetour*>(m_detours[idx]);
			if(pDetour)
			{
				m_detours.remove(idx);
				delete pDetour;
				return true;
			}
		}
		return false;
	}

	virtual IDetour *GetDetour(int idx) const
	{
		if(idx < m_detours.length())
		{
			return m_detours[idx];
		}
		return NULL;
	}

	virtual int DetourCount() const
	{
		return m_detours.length();
	}

public:
	void RemoveDetourAll()
	{
		m_detours.clear();
	}

	static CDetourCollection *CreateOnAddress(byte *func, prototype_t *proto)
	{
		CDetourCollection *collection = new CDetourCollection(func);
		Patch_Create(&collection->m_patch, func);

		int status = DetourGen::Generate(func, collection, proto);
		if(status == DETOURGEN_OK)
		{
			return collection;
		}

		/* Failed to generate detour manager - cleanup. */
		Patch_Destroy(&collection->m_patch);
		delete collection;
		return NULL;
	}
};

CDetourManager::~CDetourManager()
{
	for(size_t i=0; i<m_collections.length(); ++i)
	{
		Destroy(m_collections[i]);
	}
}

IDetourCollection *CDetourManager::GetDetourCollection(byte *addr)
{
	for(size_t i=0; i<m_collections.length(); ++i)
	{
		IDetourCollection *pCollection = m_collections[i];
		if(pCollection->Function() == addr)
		{
			return pCollection;
		}
	}

	return NULL;
}

void CDetourManager::Destroy(IDetourCollection *pCollection)
{
	for(size_t i=0; i<m_collections.length(); ++i)
	{
		if(m_collections[i] == pCollection)
		{
			static_cast<CDetourCollection*>(pCollection)->RemoveDetourAll();
			delete pCollection;
			return;
		}
	}
}

IDetourCollection *CDetourManager::Detour(byte *addr, prototype_t *proto)
{
	// TODO: FollowJump?
	addr = FollowJump(addr);

	/* See if there is already a detour collection for this address. */
	IDetourCollection *pCollection = GetDetourCollection(addr);
	if(!pCollection)
	{
		/* No existing collection. Create a new! */
		pCollection = CDetourCollection::CreateOnAddress(addr, proto);
		m_collections.append(pCollection);
	}

	assert(pCollection);
	return pCollection;
}