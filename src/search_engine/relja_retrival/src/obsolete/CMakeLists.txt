if ( ISTITAN )
    
    ## sqlite3
    include_directories( "/users/relja/Relja/Programs/sqlite3/include" )
    link_directories( "/users/relja/Relja/Programs/sqlite3/lib" )
    
    ## should fix this HACK
    ADD_DEFINITIONS(-DISTITAN)
    
endif ( ISTITAN )
