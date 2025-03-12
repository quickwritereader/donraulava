# CMakeList.txt : CMake project for donRaulAva, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)


# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


project ("donRaulAva")

# Set policy CMP0115 to NEW
if (POLICY CMP0115)
  cmake_policy(SET CMP0115 NEW)
endif()


include(FetchContent)
# Add openCV package
# Declare and configure OpenCV
FetchContent_Declare(
  opencv
  GIT_REPOSITORY https://github.com/opencv/opencv.git
  GIT_TAG        4.11.0  
)
# Set the option to build OpenCV as a static library
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build OpenCV as static libraries")
FetchContent_MakeAvailable(opencv)

# Find and include OpenCV
find_package(OpenCV REQUIRED)

# Add executable and source files
add_executable(donRaulAva WIN32
  "donRaulAva.cpp"
  "utils.cpp"
  "donRaulAva.h"
  "detect_small.h"
  "resource.h"
  "ConfigDialog.h"
  "BaseDialog.h"
  "resource.rc"
)
target_include_directories(donRaulAva PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(donRaulAva ${OpenCV_LIBS})
# Set C++ standard if CMake version is greater than 3.12
if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET donRaulAva PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
