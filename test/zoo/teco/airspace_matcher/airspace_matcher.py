# BSD 3-Clause License
#
# Copyright (c) 2024, Tecorigin Co., Ltd.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# encoding:utf-8


import os
import sys
import json
import numpy as np
import torch
sys.path.append("../zoo/teco/")
sys.path.append("../")
from executor import *


def _airspace_matcher_ref(dmask, gridEnvCode, aircraftEnvCode):
    """CPU reference for airspace_matcher

    For each grid i:
        Flag[i] = (aircraftEnvCode - gridEnvCode[i]) & DMASK
        outFlag[i] = 0 if Flag == 0 (match), else 1 (no match)
    """
    aircraftEnvCode_u64 = aircraftEnvCode.view(np.uint64)
    gridEnvCode_u64 = gridEnvCode.view(np.uint64)
    dmask_u64 = dmask.view(np.uint64)
    diff = aircraftEnvCode_u64 - gridEnvCode_u64

    flag = diff & dmask_u64
    outFlag = (flag != 0)
    return outFlag.astype(bool)


def _airspace_matcher_ref_int32(dmask, gridEnvCode, aircraftEnvCode):
    """CPU reference for airspace_matcher (int32 data compression mode)

    For each grid i:
        Flag[i] = (aircraftEnvCode - gridEnvCode[i]) & DMASK
        outFlag[i] = 0 if Flag == 0 (match), else 1 (no match)
    """
    aircraftEnvCode_i32 = aircraftEnvCode.view(np.int32)
    gridEnvCode_i32 = gridEnvCode.view(np.int32)
    dmask_i32 = dmask.view(np.int32)
    diff = aircraftEnvCode_i32 - gridEnvCode_i32

    flag = diff & dmask_i32
    outFlag = (flag != 0).astype(np.int16)
    return outFlag


def check_inputs(param_path, input_lists, reuse_lists, output_lists):
    if param_path == "":
        print("The path of prototxt file is empty.")
        return False
    if len(input_lists) != 3:
        print("The number of input data is wrong (expected 3: dmask, gridEnvCode, aircraftEnvCode).")
        return False
    if len(reuse_lists) != 0:
        print("The number of reuse data is wrong.")
        return False
    if len(output_lists) != 1:
        print("The number of output data is wrong (expected 1: outFlag).")
        return False
    return True


def test_airspace_matcher(param_path, input_lists, reuse_lists, output_lists, device):
    if not check_inputs(param_path, input_lists, reuse_lists, output_lists):
        return
    if device == "cuda":
        is_avail, used_device = is_device_available(device)
        if not is_avail:
            return

    params = read_prototxt(param_path)
    input_params = params["input"]
    output_params = params["output"]

    # Read is_data_compression from tecokernel_param
    kernel_param = params.get("tecokernel_param", {})
    matcher_param = kernel_param.get("airspace_matcher_param", {})
    is_data_compression = bool(matcher_param.get("is_data_compression", False))

    # Read input tensors
    dmask = to_tensor(input_lists[0], input_params[0], device=device)       # [1]
    gridEnvCode = to_tensor(input_lists[1], input_params[1], device=device) # [gridSize]
    aircraftEnvCode = to_tensor(input_lists[2], input_params[2], device=device)  # [1]

    if is_data_compression:
        # int32 inputs, int16 output
        dmask_np = np.int32(dmask.cpu().numpy()) if hasattr(dmask, 'cpu') else np.int32(dmask)
        grid_np = np.int32(gridEnvCode.cpu().numpy()) if hasattr(gridEnvCode, 'cpu') else np.int32(gridEnvCode)
        aircraft_np = np.int32(aircraftEnvCode.cpu().numpy()) if hasattr(aircraftEnvCode, 'cpu') else np.int32(aircraftEnvCode)

        dmask_val = dmask_np.flatten()[0]
        aircraft_val = aircraft_np.flatten()[0]

        outFlag = torch.from_numpy(_airspace_matcher_ref_int32(dmask_val, grid_np, aircraft_val))
    else:
        # uint64 inputs, bool output
        dmask_np = np.int64(dmask.cpu().numpy()) if hasattr(dmask, 'cpu') else np.int64(dmask)
        grid_np = np.int64(gridEnvCode.cpu().numpy()) if hasattr(gridEnvCode, 'cpu') else np.int64(gridEnvCode)
        aircraft_np = np.int64(aircraftEnvCode.cpu().numpy()) if hasattr(aircraftEnvCode, 'cpu') else np.int64(aircraftEnvCode)

        dmask_val = dmask_np.flatten()[0]
        aircraft_val = aircraft_np.flatten()[0]

        outFlag = torch.from_numpy(_airspace_matcher_ref(dmask_val, grid_np, aircraft_val))

    # Save output
    with open(output_lists[0], "wb") as f:
        save_tensor(f, outFlag, output_params["dtype"])


def parse_params(filename):
    with open(filename, "r") as f:
        params = json.load(f)
    return params


if __name__ == "__main__":
    params = parse_params(sys.argv[1])
    device = sys.argv[2]
    test_airspace_matcher(params["param_path"], params["input_lists"],
                          params["reuse_lists"], params["output_lists"], device)
