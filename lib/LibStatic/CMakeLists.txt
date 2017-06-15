project(RakNetLibStatic)

add_library(RakNetLibStatic STATIC ${ALL_CPP_SRCS})

IF(MSVC)
    add_definitions("/D _RAKNET_LIB")
ENDIF(MSVC)

target_link_libraries (RakNetLibStatic ${RAKNET_LIBRARY_LIBS})

IF(MSVC)
    IF( MSVC10 OR MSVC11 OR MSVC12 OR MSVC14 )
        set_target_properties(RakNetLibStatic PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB:\"LIBCD.lib LIBCMTD.lib MSVCRT.lib\"" )
    ELSE()
        set_target_properties(RakNetLibStatic PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB:&quot;LIBCD.lib LIBCMTD.lib MSVCRT.lib&quot;" )
    ENDIF()
ENDIF(MSVC)


