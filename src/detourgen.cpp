#include "detourgen.h"
#include "platform_compat.h"
#include "disasm.h"
#include "asmhelper.h"
#include "asmjit.h"
#include <am-vector.h>
using namespace asmjit;
using namespace asmjit::host;

#if defined(ASMJIT_HOST_X86)
#	define	REQUIRED_FUNC_SIZE	5
#else
#	error
#endif

enum
{
	kOPCode_NOP = 0x90,			/**< Perform no operation. */
	kOPCode_JMPREL32 = 0xE9,	/**< Intersegment relative jump. */
	kOPCode_RET = 0xC3,			/**< Return from procedure(intersegment). */
	kOPCode_RETN = 0xC2,		/**< Return from procedure(intersegment) and pop imm16 from stack. */
	kOPCode_INT3 = 0xCC			/**< Interrupt 3 (aka breakpoint). */
};

JitRuntime g_Runtime;	/**< Global runtime object used for code generation. */

/* Detour manager locals. */
static const int var_detourCollection = -4;
static const int var_detourCount = -8;
static const int var_detourIndex = -12;
static const int var_detourObject = -16;
static const int var_detourContext = -20;
static const int var_returnValue = -24;//var_detourContext - sizeof(DetourContext);

class DetourGenContext
{
	X86Assembler &a_;
	prototype_t *proto_;
public:
	DetourGenContext(X86Assembler &a, prototype_t *proto):
		a_(a), proto_(proto)
	{
	}

	/**
	 * Generates the function prolog. Reserve space for local variables, preserve registers.
	 */
	inline void GenProlog(uint32_t localVarSize)
	{
		a_.push(ebp);
		a_.mov(ebp, esp);
		a_.sub(esp, localVarSize);

	#if DD_COMPILER == COMPILER_MSVC
		if(proto_->callconv == CallConv_thiscall)
		{
			/* Preserve thisptr in ESI */
			a_.push(esi);
			a_.mov(esi, ecx);
		}
	#endif
	}

	/**
	 * Generates the function epilog. Stack cleanup, register restoration.
	 */
	inline void GenEpilog(uint32_t cleanupSize)
	{
		a_.mov(esp, ebp);

	#if DD_COMPILER == COMPILER_MSVC
		if(proto_->callconv == CallConv_thiscall)
		{
			/* Restore ESI */
			a_.pop(esi);
		}
	#endif

		a_.pop(ebp);
		if(proto_->callconv == CallConv_stdcall || proto_->callconv == CallConv_thiscall)
		{
			a_.ret(cleanupSize);
		} else {
			a_.ret();
		}
	}

	/**
	 * Generate detour loop body. Tests if IDetour::Type is zero
	 */
	void GenDetourLoopBody()
	{
		a_.mov(ecx, dword_ptr(ebp, var_detourCollection));
		a_.mov(eax, dword_ptr(ebp, var_detourIndex));
		a_.push(eax);
		a_.mov(eax, dword_ptr(ecx));
		a_.call(ptr(eax, GetVFuncOffset(&IDetourCollection::GetDetour)));
		GenCleanupStackThiscall(sizeof(int));
		a_.mov(dword_ptr(ebp, var_detourObject), eax);
		a_.mov(ecx, eax);
		a_.mov(eax, dword_ptr(eax));
		a_.call(dword_ptr(eax, GetVFuncOffset(&IDetour::Type)));
		a_.test(eax, eax);
	}

	void GenPushParameters(int paramTotalSize)
	{
		/* Work out the highest displacement(aka last argument) because we're
		 * going to push the parameters in reverse order(right to left).
		 *
		 * ebp+4 is return address (hence the +4) */
		int paramDisplacement = paramTotalSize + 4;

		for(int i=proto_->paramCount; i-- > 0;)
		{
			paramDisplacement -= GenPushSingleParam(proto_->params[i], paramDisplacement);
		}
		assert(paramDisplacement == 4);

#if DD_COMPILER == COMPILER_MSVC
		if(proto_->callconv == CallConv_thiscall)
		{
			a_.mov(ecx, esi);
		}
#endif
	}

	void GenCleanupStack(int size)
	{
		/* Check if callconv of the prototype is callee cleanup. */
		if(proto_->callconv == CallConv_cdecl)
		{
			a_.add(esp, size);
		}

#if DD_COMPILER == COMPILER_GNUC
		else if(proto_->callconv == CallConv_thiscall)
		{
			a_.add(esp, size + sizeof(void*));
		}
#endif
	}

