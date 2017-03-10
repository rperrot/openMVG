set( DOM_SET_SOURCES domset.cc )
set( DOM_SET_HEADERS domset.h types.h )

add_library( domset ${DOM_SET_SOURCES} ${DOM_SET_HEADERS} )
target_include_directories( domset PUBLIC ${EIGEN_INCLUDE_DIRS} )
