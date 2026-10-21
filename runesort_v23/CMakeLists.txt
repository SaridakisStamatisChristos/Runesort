add_executable(test_v23_prefetch test_prefetch.cpp)
target_compile_features(test_v23_prefetch PRIVATE cxx_std_17)
target_include_directories(test_v23_prefetch PRIVATE ..)
add_test(NAME v23_prefetch COMMAND test_v23_prefetch)