	void GenCleanupStackThiscall(int size)
	{
#if DD_COMPILER == COMPILER_GNUC
		a_.add(esp, thisSize);
#elif DD_COMPILER == COMPILER_MSVC
		// TODO: if variadic args -> cleanup
#endif
	}

private:
	size_t GenPushSingleParam(const param_t &param, int displacement)
	{
		switch(param.type)
		{
		case ParamType_POD:
			switch(param.size)
			{
			case 1: /* byte */
				a_.movzx(edx, byte_ptr(ebp, displacement));
				a_.push(edx);
				return 1;
			case 2: /* word */
				a_.movzx(edx, word_ptr(ebp, displacement));
				a_.push(edx);
				return 2;
			case 4: /* dword */
				a_.mov(edx, dword_ptr(ebp, displacement));
				a_.push(edx);
				return 4;
			case 8: /* qword */
				a_.mov(eax, dword_ptr(ebp, displacement-4));
				a_.mov(edx, dword_ptr(ebp, displacement));
				a_.push(eax);
				a_.push(edx);
				return 8;
			default:
				assert(!"cannot handle param size");
				break;
			}
			break;
		case ParamType_ByRef:
			a_.lea(edx, dword_ptr(ebp, displacement));
			a_.push(edx);
			return param.size;
			break;
		default:
			assert(!"cannot handle param type");
			break;
		}
		return 0;
	}
};

byte *DetourGen::CopyToTrampoline(byte *targetAddr, size_t copySize)
{
	/* Get enough space for the trampoline. */
	//uint8_t *pTrampoline = (uint8_t*)g_PagePool.Alloc(copySize + REQUIRED_FUNC_SIZE);
	VMemMgr *memMngr = g_Runtime.getMemMgr();
	uint8_t *pTrampoline = (uint8_t*)memMngr->alloc(copySize + REQUIRED_FUNC_SIZE);
	if(!pTrampoline)
	{
		return NULL;
	}

	/* Copy the bytes that we're going to overwrite to the trampoline. */
	memcpy(pTrampoline, targetAddr, copySize);

	return (byte*)pTrampoline;
}

byte *DetourGen::GenerateManager(IDetourManager *pMngrSingleton, byte *targetAddr, byte *trampoline, prototype_t *proto)
{
	X86Assembler a(&g_Runtime);

	Label L_PreLoop(a);
	Label L_PreLoopRewind(a);
	Label L_CallDetour(a);
	Label L_CallOrig(a);
	Label L_CallOrigProcedure(a);
	Label L_OverrideReturn(a);
	Label L_Epilog(a);

	/* Calculate total paramter size in bytes. */
	size_t paramTotalSize = 0;
	for(int i=0; i<proto->paramCount; ++i)
	{
 		paramTotalSize += proto->params[i].size;
	}

	DetourGenContext genCtx(a, proto);

	/* Function prolog. */
	genCtx.GenProlog(sizeof(IDetourCollection*) + sizeof(int32_t)*2 + sizeof(IDetour*) + sizeof(DetourContext) + proto->ret.size);

	/* Init detour context. */
	a.mov(dword_ptr(ebp, var_detourContext), Detour_Ignored);

	/* Get IDetourManager */
	a.mov(ecx, (Ptr)pMngrSingleton);

	/* Fetch detour collection for 'targetAddr' */
	a.mov(eax, dword_ptr(ecx));
	a.push((uint64_t)targetAddr);
	a.call(dword_ptr(eax, GetVFuncOffset(&IDetourManager::GetDetourCollection)));
	genCtx.GenCleanupStackThiscall(sizeof(uint64_t));
	a.mov(dword_ptr(ebp, var_detourCollection), eax);

	/* Get total count of detours in this collection. */
	a.mov(ecx, eax);
	a.mov(eax, dword_ptr(eax));
	a.call(dword_ptr(eax, GetVFuncOffset(&IDetourCollection::DetourCount)));
	a.mov(dword_ptr(ebp, var_detourCount), eax);
	a.mov(dword_ptr(ebp, var_detourIndex), 0);

	/* Detour loop. */
	a.bind(L_PreLoop);
	genCtx.GenDetourLoopBody();
	a.jz(L_CallDetour);

	/* Check if we've to loop once again. */
	a.bind(L_PreLoopRewind);
	a.mov(eax, dword_ptr(ebp, var_detourIndex));
	a.inc(eax);
	a.mov(dword_ptr(ebp, var_detourIndex), eax);
	a.cmp(eax, dword_ptr(ebp, var_detourCount));
	a.jl(L_PreLoop);
	a.jmp(L_CallOrig);
	
	/* Call the detour; Push detour context to stack first to make sure it's the last argument. */
	a.bind(L_CallDetour);
	a.lea(edx, dword_ptr(ebp, var_detourContext));
	a.push(edx);
	
	genCtx.GenPushParameters(paramTotalSize);

	/* Now, call it! */
	a.mov(ecx, dword_ptr(ebp, var_detourObject));
	a.mov(eax, dword_ptr(ecx));
	a.call(dword_ptr(eax, GetVFuncOffset(&IDetour::Callback)));
	a.call(eax);
	genCtx.GenCleanupStack(paramTotalSize + sizeof(DetourContext*));
	
	if(proto->ret.size > 0)
	{
		/* Figure what to do with the return. */
		a.mov(edx, dword_ptr(ebp, var_detourContext));
		a.cmp(edx, Detour_Override);
		a.jl(L_PreLoopRewind);
		a.mov(dword_ptr(ebp, var_returnValue), eax);
	}
	a.jmp(L_PreLoopRewind);

	/* See if we need to call the original function. */
	a.bind(L_CallOrig);
	a.mov(edx, dword_ptr(ebp, var_detourContext));
	a.cmp(edx, Detour_Override);
	a.jl(L_CallOrigProcedure); // if ctx.status < Detour_Override -> call orig

	a.cmp(edx, Detour_Skip);
	if(proto->ret.size > 0)
		a.je(L_OverrideReturn);
	else
		a.je(L_Epilog);

	/* Call original procedure. */
	a.bind(L_CallOrigProcedure);
	genCtx.GenPushParameters(paramTotalSize);
	a.call((Ptr)trampoline);
	genCtx.GenCleanupStack(paramTotalSize);

	if(proto->ret.size > 0)
	{
		/* Check if a callback overrode the function return value. */
		a.mov(edx, dword_ptr(ebp, var_detourContext));
		a.cmp(edx, Detour_Override);
		a.je(L_OverrideReturn);
	}

	/* Function epilog.*/
	a.bind(L_Epilog);
	genCtx.GenEpilog(paramTotalSize);

	/* Return the override value. */
	if(proto->ret.size > 0)
	{
		a.bind(L_OverrideReturn);
		a.mov(eax, dword_ptr(ebp, var_returnValue));
		a.jmp(L_Epilog);
	}

	return (byte*)a.make();
}

