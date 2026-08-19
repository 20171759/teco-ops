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

#ifndef TECOOPS_UAL_OPS_AIRSPACE_MATCHER_AIRSPACE_MATCHER_HPP_
#define TECOOPS_UAL_OPS_AIRSPACE_MATCHER_AIRSPACE_MATCHER_HPP_

#include "ual/ops/base_op.hpp"
#include "ual/com/log.h"
#include "ual/ops/airspace_matcher/find_airspace_matcher.h"
#include "ual/kernel/airspace_matcher/airspace_matcher.h"

namespace tecoops {
namespace ual {
namespace ops {

using tecoops::ual::args::AirspaceMatcherArgs;
using tecoops::ual::args::AirspaceMatcherPatchArgs;
using tecoops::ual::common::Status;

struct AirspaceMatcherType {
    using ArgsType = AirspaceMatcherArgs;
    using PatchType = AirspaceMatcherPatchArgs;
    using RetType = void;
    using PImplType = void (*)(ArgsType);
};

static AirspaceMatcherType::PImplType AirspaceMatcherAlgos[] = {
    teco_slave_airspace_matcher,  // index 0: uint64 bitwise match
    teco_slave_airspace_matcher_int32,  // index 1: int32 bitwise match (data compression)
};

static const char *AirspaceMatcherDiscription[] = {
    "teco_slave_airspace_matcher",
    "teco_slave_airspace_matcher_int32",
};

struct AirspaceMatcherOp : public BaseOp<AirspaceMatcherOp, AirspaceMatcherType> {
 public:
    using ArgsType = typename AirspaceMatcherType::ArgsType;
    using PatchType = typename AirspaceMatcherType::PatchType;
    using RetType = typename AirspaceMatcherType::RetType;
    using PImplType = typename AirspaceMatcherType::PImplType;

    AirspaceMatcherOp() = default;
    ~AirspaceMatcherOp() = default;

    static const char *name() { return "airspace_matcher"; }

    Status findImpl(const PatchType *args) {
        int index = findAirspaceMatcherBranch(args);
        if (index == -1) {
            ERROR("airspace_matcher branch is not exit!");
            return Status::NOT_IMPLEMENTED;
        }
        setInstance(AirspaceMatcherAlgos[index], AirspaceMatcherDiscription[index]);
        return Status::SUCCESS;
    }
};

}  // namespace ops
}  // namespace ual
}  // namespace tecoops

#endif  // TECOOPS_UAL_OPS_AIRSPACE_MATCHER_AIRSPACE_MATCHER_HPP_
