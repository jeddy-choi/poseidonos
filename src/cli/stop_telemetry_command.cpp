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

#include "src/cli/stop_telemetry_command.h"

#include <vector>

#include "src/array_mgmt/array_manager.h"
#include "src/cli/cli_event_code.h"
#include "src/cli/request_handler.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "src/telemetry/telemetry_config/telemetry_config.h"

namespace pos_cli
{
StopTelemetryCommand::StopTelemetryCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
StopTelemetryCommand::~StopTelemetryCommand(void)
{
}
// LCOV_EXCL_STOP

string
StopTelemetryCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    JsonElement data("data");

    TelemetryClient* tc = TelemetryClientSingleton::Instance();
    bool result = tc->StopAllPublisher();
    if (!result)
    {
        return jFormat.MakeResponse("STOPTELEMETRY", rid, result,
            "Telemetry could not stop ", data, GetPosInfo());
    }

    TelemetryConfig* config = TelemetryConfigSingleton::Instance();
    if (!config->GetClient().UpdateConfig(TelemetryConfigType::Client, "enabled", false, true))
    {
        return jFormat.MakeResponse("STOPTELEMETRY", rid, result,
            "The config for telemetry client could not be set as false", data, GetPosInfo());
    }

    return jFormat.MakeResponse("STOPTELEMETRY", rid, SUCCESS,
        "Telemetry has stopped ", data, GetPosInfo());
}
}; // namespace pos_cli
