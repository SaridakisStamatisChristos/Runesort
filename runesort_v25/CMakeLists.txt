add_executable(test_v25_simd test_simd.cpp)
target_compile_features(test_v25_simd PRIVATE cxx_std_17)
target_include_directories(test_v25_simd PRIVATE ..)
add_test(NAME v25_simd COMMAND test_v25_simd)
