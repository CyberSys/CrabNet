if( RAKNET_ENABLE_STATIC )
	add_subdirectory(LibStatic)
endif()

if( RAKNET_ENABLE_DLL )
	add_subdirectory(DLL)
endif()
