export WORKDIR=../smm-test
export GCC5_RISCV64_PREFIX=riscv64-unknown-linux-gnu-
. edksetup.sh
build -a RISCV64 -t GCC5 -p OvmfPkg/RiscVVirt/RiscVVirtQemuStandaloneMm.dsc -b DEBUG -D FW_BASE_ADDRESS=0x80C00000
cp -av Build/RiscVVirtQemu/DEBUG_GCC5/FV/RISCV_VIRT_CODE.fd $WORKDIR
cp -av Build/RiscVVirtQemu/DEBUG_GCC5/FV/RISCV_VIRT_VARS.fd $WORKDIR
cp -av Build/RiscVVirtQemuStandaloneMm/DEBUG_GCC5/FV/STANDALONE_MM.fd $WORKDIR
