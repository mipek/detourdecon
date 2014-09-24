#include <DetourDecon.h>
#include <iostream>
#include <cassert>
using namespace std;

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

int g_iGlobalVar = 0;
void __cdecl WriteToGlobal(int a1)
{
	printf("WriteToGlobal(%d)\n", a1);
	g_iGlobalVar = a1;
}

int __cdecl Return2()
{
	return 2;
}

void __cdecl hkWriteToGlobal(int a1, DetourContext *ctx)
{
	printf("hkWriteToGlobal(%d)\n", a1);
	g_iGlobalVar = 4;

	DETOUR_RETURN_VOID(ctx, Detour_Skip);
}

int __cdecl hkReturn2_a(DetourContext *ctx)
{
	printf("hkReturn2 a()\n");
	DETOUR_RETURN(4, ctx, Detour_Override);
}


int __cdecl hkReturn2_b(DetourContext *ctx)
{
	assert(ctx->status == Detour_Override);

	printf("hkReturn2 b()\n");
	DETOUR_RETURN(4, ctx, Detour_Skip);
}

int main(int argc, char *argv[])
{
	IDetourManager *pMngr = IDetourDecon::Singleton()->DetourManager();

	prototype_t proto1, proto2;
	TParamInfo<1> paramInf1;
	paramInf1.AddParam(sizeof(char*), ParamType_POD);

	proto1.callconv = CallConv_cdecl;
	proto1.paramCount = 1;
	proto1.params = paramInf1.ParamInfo();
	proto1.ret.size = 0;

	proto2.callconv = CallConv_cdecl;
	proto2.paramCount = 0;
	proto2.params = NULL;
	proto2.ret.size = sizeof(int);
	proto2.ret.type = ParamType_POD;

	IDetourCollection *pCollection = pMngr->Detour((byte*)WriteToGlobal, &proto1);
	pCollection->AddDetour((byte*)hkWriteToGlobal, Detour_Pre);

	IDetourCollection *pCollection2 = pMngr->Detour((byte*)Return2, &proto2);
	pCollection2->AddDetour((byte*)hkReturn2_a, Detour_Pre);
	pCollection2->AddDetour((byte*)hkReturn2_b, Detour_Pre);

	WriteToGlobal(2);
	printf("GlobalVar: %d\n", g_iGlobalVar);
	printf("ReturnValue: %d\n", Return2());

	assert(g_iGlobalVar == 4);
	assert(Return2() == 4);

	int dummy;
	scanf("%d", &dummy);
	return 0;
}