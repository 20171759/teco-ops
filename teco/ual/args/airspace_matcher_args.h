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

#ifndef TECO_UAL_ARGS_AIRSPACE_MATCHER_ARGS_H_
#define TECO_UAL_ARGS_AIRSPACE_MATCHER_ARGS_H_

#include <cstdint>
#include "ual/com/def.h"

namespace tecoops {
namespace ual {
namespace args {

// airspace_matcher: check whether airspace grid environment matches aircraft specs
//
// For each grid i:
//   Flag[i] = (aircraftEnvCode - gridEnvCode[i]) & DMASK
//   Flag[i] == 0  => environment matches (output 0)
//   Flag[i] != 0  => environment does not match (output 1)
struct AirspaceMatcherArgs {
    int spe_num;
    int gridSize;
    bool isDataCompression;

    const void *dmask;             // pointer to DMASK value on device
    const void *gridEnvCode;       // [gridSize] grid environment codes
    const void *aircraftEnvCode;   // pointer to single aircraft env code
    void *outFlag;                 // [gridSize] output match flags (bool array)
};

struct AirspaceMatcherPatchArgs {
    AirspaceMatcherArgs *atargs;
    common::UALDataType data_type;
};

}  // namespace args
}  // namespace ual
}  // namespace tecoops

#endif  // TECO_UAL_ARGS_AIRSPACE_MATCHER_ARGS_H_
