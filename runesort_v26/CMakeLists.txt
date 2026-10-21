add_executable(test_v26_nibble test_nibble.cpp)
target_compile_features(test_v26_nibble PRIVATE cxx_std_17)
target_include_directories(test_v26_nibble PRIVATE ..)
add_test(NAME v26_nibble COMMAND test_v26_nibble)
