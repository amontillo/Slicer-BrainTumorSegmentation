#-----------------------------------------------------------------------------
set(MODULE_NAME BrainTumorSegmentation)

#-----------------------------------------------------------------------------

# Set compiler Preprocessor Definitions
#  /D "BOOST_DISABLE_ASSERTS"
ADD_DEFINITIONS(-D"BOOST_DISABLE_ASSERTS")

# For Win32 Disable specific warnings
# /wd4267 /wd4244 /wd4805 /wd4800
if(WIN32)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244 /wd4805 /wd4800")
	  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244 /wd4805 /wd4800" CACHE STRING "compile flags" FORCE)
endif(WIN32)

#set(MODULE_INCLUDE_DIRECTORIES
#    ${BrainTumorSeg_SOURCE_DIR}/Blitz0.09CMake/include
#	boost_1_46_1
#    tinyxml_2_6_1/tinyxml
#  )


set(MODULE_INCLUDE_DIRECTORIES
   Blitz0.09CMake/include
	boost_1_46_1
    tinyxml_2_6_1/tinyxml
  )

set(MODULE_SRCS
    Controller.cpp
    ControllerITK.cpp
    Dataset.cpp
    DecisionForestTester.cpp
    DecisionTree.cpp
    DefaultForest.cpp    
    EnvironmentSettingsFileManager.cpp
    FeatureFarm.cpp
    ForestParameters.cpp
    IndexVolume.cpp
    Posterior.cpp
	tinyxml_2_6_1/tinyxml/tinystr.cpp
	tinyxml_2_6_1/tinyxml/tinyxml.cpp
	tinyxml_2_6_1/tinyxml/tinyXMLDumpToStdOut.cpp
	tinyxml_2_6_1/tinyxml/tinyxmlerror.cpp
	tinyxml_2_6_1/tinyxml/tinyxmlparser.cpp
  )

set(MODULE_TARGET_LIBRARIES
  blitz
  ${ITK_LIBRARIES}  
  )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  ADDITIONAL_SRCS ${MODULE_SRCS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()