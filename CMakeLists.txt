# CMakeLists.txt : CMake project for donRaulAva, include source and define
# project-specific logic here.

cmake_minimum_required(VERSION 3.17)

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
      "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Define project
project("donRaulAva")

# Set policy CMP0115 to NEW
if(POLICY CMP0115)
  cmake_policy(SET CMP0115 NEW)
endif()

# Set optimization flags for release
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL") # Link-time code generation
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2")

# Set up OpenCV environment variables
if(NOT DEFINED ENV{OpenCV_DIR})
  set(OpenCV_DIR "$ENV{USERPROFILE}\\Downloads\\opencv\\build")
endif()

set(CMAKE_PREFIX_PATH "${OpenCV_DIR}")
if(NOT CMAKE_SIZEOF_VOID_P)
  set(CMAKE_SIZEOF_VOID_P 8) # Replace with 4 for 32-bit systems
endif()

# Find and include OpenCV
find_package(OpenCV REQUIRED)

# Add executable and source files
add_executable(donRaulAva WIN32
  "donRaulAva.cpp"
  "utils.cpp"
  "WinApiScreenCapture.cpp"
  "DesktopDuplicateCapture.cpp"
  "ConfigDialog.cpp"
  "DetectLoop.cpp"
  "cv_utils.cpp"
  "resource.rc"
)

# Include and link OpenCV
target_include_directories(donRaulAva PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(donRaulAva
  ${OpenCV_LIBS}
  d3d11
  dxgi
)

# Set C++ standard to C++20 if supported
if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET donRaulAva PROPERTY CXX_STANDARD 20)
endif()

# Automatically copy OpenCV DLLs to the output directory after building
if (WIN32 AND OpenCV_FOUND)

# Check if OpenCV was found
  if (OpenCV_FOUND) 
    # Infer the bin folder path (typically, ../bin relative to lib)
    get_filename_component(OpenCV_INSTALL_DIR ${OpenCV_CONFIG_PATH} DIRECTORY)
    set(OpenCV_DLL_DIR "${OpenCV_INSTALL_DIR}/bin") 
    message(STATUS "OpenCV dll directory: ${OpenCV_DLL_DIR}")
    # Locate debug and release versions of opencv_world DLLs
    file(GLOB OpenCV_DEBUG_DLL "${OpenCV_DLL_DIR}/opencv_world*d.dll")
    file(GLOB OpenCV_RELEASE_DLL "${OpenCV_DLL_DIR}/opencv_world*.dll")

    # Remove debug DLLs from the release list by excluding anything that ends with "d.dll"
    list(FILTER OpenCV_RELEASE_DLL EXCLUDE REGEX ".*d\\.dll$")
    # Copy the correct DLL based on the configuration
    add_custom_command(TARGET donRaulAva POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<IF:$<CONFIG:Debug>,${OpenCV_DEBUG_DLL},${OpenCV_RELEASE_DLL}>
        $<TARGET_FILE_DIR:donRaulAva>)
  endif()
else()
    message(FATAL_ERROR "OpenCV was not found. Please make sure OpenCV is properly configured.")
endif()


# CPack Configuration
set(CPACK_PACKAGE_NAME "DonRaulAva")
set(CPACK_PACKAGE_VENDOR "Cobraxava DonRaul")
set(CPACK_PACKAGE_CONTACT "support@yourcompany.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "DonRaul's mini game player bot")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "DonRaulAva")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
#set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/favicon.ico")
#set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/favicon.ico")
#set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}/header.bmp")
set(CPACK_NSIS_WELCOME_TITLE "Welcome to DonRaulAva. We hope you enjoy it!")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "
!define MUI_ICON '${CMAKE_SOURCE_DIR}\\\\favicon.ico'
!define MUI_UNICON '${CMAKE_SOURCE_DIR}\\\\favicon.ico'
!define MUI_WELCOMEFINISHPAGE_BITMAP '${CMAKE_SOURCE_DIR}\\\\side.bmp'
!define MUI_HEADERIMAGE_BITMAP '${CMAKE_SOURCE_DIR}\\\\header.bmp'
")
# Create shortcuts
set(CPACK_NSIS_CREATE_ICONS_EXTRA "
  CreateShortCut '\$DESKTOP\\\\DonRaulAva.lnk' '\$INSTDIR\\\\bin\\\\DonRaulAva.exe'
  CreateDirectory '\$SMPROGRAMS\\\\DonRaulAva'
  CreateShortCut '\$SMPROGRAMS\\\\DonRaulAva\\\\DonRaulAva.lnk' '\$INSTDIR\\\\bin\\\\DonRaulAva.exe'
  CreateShortCut '\$SMPROGRAMS\\\\DonRaulAva\\\\Uninstall DonRaulAva.lnk' '\$INSTDIR\\\\Uninstall.exe'
 
")
# startup  CreateShortCut '\$SMSTARTUP\\\\DonRaulAva.lnk' '\$INSTDIR\\\\bin\\\\DonRaulAva.exe'
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
  " 
  ; Set Current User Context
  SetShellVarContext current

  ; Write Registry Entries for Current User
  WriteRegStr HKCU 'Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Uninstall\\\\DonRaulAva' 'DisplayName' 'DonRaulAva'
  WriteRegStr HKCU 'Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Uninstall\\\\DonRaulAva' 'UninstallString' '\$INSTDIR\\\\Uninstall.exe'
  WriteRegStr HKCU 'Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Uninstall\\\\DonRaulAva' 'InstallLocation' '\$INSTDIR'
  WriteRegStr HKCU 'Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Uninstall\\\\DonRaulAva' 'Publisher' 'CobraxAva Team'
  WriteRegStr HKCU 'Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Uninstall\\\\DonRaulAva' 'Version' '${CPACK_PACKAGE_VERSION}'
  ; Add the DisplayIcon key
  WriteRegStr HKCU 'Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Uninstall\\\\DonRaulAva' 'DisplayIcon' '\$INSTDIR\\\\Uninstall.exe'
  "
)

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
  ; Remove shortcuts from the desktop, Start Menu, and Startup folder
  Delete '\$DESKTOP\\\\DonRaulAva.lnk'
  Delete '\$SMPROGRAMS\\\\DonRaulAva\\\\DonRaulAva.lnk'
  Delete '\$SMPROGRAMS\\\\DonRaulAva\\\\Uninstall DonRaulAva.lnk'
  RmDir '\$SMPROGRAMS\\\\DonRaulAva' ; Remove the folder from Start Menu if empty

  ; Remove registry keys
  DeleteRegKey HKCU 'Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Uninstall\\\\DonRaulAva'
")
#  Delete '\$SMSTARTUP\\\\DonRaulAva.lnk'

include(CPack)

# Add runtime libraries
include(InstallRequiredSystemLibraries)

# Install your application files
install(TARGETS donRaulAva DESTINATION "bin")
install(FILES ${OpenCV_RELEASE_DLL} DESTINATION "bin")
install(FILES "Donation.txt" DESTINATION ".")
install(FILES "LICENSE.txt" DESTINATION ".")