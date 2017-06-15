project(RakNetLibStatic)

FILE(GLOB ALL_CPP_SRCS ${RakNet_SOURCE_DIR}/Source/*.cpp)

add_library(RakNetLibStatic STATIC ${ALL_CPP_SRCS})

IF(MSVC)
	SET( CMAKE_CXX_FLAGS "/D WIN32 /D _RAKNET_LIB /D _CRT_NONSTDC_NO_DEPRECATE /D _CRT_SECURE_NO_DEPRECATE /GS- /GR- ")
ENDIF(MSVC)

target_link_libraries (RakNetLibStatic ${RAKNET_LIBRARY_LIBS})

IF(MSVC)
	IF( MSVC10 OR MSVC11 OR MSVC12 OR MSVC14 )
		set_target_properties(RakNetLibStatic PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB:\"LIBCD.lib LIBCMTD.lib MSVCRT.lib\"" )
	ELSE()
		set_target_properties(RakNetLibStatic PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB:&quot;LIBCD.lib LIBCMTD.lib MSVCRT.lib&quot;" )
	ENDIF()		
ELSE(MSVC)
	INSTALL(TARGETS RakNetLibStatic DESTINATION ${RakNet_SOURCE_DIR}/Lib/RakNetLibStatic)
ENDIF(MSVC)


