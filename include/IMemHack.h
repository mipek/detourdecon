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
#ifndef _include_imemhack_h_
#define _include_imemhack_h_

/**
 * Called for each pattern in a scan.
 *
 * @brief    pattern        Pattern index.
 * @brief    ptr            Pointer to pattern found in memory. NULL if not found.
 * @brief    patched        True when the memory the pattern matched against was patched.
 */
typedef void (* OnPatternMatch_t)(int pattern, void *ptr, bool patched);

/**
 * Memory hacking interface that takes applied patches into account.
 */
class IMemHack
{
public:
    /**
     * Checks whether or not the specified memory range was patched.
     *
     * @param    addr        Base address in memory.
     * @param    len            Length in bytes.
     * @return                True when there was a patch in the specified region.
     */
    virtual bool IsPatched(const void *addr, size_t len) =0;

    /**
     * Patches memory.
     *
     * @param    addr        Base address in memory.
     * @param    bytes        Patch data.
     * @param    len            Length of the patch.
     */
    virtual void Patch(const void *addr, const void *bytes, size_t len) =0;

    /**
     * Adds a new pattern to the current batch.
     *
     * @brief    pattern        Memory pattern.
     * @brief    len            Length of the pattern.
     * @brief    cb            Callback that is scheduled to be called after Scan() completed.
     * @return                Pattern index.
     */
    virtual int AddPattern(const char *pattern, size_t len, OnPatternMatch_t cb) =0;

    /**
     * Scans for all added patterns in the specified module.
     *
     * @brief    module        Base address of a module.
     * @brief    len            Length of the module(in bytes).
     */
    virtual void Scan(const void *module, size_t len) =0;
};

#endif //_include_imemhack_h_