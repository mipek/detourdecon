#include "testunit.h"
#include <DetourDecon.h>

template<int P_COUNT>
class TParamInfo
{
	param_t paraminf_[P_COUNT];
	int idx_;
public:
	TParamInfo():
	  idx_(0)
	{
	}
	param_t *ParamInfo()
	{
		return paraminf_;
	}
	void AddParam(size_t sz, paramType_e type)
	{
		paraminf_[idx_].size = sz;
		paraminf_[idx_++].type = type;
	}
};

namespace
{
	int GlobalInt = 0;

	void __cdecl SetGlobalInt(int a1)
	{
		GlobalInt = a1;
	}

	void __cdecl hkSetGlobalInt(int a1, DetourContext *ctx)
	{
		/* do nothing. */
		DETOUR_RETURN_VOID(ctx, Detour_Skip);
	}
}

class Test_cdecl: public TestUnit
{
public:
	Test_cdecl():
	  TestUnit("cdecl")
	{
	}

	virtual bool Run(char **testreason)
	{
		IDetourManager *pMngr = IDetourDecon::Singleton()->DetourManager();

		/* init detour. */
		prototype_t proto1;
		TParamInfo<1> paramInf1;
		paramInf1.AddParam(sizeof(int), ParamType_POD);

		proto1.callconv = CallConv_cdecl;
		proto1.paramCount = 1;
		proto1.params = paramInf1.ParamInfo();
		proto1.ret.size = 0;

		/* exec func to see see how it works. */
		SetGlobalInt(3);
		TEST_COND(GlobalInt != 3, "GlobalInt hasn't changed");

		/* now attach detour and test again. */
		IDetourCollection *pCollection = pMngr->Detour((byte*)SetGlobalInt, &proto1);
		pCollection->AddDetour((byte*)hkSetGlobalInt, Detour_Pre);
		SetGlobalInt(5);
		TEST_COND(GlobalInt != 3, "GlobalInt changed");
		return NULL;
	}
} sTest_cdecl;