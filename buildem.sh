#!/bin/bash

TCWD=`pwd`

mkdir Conf

source edksetup.sh BaseTools

make -C BaseTools

cd $TCWD
cd OvmfPkg
./build.sh
cd $TCWD
cd EmulatorPkg
./build.sh
cd $TCWD
