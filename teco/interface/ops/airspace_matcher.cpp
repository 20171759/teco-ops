// BSD 3- Clause License Copyright (c) 2024, Tecorigin Co., Ltd. All rights
// reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
// OF SUCH DAMAGE.

#include "ual/ops/airspace_matcher/airspace_matcher.hpp"

#include "interface/common/convert.h"
#include "interface/common/macro.h"
#include "interface/include/builtin_type.h"
#include "interface/include/tecoops.h"
#include "ual/args/airspace_matcher_args.h"

using tecoops::ual::args::AirspaceMatcherArgs;
using tecoops::ual::args::AirspaceMatcherPatchArgs;
using tecoops::ual::ops::AirspaceMatcherOp;

static tecoopsStatus_t checkAirspaceMatcherInput(tecoopsHandle_t handle,
                                                  int gridSize,
                                                  const void *dmask,
                                                  const void *gridEnvCode,
                                                  const void *aircraftEnvCode,
                                                  void *outFlag) {
    if (handle == nullptr) {
        return TECOOPS_STATUS_NOT_INITIALIZED;
    }
    if (dmask == nullptr || gridEnvCode == nullptr ||
        aircraftEnvCode == nullptr || outFlag == nullptr) {
        return TECOOPS_STATUS_BAD_PARAM;
    }
    if (gridSize <= 0) {
        return TECOOPS_STATUS_BAD_PARAM;
    }
    return TECOOPS_STATUS_SUCCESS;
}

tecoopsStatus_t tecoopsAirspaceMatcher(tecoopsHandle_t handle, int gridSize, const void *dmask,
                                       const void *gridEnvCode, const void *aircraftEnvCode, void *outFlag,
                                       bool isDataCompression) {
    tecoopsStatus_t input_error = checkAirspaceMatcherInput(
        handle, gridSize, dmask, gridEnvCode, aircraftEnvCode, outFlag);
    if (input_error != TECOOPS_STATUS_SUCCESS)
        return input_error;

    AirspaceMatcherArgs arg;
    arg.spe_num = handle->spe_num;
    arg.gridSize = gridSize;
    arg.isDataCompression = isDataCompression;
    arg.dmask = dmask;
    arg.gridEnvCode = gridEnvCode;
    arg.aircraftEnvCode = aircraftEnvCode;
    arg.outFlag = outFlag;

    AirspaceMatcherPatchArgs patch_arg;
    patch_arg.atargs = &arg;
    if (isDataCompression) {
        patch_arg.data_type = tecoops::ual::common::UAL_DTYPE_INT16;
    } else {
        patch_arg.data_type = tecoops::ual::common::UAL_DTYPE_BOOL;
    }

    RUN_OP(AirspaceMatcherOp, arg, patch_arg, handle);
    return TECOOPS_STATUS_SUCCESS;
}
