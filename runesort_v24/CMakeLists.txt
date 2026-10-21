add_executable(test_v24_dispatch test_dispatch.cpp)
target_compile_features(test_v24_dispatch PRIVATE cxx_std_17)
target_include_directories(test_v24_dispatch PRIVATE ..)
add_test(NAME v24_dispatch COMMAND test_v24_dispatch)
