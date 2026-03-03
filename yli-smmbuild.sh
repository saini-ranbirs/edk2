echo "script parameter 0 - complete clean build 1 - continued build"
echo "./yli-smmbuild.sh 1"

# 0. https://lf-rise.atlassian.net/wiki/spaces/HOME/pages/8588609/EDK2_00_02_04+Evaluate+with+OpenSBI
# 0. https://github.com/yli147/edk2/tree/dev-standalonemm-mpxy-v3

git checkout yli147-dev-standalonemm-mpxy-v3-rs_debug

if [ $1 -eq 0 ]; then
	./smm-build-edk2-riscv-0.sh
fi

git apply -R 0001-ENABLE_RS_SVA-for-smm-2-build-and-disable-it-for-smm.patch
./smm-build-edk2-riscv-1.sh
git apply 0001-ENABLE_RS_SVA-for-smm-2-build-and-disable-it-for-smm.patch
./smm-build-edk2-riscv-2.sh

