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

#include <stdio.h>
#include <cstring>
#include <iostream>
#include <string>
#include "zoo/teco/convert.h"
#include "common/time.hpp"
#include "zoo/teco/airspace_matcher/airspace_matcher.h"
#include "interface/include/tecoops.h"

namespace optest {

void AirspaceMatcherExecutor::paramCheck() {
    int nin = parser_->inputs().size();
    int nout = parser_->outputs().size();

    // 4 inputs: dmask, gridEnvCode, aircraftEnvCode, (unused placeholder)
    // 1 output: outFlag
    if (nin != 3 || nout != 1) {
        ALLOG(ERROR) << "airspace_matcher requires 3 inputs and 1 output, got "
                     << nin << " in, " << nout << " out.";
        throw std::invalid_argument(std::string(__FILE__) + ":" + std::to_string(__LINE__));
    }
}

void AirspaceMatcherExecutor::paramParse() {
    // input[0]: dmask [1]
    // input[1]: gridEnvCode [gridSize]
    // input[2]: aircraftEnvCode [1]
    auto meta_grid = parser_->input(1);
    gridSize_ = meta_grid->shape[0];

    auto matcher_param = parser_->getProtoNode()->tecokernel_param().airspace_matcher_param();
    isDataCompression_ = matcher_param.is_data_compression();
}

void AirspaceMatcherExecutor::paramGeneration() {
    dmask_ = dev_input[0];
    gridEnvCode_ = dev_input[1];
    aircraftEnvCode_ = dev_input[2];
    outFlag_ = dev_output[0];
}

void AirspaceMatcherExecutor::compute() {
    checkTECOOPS(tecoopsAirspaceMatcher(
        handle_,
        gridSize_,
        dmask_,
        gridEnvCode_,
        aircraftEnvCode_,
        outFlag_,
        isDataCompression_));
}

void AirspaceMatcherExecutor::cpuCompute() {
    pythonComputeCPU("cpu");
}

int64_t AirspaceMatcherExecutor::getTheoryOps() {
    // Each grid: 1 subtraction + 1 AND + 1 comparison = 3 ops
    int64_t ops = static_cast<int64_t>(gridSize_) * 3;
    return ops;
}

int64_t AirspaceMatcherExecutor::getTheoryIoSize() {
    constexpr int kUint64Size = 8;
    constexpr int kInt32Size = 4;
    constexpr int kInt16Size = 2;
    int64_t size = 0;
    if (!isDataCompression_) {
        // read: dmask (1 uint64)
        size += 1 * kUint64Size;
        // read: gridEnvCode (gridSize uint64)
        size += static_cast<int64_t>(gridSize_) * kUint64Size;
        // read: aircraftEnvCode (1 uint64)
        size += 1 * kUint64Size;
        // write: outFlag (gridSize bool, 1 byte each)
        size += static_cast<int64_t>(gridSize_) * 1;
    } else {
        // read: dmask (1 int32)
        size += 1 * kInt32Size;
        // read: gridEnvCode (gridSize int32)
        size += static_cast<int64_t>(gridSize_) * kInt32Size;
        // read: aircraftEnvCode (1 int32)
        size += 1 * kInt32Size;
        // write: outFlag (gridSize int16, 2 bytes each)
        size += static_cast<int64_t>(gridSize_) * kInt16Size;
    }
    return size;
}

}  // namespace optest
