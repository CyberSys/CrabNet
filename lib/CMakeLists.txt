FILE(GLOB ALL_CPP_SRCS
        ${CrabNet_SOURCE_DIR}/Source/*.cpp
        ${CrabNet_SOURCE_DIR}/Source/Utils/*.cpp
        ${CrabNet_SOURCE_DIR}/Source/Plugins/*.cpp
        ${CrabNet_SOURCE_DIR}/include/crabnet/*.h
        )

if(MSVC)
    add_definitions("/D WIN32 /D _CRT_NONSTDC_NO_DEPRECATE /D _CRT_SECURE_NO_DEPRECATE /GS- /GR-")
    set(CMAKE_CXX_FLAGS_DEBUG /D _DEBUG)
else(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG -D_DEBUG)
endif(MSVC)

if(CRABNET_ENABLE_LIBCAT_SECURITY)
    if(MSVC)
        add_definitions("/D LIBCAT_SECURITY")
    else(MSVC)
        add_definitions("-DLIBCAT_SECURITY")
    endif(MSVC)
    message(STATUS "Security enabled")
endif()

if(CRABNET_ENABLE_STATIC)
    add_library(CrabNetLibStatic STATIC ${ALL_CPP_SRCS})

    target_link_libraries (CrabNetLibStatic ${CRABNET_LIBRARY_LIBS})

    if(MSVC)
        set_target_properties(CrabNetLibStatic PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB:\"LIBCD.lib LIBCMTD.lib MSVCRT.lib\"" )
    endif(MSVC)
endif()

if(CRABNET_ENABLE_DLL)
    if(MSVC)
        add_definitions("/D _CRABNET_DLL")
    else(MSVC)
        add_definitions("-D_CRABNET_DLL")
    endif(MSVC)

    add_library(${CRABNET_LIBRARY_SO_NAME} SHARED ${ALL_CPP_SRCS})
    target_link_libraries (${CRABNET_LIBRARY_SO_NAME} ${CRABNET_LIBRARY_LIBS})
    SET_TARGET_PROPERTIES(${CRABNET_LIBRARY_SO_NAME} PROPERTIES SOVERSION 4.081)
endif()

if(CRABNET_ENABLE_PVS)
pvs_studio_add_target(TARGET crabnet.analyze ALL
        OUTPUT FORMAT errorfile
        ANALYZE CrabNetLibStatic
        LOG target.err)
endif()