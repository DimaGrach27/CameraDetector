cmake_minimum_required(VERSION 3.10)
project(Camera_tracking)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED COMPONENTS
    core
    imgproc
    imgcodecs
    videoio
    highgui
    objdetect
    dnn
    tracking
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../shared ${CMAKE_BINARY_DIR}/shared_pi)

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
)

target_link_options(${PROJECT_NAME} PRIVATE
    "-Wl,-rpath-link,/Users/dhrachov/rpi-sysroot/usr/lib/aarch64-linux-gnu/lapack"
    "-Wl,-rpath-link,/Users/dhrachov/rpi-sysroot/usr/lib/aarch64-linux-gnu/blas"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${OpenCV_LIBS}

    hailort
    shared_protocol
    # armadillo
    # arpack

    # lapack
    # blas

    # /Users/dhrachov/rpi-sysroot/lib/libarmadillo.so.14
    # /Users/dhrachov/rpi-sysroot/usr/lib/aarch64-linux-gnu/libarpack.so.2
    # /Users/dhrachov/rpi-sysroot/usr/lib/aarch64-linux-gnu/liblapack.so.3
    # /Users/dhrachov/rpi-sysroot/usr/lib/aarch64-linux-gnu/libblas.so.3
)

set(PI_HOST "pi@192.168.0.107")
set(PI_APP_DIR "/home/pi/camera_tracking")

add_custom_target(deploy-rpi
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/deploy_stamp
    COMMAND rsync -avz
            $<TARGET_FILE:Camera_tracking>
            ${PI_HOST}:${PI_APP_DIR}/bin/
    COMMAND rsync -avz
            ${CMAKE_SOURCE_DIR}/resources/
            ${PI_HOST}:${PI_APP_DIR}/resources/
    DEPENDS Camera_tracking
    COMMENT "Deploy camera_tracking and resources to Raspberry Pi"
)