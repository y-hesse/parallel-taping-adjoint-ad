cmake_minimum_required(VERSION 3.21.1)
project(Adjoint)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fopenmp")

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(src)

add_executable(adjoint src/main.cpp src/dag.hpp src/double_o.hpp src/primal/primal.hpp
        src/verify.hpp src/checkpoint.hpp
        src/checkpointLoaderInterface.hpp
        src/disk/checkpointLoader.cpp src/disk/checkpointLoader.hpp
        src/memory/checkpointLoader.cpp src/memory/checkpointLoader.hpp
        src/hybrid/checkpointLoader.cpp src/hybrid/checkpointLoader.hpp
        src/naive/checkpointLoader.hpp
        examples/burgers/f.cpp examples/burgers/burgers.h examples/burgers/gauss.h examples/burgers/utils.h src/profiler.hpp src/aad.hpp examples/example1/example1.hpp examples/example1/example1.cpp examples/example2/example2.cpp examples/example2/example2.hpp src/primal/primal.cpp)

add_executable(adjoint_test tests/src/main.cpp src/dag.hpp src/double_o.hpp
        src/verify.hpp src/checkpoint.hpp
        src/checkpointLoaderInterface.hpp
        src/disk/checkpointLoader.cpp src/disk/checkpointLoader.hpp
        src/memory/checkpointLoader.cpp src/memory/checkpointLoader.hpp
        src/hybrid/checkpointLoader.cpp src/hybrid/checkpointLoader.hpp
        src/naive/checkpointLoader.hpp
        src/profiler.hpp src/aad.hpp tests/src/double_o_test.hpp tests/src/dag_test.hpp tests/src/checkpoint_test.hpp tests/src/test_function/test_function.cpp tests/src/test_function/test_function.hpp
        tests/src/aad_test.hpp
        examples/burgers/f.cpp examples/burgers/burgers.h examples/burgers/gauss.h examples/burgers/utils.h src/profiler.hpp src/aad.hpp examples/example1/example1.hpp examples/example1/example1.cpp examples/example2/example2.cpp examples/example2/example2.hpp
        src/primal/primal.cpp)

target_link_libraries(adjoint -lm -lstdc++)

enable_testing()

target_link_libraries(
        adjoint_test
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(adjoint_test)