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
 *   * Neither the name of the copyright holder nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
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
#ifndef _include_pagepool_h_
#define _include_pagepool_h_

#include <cstddef>
#include <cassert>
#include <am-linkedlist.h>
#include <am-inlinelist.h>
#include "platform_compat.h"

#if DD_SYS == DD_SYS_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#else
#error unsupported system
#endif

/**
 * An allocator that is specifically designed for JIT code generation.
 * Please note that this allocator is extremely slow.
 * Avoid making single, small allocations.
 *
 * All memory you get from the pool is guaranteed to have read, write and execute permissions.
 */
class CPagePool
{
public:
	class CPage;

	/**
	 * Describes a region in a page that is already allocated.
	 */
	class CAllocRegion: public ke::InlineListNode<CAllocRegion>
	{
		friend class CPage;
		size_t m_begin;
		size_t m_size;
	public:
		CAllocRegion(size_t begin, size_t size):
		  m_begin(begin), m_size(size)
		{
		}
		size_t Start() const
		{
			return m_begin;
		}

		size_t Size() const
		{
			return m_size;
		}

		size_t End() const
		{
			return m_begin + m_size;
		}
	};

	/**
	 * A single page.
	 */
	class CPage
	{
		bool m_hasRegions;
		ke::InlineList<CAllocRegion> m_regions;
		void *m_address;
		size_t m_size;

		void *AddRegion(size_t begin, size_t size)
		{
			m_hasRegions = true;
			CAllocRegion *pRegion = new CAllocRegion(begin, size);
			m_regions.append(pRegion);
			return (void*)pRegion->Start();
		}
		void *InsertRegion(CAllocRegion *after, CAllocRegion *before, size_t begin, size_t size)
		{
			m_hasRegions = true;
			CAllocRegion *pRegion = new CAllocRegion(begin, size);
			after->next_ = pRegion;
			pRegion->prev_ = after;
			pRegion->next_ = before;
			return (void*)pRegion->Start();
		}
	public:
		CPage(size_t allocSize):
			m_size(allocSize), m_hasRegions(false)
		{
#if DD_SYS == DD_SYS_WINDOWS
			m_address = VirtualAlloc(NULL, allocSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
#else
			SetPageProtection(m_address, allocSize, PAGEPROT_RWEXEC);
#endif

#if defined(DEBUG)
			memset(m_address, 0xCC, allocSize);
#endif
		}
		~CPage()
		{
			for(ke::InlineList<CAllocRegion>::iterator iter = m_regions.begin(); iter!=m_regions.end(); iter++)
			{
				delete *iter;
			}
#if DD_SYS == DD_SYS_WINDOWS
			VirtualFree(m_address, 0, MEM_RELEASE);
#else
#endif
		}

		void *AllocRegion(size_t size)
		{
			if(m_hasRegions)
			{
				/* Look for gaps between regions. */
				ke::InlineList<CAllocRegion>::iterator iter = m_regions.begin();
				ke::InlineList<CAllocRegion>::iterator iter2 = m_regions.begin();
				iter2++;
				for(;iter!=m_regions.end(), iter2!=m_regions.end(); iter++, iter2++)
				{
					CAllocRegion *pRegion1 = *iter;
					CAllocRegion *pRegion2 = *iter2;

					if(pRegion1 == pRegion2)
					{
						break;
					}

					/* Check if the gap between Region1 and Region2 is atleast 'size' bytes large. */
					size_t end1 = pRegion1->End();
					size_t end2 = pRegion2->Start();
					if(end2 - end1 > size)
					{
						/* Insert the new region right after 'pRegion1', aka the first. */
						return InsertRegion(pRegion1, pRegion2, end1+1, size);
					}
				}

				// TODO: *m_regions.end() returns some weird stuff. This workarounds that.
				CAllocRegion *pLastRegion;
				for(ke::InlineList<CAllocRegion>::iterator iter = m_regions.begin(); iter!=m_regions.end(); iter++)
				{
					pLastRegion = *iter;
				}

				/* See if there's some space left in this page. */
				size_t end = pLastRegion->End();
				if(end+size+1 < End())
				{
					return AddRegion(end+1, size);
				}
			} else {
				return AddRegion((size_t)m_address, size);
			}
			return NULL;
		}

		bool FreeRegion(void *memory)
		{
			for(ke::InlineList<CAllocRegion>::iterator iter = m_regions.begin(); iter!=m_regions.end(); iter++)
			{
				CAllocRegion *pRegion = *iter;
				if((void*)pRegion->Start() == memory)
				{
					m_regions.erase(iter);
					delete pRegion;
					return true;
				}
			}

			return false;
		}

		bool IsWithin(void *addr) const
		{
			size_t addressEnd = (size_t)m_address + m_size;
			if(addr >= m_address && addr < (void*)addressEnd)
			{
				return true;
			}
			return false;
		}

		void *Address() const
		{
			return m_address;
		}

		size_t End() const
		{
			return (size_t)m_address + m_size;
		}

#if defined(DEBUG)
		void DebugPrint()
		{
			int counter = 0;
			printf("Page range: 0x%x -> 0x%x\n", Address(), End());
			for(ke::InlineList<CAllocRegion>::iterator iter = m_regions.begin(); iter!=m_regions.end(); iter++)
			{
				CAllocRegion *pRegion = *iter;
				printf("- Region #%d: 0x%x -> 0x%x\n", ++counter, pRegion->Start(), pRegion->End());
			}
		}
#endif
	};

private:
	ke::LinkedList<CPage*> m_pages;
	size_t m_pageSize;

public:
	CPagePool()
	{
#if DD_SYS == DD_SYS_WINDOWS
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		m_pageSize = si.dwPageSize;
#endif
	}
	~CPagePool()
	{
		/* Delete all pages. */
		for(ke::LinkedList<CPage*>::iterator iter = m_pages.begin(); iter!=m_pages.end(); iter++)
		{
			delete *iter;
		}
	}
	void *Alloc(size_t minSize)
	{
		/* Try to get a memory region on a existing page first. */
		for(ke::LinkedList<CPage*>::iterator iter = m_pages.begin(); iter!=m_pages.end(); iter++)
		{
			CPage *pPage = *iter;
			void *pRegionMemory = pPage->AllocRegion(minSize);
			if(pRegionMemory)
			{
				return pRegionMemory;
			}
		}

		/* We've to allocate a new page. */
		CPage *pPage = new CPage(PageSize());
		m_pages.append(pPage);

		void *pRegionMemory = pPage->AllocRegion(minSize);
		assert(pRegionMemory);

		return pRegionMemory;
	}
	bool Free(void *memory)
	{
		for(ke::LinkedList<CPage*>::iterator iter = m_pages.begin(); iter!=m_pages.end(); iter++)
		{
			CPage *pPage = *iter;
			if(pPage->IsWithin(memory))
			{
				return pPage->FreeRegion(memory);
			}
		}
		return false;
	}
	size_t PageSize() const
	{
		return m_pageSize;
	}

#if defined(DEBUG)
	void DebugPrint()
	{
		for(ke::LinkedList<CPage*>::iterator iter = m_pages.begin(); iter!=m_pages.end(); iter++)
		{
			(*iter)->DebugPrint();
			printf("\n");
		}
	}
#endif
};

#endif //_include_pagepool_h_