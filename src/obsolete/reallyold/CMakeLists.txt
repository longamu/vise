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


############ my code

add_library( relja_putative putative.cpp putative.h )
target_link_libraries( relja_putative vnl vcl )

add_library( relja_ransac ransac.cpp )
target_link_libraries( relja_ransac vnl vcl )

add_library( relja_ellipse ellipse.cpp ellipse.h )
target_link_libraries( relja_ellipse vnl vcl vnl_algo vgl vgl_algo )

add_library( relja_homography homography.cpp homography.h ellipse.h )
target_link_libraries( relja_homography relja_ellipse ${Boost_LIBRARIES} )

add_library( relja_fit_affine fit_affine.cpp fit_affine.h homography.h ellipse.h )
target_link_libraries( relja_fit_affine relja_homography relja_ellipse vcl vgl_algo )

add_library( fit_affine3 fit_affine3.cpp )
target_link_libraries( relja_fit_affine relja_homography relja_ellipse vgl_algo )

add_library( relja_fit_fund_affine4 fit_fund_affine4.cpp fit_fund_affine4.h )
target_link_libraries( relja_fit_fund_affine4 relja_ellipse vcl vgl_algo )

add_library( relja_solve_quartic_methods solve_quartic_methods.cpp solve_quartic_methods.h )
target_link_libraries( relja_solve_quartic_methods vcl )

add_library( relja_fit_fund_affine2 fit_fund_affine2.cpp fit_fund_affine2.h )
target_link_libraries( relja_fit_fund_affine2 relja_solve_quartic_methods relja_fit_fund_affine4 relja_ellipse vcl vgl_algo )

add_executable( test test.cpp ransac.h homography.h fit_fund_affine4.h )
target_link_libraries( test relja_ransac relja_fit_affine relja_fit_fund_affine4 relja_fit_fund_affine2 )

add_library( ransac_relja_retrieval ransac_relja_retrieval.h ransac_relja_retrieval.cpp )
target_link_libraries( ransac_relja_retrieval relja_ransac relja_fit_affine relja_fit_fund_affine4 relja_fit_fund_affine2 relja_putative )
