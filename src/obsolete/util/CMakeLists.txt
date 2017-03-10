add_library( svm_util svm_util.cpp )
target_link_libraries( svm_util libsvm )

add_library( platt_sigmoidfit platt_sigmoidfit.cpp )
target_link_libraries( platt_sigmoidfit )
