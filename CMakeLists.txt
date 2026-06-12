#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16)
project(prad2sim)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Tell CMake where to find the ROOT installation
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} $ENV{ROOTSYS}/etc/cmake)
find_package(ROOT REQUIRED COMPONENTS MathMore Foam)

#----------------------------------------------------------------------------
# Setup include and lib directory for this project
#
include(${Geant4_USE_FILE})
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

#----------------------------------------------------------------------------
# CAD (STL) geometry import via the vendored CADMesh header
# (include/external/CADMesh.hh, built-in STL reader — Geant4 only).
# Needed for the full prad2 / x17 vacuum window and housing geometry.
#
option(PRAD2SIM_USE_CADMESH "Enable CAD (STL) geometry import via CADMesh" ON)
if(PRAD2SIM_USE_CADMESH)
  add_compile_definitions(PRAD2SIM_USE_CADMESH)
endif()

#----------------------------------------------------------------------------
# Project sources (explicit list — add new files here)
#
set(PRAD2SIM_SOURCES
  src/ActionInitialization.cc
  src/CalorimeterHit.cc
  src/CalorimeterSD.cc
  src/CheckScatteringSD.cc
  src/ConfigObject.cpp
  src/ConfigOption.cpp
  src/ConfigParser.cpp
  src/ConfigValue.cpp
  src/CosmicsGenerator.cc
  src/DetectorConstruction.cc
  src/DetectorMessenger.cc
  src/DeuteronDisintegration.cc
  src/DRadPrimaryGenerator.cc
  src/EventAction.cc
  src/EventMessenger.cc
  src/PhysicsListMessenger.cc
  src/PhysListEmModified.cc
  src/PhysListPureEm.cc
  src/PRadPrimaryGenerator.cc
  src/PrimaryGenerator.cc
  src/PrimaryGeneratorAction.cc
  src/PrimaryGeneratorMessenger.cc
  src/RootTree.cc
  src/SimConfig.cc
  src/StandardDetectorSD.cc
  src/StandardHit.cc
  src/SteppingVerbose.cc
  src/StepRecordSD.cc
  src/TrackInformation.cc
  src/TrackingAction.cc
  src/TrackingDetectorSD.cc
  src/TrackingMessenger.cc
  src/detector/BeamlineModule.cc
  src/detector/CadInsertsModule.cc
  src/detector/GEMModule.cc
  src/detector/HeBagModule.cc
  src/detector/HyCalModule.cc
  src/detector/MaterialBuilder.cc
  src/detector/ScintillatorModule.cc
  src/detector/TargetModule.cc
  src/detector/VacuumSystemModule.cc
  src/detector/VirtualDetModule.cc
)

#----------------------------------------------------------------------------
# Add the executable, and link it to the libraries
#
add_executable(prad2sim prad2sim.cc ${PRAD2SIM_SOURCES})
target_link_libraries(prad2sim ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------
# Add subdirectories to build
#
# The digitization stage (PRadDig/PRadRec) needs EVIO and the old PRadAnalyzer
# ($PRAD_PATH). Make it optional so the core simulation builds without them.
# The PRad-II data interface goes through prad2's sim2replay instead — see
# plans/03-data-interface.md.
option(BUILD_DIGITIZATION "Build the legacy EVIO/PRadAnalyzer digitization stage" ON)
if(BUILD_DIGITIZATION)
  add_subdirectory(digitization)
endif()

#----------------------------------------------------------------------------
# Copy all scripts to the build directory
# This is so that we can run the executable directly because it relies on these scripts being in the current working directory
#
set(PRAD2SIM_RESOURCE_FILES init_vis.mac vis.mac gui.mac run.mac)

add_custom_target(copy_files ALL COMMENT "Copy files")
foreach(_file ${PRAD2SIM_RESOURCE_FILES})
  add_custom_command(TARGET copy_files POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file}
  )
endforeach()

# Copy JSON configuration files to build directory
add_custom_command(TARGET copy_files POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/config ${PROJECT_BINARY_DIR}/config
)
