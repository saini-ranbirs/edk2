export GCC5_RISCV64_PREFIX=riscv64-unknown-linux-gnu-
. edksetup.sh
build -a RISCV64 -t GCC5 -p OvmfPkg/RiscVVirt/RiscVVirtQemu.dsc -b DEBUG -DSECURE_BOOT_ENABLE=TRUE
