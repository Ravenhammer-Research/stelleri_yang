cmake_minimum_required(VERSION 3.16)
project(yang_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBYANG REQUIRED libyang)

# Try to find libatf-c++-2 via pkg-config for building ATF tests (kyua-compatible)
pkg_check_modules(ATF_CPP QUIET atf-c++-2)

# If pkg-config didn't find ATF, try locating headers/libs directly
if (NOT ATF_CPP_FOUND)
	find_path(ATF_CPP_INCLUDE_DIR atf-c++.hpp)
	find_library(ATF_CPP_LIBRARIES NAMES atf-c++-2 atf-c++)
	if (ATF_CPP_INCLUDE_DIR AND ATF_CPP_LIBRARIES)
		set(ATF_CPP_FOUND TRUE)
		set(ATF_CPP_INCLUDE_DIRS ${ATF_CPP_INCLUDE_DIR})
		# already have ATF_CPP_LIBRARIES from find_library
	endif()
endif()

include_directories(${LIBYANG_INCLUDE_DIRS})
link_directories(${LIBYANG_LIBRARY_DIRS})
add_compile_options(${LIBYANG_CFLAGS_OTHER})

# project includes and sources
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
file(GLOB YANG_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Build as a library instead of an executable
add_library(yang_lib STATIC ${YANG_SOURCES})
target_include_directories(yang_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(yang_lib PUBLIC ${LIBYANG_LIBRARIES})

# Optional: provide a lightweight test executable if desired (disabled by default)
add_executable(TestYang tests/TestYang.cpp)
target_link_libraries(TestYang PRIVATE yang_lib ${LIBYANG_LIBRARIES})

add_executable(TestYangContext tests/TestYangContext.cpp)
target_link_libraries(TestYangContext PRIVATE yang_lib ${LIBYANG_LIBRARIES})


# add tests
add_executable(TestIetfInterfaces tests/TestIetfInterfaces.cpp)
target_link_libraries(TestIetfInterfaces PRIVATE yang_lib ${LIBYANG_LIBRARIES})

add_executable(TestIetfRouting tests/TestIetfRouting.cpp)
target_link_libraries(TestIetfRouting PRIVATE yang_lib ${LIBYANG_LIBRARIES})

# Add IETF network-instance test (use existing TestIetfNetworkInstance.cpp)
add_executable(TestIetfNetworkInstance tests/TestIetfNetworkInstance.cpp)
target_link_libraries(TestIetfNetworkInstance PRIVATE yang_lib ${LIBYANG_LIBRARIES})

if(ATF_CPP_FOUND)
	target_include_directories(TestYang PRIVATE ${ATF_CPP_INCLUDE_DIRS})
	target_link_libraries(TestYang PRIVATE ${ATF_CPP_LIBRARIES})
	target_include_directories(TestYangContext PRIVATE ${ATF_CPP_INCLUDE_DIRS})
	target_link_libraries(TestYangContext PRIVATE ${ATF_CPP_LIBRARIES})

	target_include_directories(TestIetfInterfaces PRIVATE ${ATF_CPP_INCLUDE_DIRS})
	target_link_libraries(TestIetfInterfaces PRIVATE ${ATF_CPP_LIBRARIES})
	target_include_directories(TestIetfRouting PRIVATE ${ATF_CPP_INCLUDE_DIRS})
	target_link_libraries(TestIetfRouting PRIVATE ${ATF_CPP_LIBRARIES})

	# ATF settings for network-instance test
	target_include_directories(TestIetfNetworkInstance PRIVATE ${ATF_CPP_INCLUDE_DIRS})
	target_link_libraries(TestIetfNetworkInstance PRIVATE ${ATF_CPP_LIBRARIES})
else()
	message(WARNING "libatf-c++-2 not found via pkg-config; tests will still build but won't include ATF headers")
endif()

enable_testing()
add_test(NAME IetfInterfaces COMMAND TestIetfInterfaces)
add_test(NAME IetfRouting COMMAND TestIetfRouting)
add_test(NAME IetfNetworkInstance COMMAND TestIetfNetworkInstance)

# Copy Kyuafile from tests/ into the build directory so kyua can find it next
# to the built test executables. This runs at configure time.
configure_file(${CMAKE_SOURCE_DIR}/tests/Kyuafile ${CMAKE_BINARY_DIR}/Kyuafile COPYONLY)

# Ensure a default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Always enable debug flags for C and C++ (unconditional)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -g -O0 -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fno-omit-frame-pointer")
add_compile_options(-g -O0 -fno-omit-frame-pointer)

# Ensure executables export symbols for backtrace resolution and avoid stripping
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
string(REPLACE "-s" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
string(REPLACE "-s" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

