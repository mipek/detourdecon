#include "detour.h"
#include "detourgen.h"
#include "patch.h"
#include "asmhelper.h"
using namespace ke;

class CDetour : public IDetour
{
	byte *detourFunc_;
	detourtype_e type_;
public:
	CDetour(byte *addr, detourtype_e type):
	  detourFunc_(addr), type_(type)
	{
	}

	virtual byte *CDetour::Callback() const
	{
		return detourFunc_;
	}

	virtual detourtype_e Type() const
	{
		return type_;
	}
    
    virtual void SetType(detourtype_e type)
    {
        type_ = type;
    }
};

class CDetourCollection : public IDetourCollection
{
	byte *origfunc_;
	byte *trampoline_;
	Vector<IDetour*> detours_;
	jmppatch_t patch_;
	friend IDetourCollection *CDetourManager::CreateDetourCollection(byte*, prototype_t*);

	CDetourCollection(byte *func):
	  origfunc_(func)
	{
	}

public:
	~CDetourCollection()
	{
		Patch_Restore(&patch_);
		for(size_t i=0; i<detours_.length(); ++i)
		{
			delete detours_[i];
		}
		DetourGen::Destroy(trampoline_);
	}

	virtual byte *Function() const
	{
		return origfunc_;
	}

	virtual byte *Trampoline() const
	{
		return trampoline_;
	}

	virtual IDetour *AddDetour(byte *cb, detourtype_e type)
	{
		IDetour *pDetour = static_cast<IDetour*>(new CDetour(cb, type));
		detours_.append(pDetour);
		return pDetour;
	}

	virtual bool RemoveDetour(IDetour *pDetour)
	{
		for(size_t i=0; i<detours_.length(); ++i)
		{
			if(detours_[i] == pDetour)
			{
				detours_.remove(i);
				delete pDetour;
				return true;
			}
		}
		return false;
	}

	virtual bool RemoveDetour(size_t idx)
	{
		if(idx < detours_.length())
		{
			CDetour *pDetour = static_cast<CDetour*>(detours_[idx]);
			if(pDetour)
			{
				detours_.remove(idx);
				delete pDetour;
				return true;
			}
		}
		return false;
	}

	virtual IDetour *GetDetour(size_t idx) const
	{
		if(idx < detours_.length())
		{
			return detours_[idx];
		}
		return NULL;
	}

	virtual int DetourCount() const
	{
		return detours_.length();
	}

public:
	void RemoveDetourAll()
	{
		detours_.clear();
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
		pCollection = CreateDetourCollection(addr, proto);
		m_collections.append(pCollection);
	}

	assert(pCollection);
	return pCollection;
}

IDetourCollection *CDetourManager::CreateDetourCollection(byte *func, prototype_t *proto)
{
	CDetourCollection *collection = new CDetourCollection(func);
	Patch_Create(&collection->patch_, func);

	int status = DetourGen::Generate(func, this, collection, proto, &collection->trampoline_);
	if(status == DETOURGEN_OK)
	{
		return collection;
	}

	/* Failed to generate detour manager - cleanup. */
	Patch_Destroy(&collection->patch_);
	delete collection;
	return NULL;
}