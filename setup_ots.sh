. setup.sh
source /data/ups/setup
setup gcc v4_9_3a
setup root v6_08_04c -q e10:prof
source ${ROOTSYS}/bin/thisroot.sh
setup cmake v3_7_1
export CC=${GCC_FQ_DIR}/bin/gcc
export CXX=${GCC_FQ_DIR}/bin/g++ 
