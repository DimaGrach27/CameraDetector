add_library(shared_protocol STATIC
    protocol/Packet.cpp
)

target_include_directories(shared_protocol
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(shared_protocol
    PUBLIC
        cxx_std_17
)