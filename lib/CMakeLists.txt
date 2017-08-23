FILE(GLOB ALL_CPP_SRCS
        ${RakNet_SOURCE_DIR}/Source/*.cpp
        ${RakNet_SOURCE_DIR}/Source/Utils/*.cpp
        ${RakNet_SOURCE_DIR}/Source/Plugins/*.cpp
        ${RakNet_SOURCE_DIR}/include/raknet/*.h
        )

if(MSVC)
    add_definitions("/D WIN32 /D _CRT_NONSTDC_NO_DEPRECATE /D _CRT_SECURE_NO_DEPRECATE /GS- /GR-")
    set(CMAKE_CXX_FLAGS_DEBUG /D _DEBUG)
else(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG -D_DEBUG)
endif(MSVC)

if(RAKNET_ENABLE_LIBCAT_SECURITY)
    if(MSVC)
        add_definitions("/D LIBCAT_SECURITY")
    else(MSVC)
        add_definitions("-DLIBCAT_SECURITY")
    endif(MSVC)
    message(STATUS "Security enabled")
endif()

if(RAKNET_ENABLE_STATIC)
    add_library(RakNetLibStatic STATIC ${ALL_CPP_SRCS})

    target_link_libraries (RakNetLibStatic ${RAKNET_LIBRARY_LIBS})

    if(MSVC)
        set_target_properties(RakNetLibStatic PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB:\"LIBCD.lib LIBCMTD.lib MSVCRT.lib\"" )
    endif(MSVC)
endif()

if(RAKNET_ENABLE_DLL)
    if(MSVC)
        add_definitions("/D _RAKNET_DLL")
    else(MSVC)
        add_definitions("-D_RAKNET_DLL")
    endif(MSVC)

    add_library(${RAKNET_LIBRARY_SO_NAME} SHARED ${ALL_CPP_SRCS})
    target_link_libraries (${RAKNET_LIBRARY_SO_NAME} ${RAKNET_LIBRARY_LIBS})
    SET_TARGET_PROPERTIES(${RAKNET_LIBRARY_SO_NAME} PROPERTIES SOVERSION 4.081)
endif()
