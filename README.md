# Demo1

The purpose behind this challenge is finding flaws in drivers in EDK2. To that end it's important to understand that EDK2 on most systems is not attackable from the UEFI Shell. However, we are using the UEFI Shell to mimic where an attack would come from; that is the kernel or userspace. The idea is that a UEFI app can interact with any driver with an exposed protocol or specific functionality that would be exposed to the kernel from the system table. It is important to remember that the memory in EDK2 is a flat memory model and any access can manipulate anything. However this isn't how we want to play the game, because it isn't an accurate representation of the system is used. </br>

Therefore, for rules in our game you may use any driver's protocols that are exposed and the SetAccessVariable/GetAccessVariable/CalculateCrc32 functions from the system table. And, as with any good rules you can break them - if you can show that there are other functions in the system table that would normally be exposed to the kernel or userspace, then you can use those as well.

# QuickStart

* Ensure all packages are installed:</br>

`$ sudo apt-get install git gcc g++ make uuid-dev python-is-python3 build-essential nasm iasl libx11-dev libxv-dev gdb`

## Quick Note
If you follow the instructions for building the packages and are getting a `Permission Denied` the likely problem is that the files are not set to be executable. Run the following to fix the issues: </br>
```chmod +x <somepath>/edk2/EmulatorPkg/build.sh
chmod +x <somepath>/edk2/BaseTools/BinWrappers/PosixLike/TianoCompress
chmod +x <somepath>/edk2/BaseTools/BinWrappers/PosixLike/build
chmod +x <somepath>/edk2/BaseTools/BinWrappers/PosixLike/Trim
chmod +x <somepath>/edk2/BaseTools/BinWrappers/PosixLike/VfrCompile
chmod +x <somepath>/edk2/BaseTools/BinWrappers/PosixLike/GenFw
chmod +x <somepath>/edk2/BaseTools/BinWrappers/PosixLike/GenSec
chmod +x <somepath>/edk2/BaseTools/BinWrappers/PosixLike/GenFfs
chmod +x <somepath>/edk2/BaseTools/BinWrappers/PosixLike/GenFv
```


## EmulatorPkg Build/Run
* From top directory:
```bash 
$ cd edk2
$ mkdir Conf
$ source edksetup.sh BaseTools
$ make -C BaseTools
$ EmulatorPkg/build.sh
$ EmulatorPkg/build.sh run
```
### Using the Example
Alice and Bob drivers need to be disabled from autoloading.
1. Edit EmulatorPkg.fdf and comment the Alice and Bob lines
```
# INF  EmulatorPkg/Demo1_Alice/Demo1_Alice.inf
# INF  EmulatorPkg/Demo1_Bob/Demo1_Bob.inf
```
2. Redo build/run
```bash
$ EmulatorPkg/build.sh
$ EmulatorPkg/build.sh run
```
3. Run the app
* In the uefi shell:
```bash
Shell> fs0:Demo1_Example_App.efi
```


## OvmfPkg Build/Run
* From top directory:
```bash 
$ cd edk2
$ mkdir Conf
$ source edksetup.sh BaseTools
$ make -C BaseTools
$ OvmfPkg/build.sh
$ cd ../
$ mkdir run-ovmf
$ cd run-ovmf
$ mkdir hda-contents
$ cp ../edk2/Build/OvmfX64/DEBUG_GCC5/FV/OVMF* .
$ cp ../edk2/Build/OvmfX64/DEBUG_GCC5/X64/*.efi hda-contents/
$ cp ../edk2/Build/OvmfX64/DEBUG_GCC5/X64/Demo1* hda-contents/
$ cp OVMF.fd bios.bin
$ qemu-system-x86_64  -pflash bios.bin -hda fat:rw:hda-contents -net none -debugcon file:debug.log -global isa-debugcon.iobase=0x402 -s -S
```

* In a second tab:
```bash
$ gdb
(gdb) target remote :1234
(gdb) c
```

### Using the Example
Alice and Bob drivers need to be disabled from autoloading.
1. Edit OvmfPkgX64.fdf and comment the Alice and Bob lines
```
# INF  EmulatorPkg/Demo1_Alice/Demo1_Alice.inf
# INF  EmulatorPkg/Demo1_Bob/Demo1_Bob.inf
```
2. Redo build/run
```bash
$ OvmfPkg/build.sh
$ cd ../run-ovmf
 ** copy files
$ qemu-system-x86_64  -pflash bios.bin -hda fat:rw:hda-contents -net none -debugcon file:debug.log -global isa-debugcon.iobase=0x402 -s -S
```
3. Run the app w/gdb debug tab if req'd
* In the uefi shell:
```bash
Shell> fs0:Demo1_Example_App.efi
