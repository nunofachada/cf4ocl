#!/usr/bin/env bash

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
	# Install macos dependencies
	brew update
	brew install glib bats
else
	# Install Linux dependencies
	sudo apt-get -qq update
	sudo apt-get install -y -qq libglib2.0-dev #opencl-headers ocl-icd-libopencl1 build-essential cmake 
	wget http://mirrors.kernel.org/ubuntu/pool/universe/b/bats/bats_0.4.0-1ubuntu4_all.deb
	sudo dpkg -i bats_0.4.0-1ubuntu4_all.deb
	wget http://mirrors.kernel.org/ubuntu/pool/main/o/ocl-icd/ocl-icd-libopencl1_2.2.9-1_amd64.deb
	sudo dpkg -i ocl-icd-libopencl1_2.2.9-1_amd64.deb
	wget http://mirrors.kernel.org/ubuntu/pool/main/k/khronos-opencl-headers/opencl-headers_2.0~svn32091-2_all.deb
	sudo dpkg -i opencl-headers_2.0~svn32091-2_all.deb
	bash .travis/amd_sdk.sh
	tar -xjf AMD-SDK.tar.bz2
	export AMDAPPSDK=${HOME}/AMDAPPSDK
	sudo mkdir -p /etc/OpenCL/vendors
	mkdir -p ${AMDAPPSDK}
	sh AMD-APP-SDK*.sh --tar -xf -C ${AMDAPPSDK}
	echo ${AMDAPPSDK}/lib/x86_64/sdk/libamdocl64.so | sudo tee /etc/OpenCL/vendors/amdocl64.icd
	export LD_LIBRARY_PATH=${AMDAPPSDK}/lib/x86_64:${AMDAPPSDK}/lib/x86_64/sdk:${LD_LIBRARY_PATH}
	sudo cat /etc/OpenCL/vendors/amdocl64.icd
fi
