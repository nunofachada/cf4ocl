#!/usr/bin/env bash

# Install Linux dependencies
bash .travis/amd_sdk.sh
tar -xjf AMD-SDK.tar.bz2
export AMDAPPSDK=${HOME}/AMDAPPSDK
sudo mkdir -p /etc/OpenCL/vendors
mkdir -p ${AMDAPPSDK}
sh AMD-APP-SDK*.sh --tar -xf -C ${AMDAPPSDK}
echo ${AMDAPPSDK}/lib/x86_64/sdk/libamdocl64.so | sudo tee /etc/OpenCL/vendors/amdocl64.icd
export LD_LIBRARY_PATH=${AMDAPPSDK}/lib/x86_64:${AMDAPPSDK}/lib/x86_64/sdk:${LD_LIBRARY_PATH}
