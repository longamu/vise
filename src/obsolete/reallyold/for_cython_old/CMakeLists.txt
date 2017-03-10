cmake_minimum_required (VERSION 2.6)

PROJECT( relja_ransac_cpp )

#orig path: /usr/local/cuda/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/opt/real/RealPlayer
# export PATH=/projects/metapkg/bin64:$PATH
# then rm CMakeCache.txt , cmake ../ etc
#SET(CMAKE_MODULE_PATH "/projects/metapkg/lib64")
#SET(CMAKE_LIBRARY_PATH "/projects/metapkg/lib64")
#SET(CMAKE_INCLUDE_PATH "/projects/metapkg/include/")

############ options in my code

ADD_DEFINITIONS(-D_DO_HAFF_)
#ADD_DEFINITIONS(-D_DO_FAF4_)
ADD_DEFINITIONS(-D_AB_RATIO_LIMIT_=4.0)
#ADD_DEFINITIONS(-D_DO_HAFF_THEN_FAF2_)
ADD_DEFINITIONS(-D_HAFF_MIN_INLIERS_=3)

#ADD_DEFINITIONS(-D_TAN_CHANGE_)
#ADD_DEFINITIONS(-D_TAN_CHANGE_THRESH_=1.5)

ADD_DEFINITIONS(-D_LOOSEH_AFTER_)

#ADD_DEFINITIONS(-D_NEIGH_)

ADD_DEFINITIONS(-D_POST_TAN_CHANGE_)
ADD_DEFINITIONS(-D_POST_TAN_CHANGE_THRESH_=3.0)

ADD_DEFINITIONS(-D_SURVIVAL_)
ADD_DEFINITIONS(-D_SURVIVAL_THRESH_=0.55)

#ADD_DEFINITIONS(-D_RANSAC_VERBOSE_)
#ADD_DEFINITIONS(-D_FAF2_EXTRA_VERBOSE_)


############ generate files using cython

set(CYTHON_COMPILE "cython")
ADD_CUSTOM_COMMAND(
    OUTPUT    ${CMAKE_CURRENT_SOURCE_DIR}/wrapper_ransac.cpp
    DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/wrapper_ransac.pyx
    COMMAND   ${CYTHON_COMPILE} --cplus ${CMAKE_CURRENT_SOURCE_DIR}/wrapper_ransac.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrapper_ransac.cpp
    )

############ get dependencies

### VXL project
FIND_PACKAGE(VXL)

IF(NOT VXL_FOUND)
    MESSAGE( FATAL_ERROR "Could not find VXL!" )
ENDIF(NOT VXL_FOUND)

INCLUDE(${VXL_CMAKE_DIR}/UseVXL.cmake)

### Python (needed for cython stuff for eninge_3 integration)
FIND_PACKAGE(PythonLibs)
IF(NOT PYTHON_INCLUDE_PATH)
    MESSAGE( FATAL_ERROR "Could not find PythonLibs!" )
ENDIF(NOT PYTHON_INCLUDE_PATH)

INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

#set( PYTHON_INCLUDE_PATH "/projects/metapkg/lib64/libpython2.6.so")
#set( PYTHON_INCLUDE_PATH "/projects/metapkg/include/python2.6")
MESSAGE(${PYTHON_INCLUDE_PATH})

### need -fPIC for cython stuff
SET( CMAKE_CXX_FLAGS "-Wall -fPIC -O2 -g3 -msse2")

############ my code

ADD_LIBRARY( relja_putative putative.cpp putative.h )
TARGET_LINK_LIBRARIES( relja_putative vnl vcl )

ADD_LIBRARY( relja_ransac ransac.cpp )
TARGET_LINK_LIBRARIES( relja_ransac vnl vcl )

ADD_LIBRARY( relja_ellipse ellipse.cpp ellipse.h )
TARGET_LINK_LIBRARIES( relja_ellipse vnl vcl vnl_algo vgl vgl_algo )

ADD_LIBRARY( relja_homography homography.cpp homography.h ellipse.h )
TARGET_LINK_LIBRARIES( relja_homography relja_ellipse )

ADD_LIBRARY( relja_fit_affine fit_affine.cpp fit_affine.h homography.h ellipse.h )
TARGET_LINK_LIBRARIES( relja_fit_affine relja_homography relja_ellipse vcl vgl_algo )

ADD_LIBRARY( relja_fit_fund_affine4 fit_fund_affine4.cpp fit_fund_affine4.h )
TARGET_LINK_LIBRARIES( relja_fit_fund_affine4 relja_ellipse vcl vgl_algo )

ADD_LIBRARY( relja_solve_quartic_methods solve_quartic_methods.cpp solve_quartic_methods.h )
TARGET_LINK_LIBRARIES( relja_solve_quartic_methods vcl )

ADD_LIBRARY( relja_fit_fund_affine2 fit_fund_affine2.cpp fit_fund_affine2.h )
TARGET_LINK_LIBRARIES( relja_fit_fund_affine2 relja_solve_quartic_methods relja_fit_fund_affine4 relja_ellipse vcl vgl_algo )

ADD_EXECUTABLE( test test.cpp ransac.h homography.h fit_fund_affine4.h )
TARGET_LINK_LIBRARIES( test relja_ransac relja_fit_affine relja_fit_fund_affine4 relja_fit_fund_affine2 )

ADD_LIBRARY( relja_wrapper_ransac_engine3 wrapper_ransac_engine3.cpp wrapper_ransac_engine3.h fit_affine.h fit_fund_affine4.h fit_fund_affine2.h homography.h putative.h )
TARGET_LINK_LIBRARIES( relja_wrapper_ransac_engine3 relja_ransac relja_fit_affine relja_fit_fund_affine4 relja_fit_fund_affine2 relja_putative )

ADD_LIBRARY( relja_wrapper_ransac SHARED wrapper_ransac.cpp wrapper_ransac_engine3.h )
SET_TARGET_PROPERTIES(relja_wrapper_ransac PROPERTIES OUTPUT_NAME wrapper_ransac PREFIX "")
TARGET_LINK_LIBRARIES( relja_wrapper_ransac relja_wrapper_ransac_engine3 )
