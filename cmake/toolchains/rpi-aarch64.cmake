set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(RPI_SYSROOT /Users/dhrachov/rpi-sysroot)

set(CMAKE_SYSROOT ${RPI_SYSROOT})
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

set(CMAKE_C_COMPILER /opt/homebrew/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /opt/homebrew/bin/aarch64-linux-gnu-g++)

set(COMMON_SYSROOT_FLAGS
    "--sysroot=${RPI_SYSROOT} \
     -I${RPI_SYSROOT}/usr/include/aarch64-linux-gnu \
     -B${RPI_SYSROOT}/usr/lib/aarch64-linux-gnu \
     -B${RPI_SYSROOT}/lib/aarch64-linux-gnu")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu")
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "--sysroot=${RPI_SYSROOT} \
    -B${RPI_SYSROOT}/usr/lib/aarch64-linux-gnu/crt1.o \
    -B${RPI_SYSROOT}/usr/lib/aarch64-linux-gnu/crti.o \
    -L${RPI_SYSROOT}/usr/lib/aarch64-linux-gnu \
    -L${RPI_SYSROOT}/lib/aarch64-linux-gnu \
    -Wl,-rpath-link,${RPI_SYSROOT}/usr/lib/aarch64-linux-gnu \
    -Wl,-rpath-link,${RPI_SYSROOT}/lib/aarch64-linux-gnu \
    -Wl,-rpath-link,${RPI_SYSROOT}/usr/lib/aarch64-linux-gnu/lapack \
    -Wl,-rpath-link,${RPI_SYSROOT}/usr/lib/aarch64-linux-gnu/blas")


set(CMAKE_C_FLAGS_INIT "${COMMON_SYSROOT_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${COMMON_SYSROOT_FLAGS}")

set(CMAKE_EXE_LINKER_FLAGS_INIT "${COMMON_LINK_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${COMMON_LINK_FLAGS}")

set(CMAKE_FIND_ROOT_PATH ${RPI_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)