/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/bio/trim_io.h"
#include "src/volume/volume_list.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/include/memory.h"
#include "src/logger/logger.h"

namespace pos
{
TrimIo::TrimIo(int arrayId_, uint64_t rba_, uint64_t size_)
: volumeId(MAX_VOLUME_COUNT),
  arrayId(arrayId_),
  sectorRba(rba_),
  sectorSize(size_),
  originCore(EventFrameworkApiSingleton::Instance()->GetCurrentReactor())
{
    POS_TRACE_ERROR(9999, "Create TRIM Io  : {} size : {}", sectorRba, sectorSize);
}

TrimIo::~TrimIo(void)
{
}

uint32_t
TrimIo::GetVolumeId(void)
{
    return volumeId;
}

void
TrimIo::SetVolumeId(uint32_t volumeId)
{
    this->volumeId = volumeId;
}

int
TrimIo::GetArrayId(void)
{
    return arrayId;
}

uint64_t
TrimIo::GetSectorRba(void)
{
    return sectorRba;
}

uint64_t
TrimIo::GetSectorSize(void)
{
    return sectorSize;
}

uint32_t
TrimIo::GetOriginCore(void)
{
    return originCore;
}

void
TrimIo::SetCallback(CallbackSmartPtr inputCallback)
{
    callback = inputCallback;
}

} // namespace pos
