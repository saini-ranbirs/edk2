export WORKSPACE=`pwd`
#export GCC5_RISCV64_PREFIX=riscv64-unknown-linux-gnu-
export PACKAGES_PATH=$WORKSPACE
export EDK_TOOLS_PATH=$WORKSPACE/BaseTools
source edksetup.sh --reconfig
source edksetup.sh BaseTools
build -a X64 --buildtarget DEBUG -DNETWORK_ENABLE=FALSE -DSECURE_BOOT_ENABLE=TRUE -p MdeModulePkg/MdeModulePkg.dsc -t GCC5
