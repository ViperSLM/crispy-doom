set(VSLM_64BIT OFF)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(VSLM_64BIT ON)
endif()

add_library(vslm STATIC
    vslm.h
	vslm_rng.c
)
target_include_directories(vslm PRIVATE "${CMAKE_SOURCE_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}/../..")

file(GLOB VSLMDoom_H ${CMAKE_SOURCE_DIR}/src/doom/vslm_doom*.h)
file(GLOB VSLMDoom_SRC ${CMAKE_SOURCE_DIR}/src/vslm/vslm_doom_*.c)
add_library(vslm-doom STATIC
	vslm.h ${VSLMCommon_H}
	${VSLMDoom_H}
	${VSLMDoom_SRC}
)
target_include_directories(vslm-doom PRIVATE "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/src/doom" "${CMAKE_CURRENT_BINARY_DIR}/../..")

if(VSLM_64BIT)
    target_compile_definitions(vslm PRIVATE -DVSLM_64BIT)
	target_compile_definitions(vslm-doom PRIVATE -DVSLM_64BIT)
endif()