void DetourGen::PatchNOP(byte *targetAddr, size_t size)
{
	/* Make patchable. */
	SetPageRWExec(targetAddr, size);

	memset(targetAddr, kOPCode_NOP, size);
	// TODO: restore old protection
}

void DetourGen::PatchJump(byte *targetAddr, byte *genAddr)
{
	/* Calculate JMP offset. */
	unsigned long jmp_offs = genAddr - (targetAddr+5);
	
	/* Make the function patchable. */
	SetPageRWExec(targetAddr, REQUIRED_FUNC_SIZE);

	/* Patch the function. */
	*targetAddr = kOPCode_JMPREL32;
	*(unsigned long*)(targetAddr+1) = jmp_offs;

	// TODO: restore old protection
}

int DetourGen::Generate(byte *targetAddr, IDetourManager *pMngrSingleton, IDetourCollection *pCollection, prototype_t *proto, byte **trampoline)
{
	int curSize = 0;
	CDisasm *disasm = GetDisassembler();

	while(curSize < REQUIRED_FUNC_SIZE)
	{
		curSize += disasm->GetInstructionLength(targetAddr+curSize);
		if(*targetAddr == kOPCode_RET || *targetAddr == kOPCode_RETN || *targetAddr == kOPCode_INT3)
		{
			break;
		}
	}

	/* Is function large enough? */
	if(curSize < REQUIRED_FUNC_SIZE)
	{
		return DETOURGEN_TOOFEWBYTES;
	}

	/* Copy the first few bytes(that we're going to overwrite) to a trampoline. */
	*trampoline = CopyToTrampoline(targetAddr, curSize);
	if(!(*trampoline))
	{
		return DETOURGEN_OUTOFMEMORY;
	}

	/* Replace the original code with NOPs so we don't end up destroying the encoding
	 * when patching in the jump(we jump over it anyway so it doesnt really matter anyway). */
	PatchNOP(targetAddr, curSize);

	/* Make the trampoline return to the original function. */
	PatchJump(*trampoline + curSize, targetAddr + curSize); 

	/* Redirect the original function to our manager. */
	PatchJump(targetAddr, GenerateManager(pMngrSingleton, targetAddr, *trampoline, proto));

	return DETOURGEN_OK;
}

void DetourGen::Destroy(byte *trampoline)
{
	byte *detourmngr = FollowJump(trampoline);
	if(detourmngr != trampoline)
	{
		g_Runtime.release(detourmngr);
	}
	g_Runtime.release(trampoline);
}

const char *DetourGen::GetErrorString(int error)
{
	static const char *DETOUR_ERROR_STR[] = {
		"DetourManager was successfully created.",
		"Target function doesn't have enough bytes to be detoured.",
		"Out of memory.",
		"Target function is already detoured."};
	return DETOUR_ERROR_STR[error];
}