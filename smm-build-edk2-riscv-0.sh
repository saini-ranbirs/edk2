sup
export WORKDIR=../smm-test
export GCC5_RISCV64_PREFIX=riscv64-unknown-linux-gnu-
. edksetup.sh
make -C BaseTools clean
make -C BaseTools
make -C BaseTools/Source/C
build -a RISCV64 -t GCC5 -p OvmfPkg/RiscVVirt/RiscVVirtQemu.dsc -b DEBUG -DSECURE_BOOT_ENABLE=TRUE cleanall
#build -a RISCV64 -t GCC5 -p OvmfPkg/RiscVVirt/RiscVVirtQemu.dsc -b DEBUG -DSECURE_BOOT_ENABLE=TRUE
build -a RISCV64 -t GCC5 -p OvmfPkg/RiscVVirt/RiscVVirtQemuStandaloneMm.dsc -b DEBUG -D FW_BASE_ADDRESS=0x80C00000 cleanall
#build -a RISCV64 -t GCC5 -p OvmfPkg/RiscVVirt/RiscVVirtQemuStandaloneMm.dsc -b DEBUG -D FW_BASE_ADDRESS=0x80C00000
#cp Build/RiscVVirtQemu/DEBUG_GCC5/FV/RISCV_VIRT_CODE.fd $WORKDIR
#cp Build/RiscVVirtQemu/DEBUG_GCC5/FV/RISCV_VIRT_VARS.fd $WORKDIR
#cp Build/RiscVVirtQemuStandaloneMm/DEBUG_GCC5/FV/STANDALONE_MM.fd $WORKDIR
