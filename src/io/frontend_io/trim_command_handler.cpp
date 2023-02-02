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

#include "src/io/frontend_io/trim_command_handler.h"

#include "src/event_scheduler/io_completer.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/spdk_wrapper/event_framework_api.h"

#include "src/meta_service/meta_service.h"

namespace pos
{
TrimCmdHandler::TrimCmdHandler(TrimIoSmartPtr trimIo)
:  trimIo(trimIo), 
   blockAlignment(ChangeSectorToByte(trimIo->GetSectorRba()), trimIo->GetSectorSize())
   
{

    POS_TRACE_ERROR(9999, "blockAlignment {} {}", ChangeSectorToByte(trimIo->GetSectorRba()), trimIo->GetSectorSize());
    blockCount = blockAlignment.GetBlockCount();
    startBlockAddress = blockAlignment.GetHeadBlock();

    /* remove head */
     if (blockAlignment.HasHead())
    {
        startBlockAddress++;
        blockCount--;
    }

    /* remove tail */
    if (blockAlignment.HasTail())
    {
        blockCount--;
    }

    translator = new Translator{trimIo->GetVolumeId(), startBlockAddress, blockCount, trimIo->GetArrayId(), true};

    POS_TRACE_ERROR(9999, "translator {} {} {} {}", trimIo->GetVolumeId(), startBlockAddress, ChangeSectorToByte(blockCount), trimIo->GetArrayId());

    POS_TRACE_ERROR(9999, "Requested trim strat rba : {} size : {}", trimIo->GetSectorRba(), trimIo->GetSectorSize());

}

TrimCmdHandler::~TrimCmdHandler(void)
{
}

bool
TrimCmdHandler::Execute(void)
{
    /* loop for trim*/

    for (uint32_t blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        //PhysicalBlkAddr pba = translator->GetPba(blockIndex);
        VirtualBlkAddr vsa = translator->GetVsaOverSize(blockIndex);
        //uint32_t dataSize = 4096;

        if (false == IsUnMapVsa(vsa))
        {
            VirtualBlks invalidVsaRange = {vsa, 1};
            bool allowVictimSegRelease = true;
            //MetaUpdateCallback::InvalidateBlks(invalidVsaRange, allowVictimSegRelease);

            POS_TRACE_ERROR(9999, "------------Requested Invalidate block idx {}", blockIndex);

            MetaServiceSingleton::Instance()->GetsegmentContextUpdater()->InvalidateBlocksWithGroupId(invalidVsaRange, allowVictimSegRelease, UINT32_MAX);            
        }        
    }

    POS_TRACE_ERROR(9999, "block cnt : {}", blockCount);
    return true;
}


} // namespace pos
