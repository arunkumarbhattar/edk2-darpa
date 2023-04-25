# HARDEN Sample Drivers

A collection of vulnerable drivers created for the HARDEN program.

## Building the Drivers

### Preparing EDK2

To build the drivers, start by preparing EDK2 and building with the following commands:

```bash
git clone git@github.com:BreakingBoot/edk2-vulnerable.git
git submodule update --init 
make -C BaseTools
source edksetup.sh 
build -p OvmfPkg/OvmfPkgX64.dsc -a X64 -t GCC5 -D SMM_REQUIRE
```

### Building with Nix

If you haven't installed Nix yet, follow these steps:

1. Open a terminal window and run the following command to install Nix:

```bash
curl --proto '=https' --tlsv1.2 -sSf -L https://install.determinate.systems/nix | sh -s -- install
```

2. After the installation is complete, close your terminal and open a new one to ensure Nix is properly set up.

3. Run the following command:

```bash
nix build ".?submodules=1#bitcodeEDK2" -L
```

4. The compiled build and bitcode can be found in the `result/` directory.

## Running the Firmware

### Copying the Firmware and Exploit UEFI Application

Copy the OVMF firmware and Exploit UEFI Application to `/tmp/rootfs`:

```bash
cp ./Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd /tmp/
cp ./Build/OvmfX64/DEBUG_GCC5/X64/Exploit.efi /tmp/rootfs
```

**NB: You might have a different `DEBUG_` directory whether you built the project with Nix.**

### Running OVMF in QEMU

Execute the following command to run OVMF in QEMU:

```bash
qemu-system-x86_64 \
    -m 1024    \
    -no-reboot \
    -machine q35,smm=on \
    -cpu max \
    -pflash /tmp/OVMF.fd \
    -global ICH9-LPC.disable_s3=1 \
    -global driver=cfi.pflash01,property=secure,value=on \
    -global isa-debugcon.iobase=0x402 -debugcon file:/tmp/debug.log \
    -drive format=raw,file=fat:rw:/tmp/rootfs/ \
    -net none
```

### Running the Exploit from the UEFI Shell

To run the exploit from the UEFI Shell, enter the following command:

```
FS0:\Exploit.efi
```
