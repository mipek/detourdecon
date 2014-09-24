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
#ifndef _include_idetour_h_
#define _include_idetour_h_

#include <FunctionInfo.h>

class IDetourCollection;

/**
 * Available detouring types.
 * Do not modify ordering.
 */
typedef enum eDetourType
{
	Detour_Pre = 0,
	Detour_Post
} detourtype_e;

/**
 * Detour status list.
 * Ordering is crucial, do not touch.
 */
typedef enum eDetourStatus
{
	Detour_Ignored,		/**< Don't take any action; call original function. */
	Detour_Handled,		/**< Let other detours know we changed something. */
	Detour_Override,	/**< Use my return but call original function. */
	Detour_Skip			/**< Use my return, don't call original function. >*/
} detourstatus_e;

/**
 * Context passed along the function parameters.
 */
typedef struct DetourContext
{
	detourstatus_e status;
	//bool ignoreDetours;
} detourctx_t;

/**
 * A function prototype.
 */
typedef void (*func_t)();

class IDetour
{
public:
    /**
     * @brief Returns the detour callback address.
     *
     * @return              Detour callback address.
     */
    virtual byte *Callback() const =0;

	/**
	 * @brief Returns the detouring type
	 *
	 * @return				Detour type.
	 */
	virtual detourtype_e Type() const =0;
};

class IDetourCollection
{
public:
	/**
     * @brief Returns the address of the detoured function.
     *
     * @return              Original function address.
     */
    virtual byte *Function() const =0;

	/**
	 * @brief Associate a detour with this collection.
	 *
	 * @param	cb			Callback
	 * @param	type		Detouring type.
	 */
	virtual void AddDetour(byte *cb, detourtype_e type) =0;

	/**
	 * @brief Removes the specified detour from collection and deletes it.
	 *
	 * @param	pDetour		Detour instance.
	 * @return				True if successfull
	 */
	virtual bool RemoveDetour(IDetour *pDetour) =0;

	/**
	 * @brief Removes the specified detour from collection and deletes it.
	 *
	 * @param	idx			Index of the detour to remove
	 */
	virtual bool RemoveDetour(int idx) =0;

    /**
     * @brief Retrieves a detour instance.
     *
     * @param   idx         Index of the detour.
     * @return              Detour instance.
     */
    virtual IDetour *GetDetour(int idx) const =0;

    /**
     * @brief Total amount of detours attached to this collection.
     *
     * @return              Total amount of detours.
     */
    virtual int DetourCount() const =0;
};

class IDetourManager
{
public:
    /**
     * @brief Retrieves a existing detour collection for the desired address.
     *
     * @param   addr        Function address.
     * @return              Detour collection or NULL if none is found.
     */
    virtual IDetourCollection *GetDetourCollection(byte *addr) =0;

	/**
     * @brief Destroys a detour collection.
     *
     * @param	pCollection	Pointer to a detour collection.
     */
	virtual void Destroy(IDetourCollection *pCollection) =0;

	/**
     * @brief Detours the specified function to a detour manager.
     *
     * @param   addr        Function address.
	 * @param	type		Detouring type.
	 * @param	proto		Function prototype.
     * @return              Associated detour collection or NULL if failed to create.
     */
    virtual IDetourCollection *Detour(byte *addr, prototype_t *proto) =0;

    bool IsDetoured(byte *addr)
    {
        return GetDetourCollection(addr) != NULL;
    }
};

#endif //_include_idetour_h_
