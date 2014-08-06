cmake_minimum_required(VERSION 2.8.9)

# Top-Level CmakeLists.txt
PROJECT( BrainTumorSeg )

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_NAME BrainTumorSegmentation)
  set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/BrainTumorSegmentation")
  set(EXTENSION_CATEGORY "Examples")
  set(EXTENSION_CONTRIBUTORS "Albert Montillo (GE Research)")
  set(EXTENSION_DESCRIPTION "Segment brain volume into 3 classes: tumor, edema, healthy tissue from multi-contrast MR: {T1, T2, T1-Gd, FLAIR}")
  set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/BrainTumorSegmentation/BrainTumorSegmentation.png?revision=21745&view=co")
  set(EXTENSION_SCREENSHOTURLS "http://wiki.slicer.org/slicerWiki/images/a/ab/Slicer-r19441-BrainTumorSegmentation-screenshot.png http://wiki.slicer.org/slicerWiki/images/1/1e/Slicer-r19441-BrainTumorSegmentation-screenshot-2.png")
endif()

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
endif()

#-----------------------------------------------------------------------------
add_subdirectory(Blitz0.09CMake)
add_subdirectory(BrainTumorSegmentation)


#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()
