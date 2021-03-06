include ( "CMakeModules/ODExternals.cmake" )

DEFINE_GIT_EXTERNAL( proj4 https://github.com/OpendTect/proj4.git 5.1.0 )

if ( NOT OD_NO_OSG )
    DEFINE_GIT_EXTERNAL( osgGeo https://github.com/OpendTect/osgGeo.git master )
endif()
