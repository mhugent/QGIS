
########################################################
# Files

SET (vectoranalysis_SRCS
    convexhulltooldialog.cpp
    vectoranalysis.cpp
    geoprocessingerrordialog.cpp
    geoprocessingtooldialog.cpp
    buffertooldialog.cpp
    convexhulltooldialog.cpp
    dissolvetooldialog.cpp
    vectoranalysisutils.cpp
)

SET (vectoranalysis_HDRS
  convexhulltooldialog.h
  geoprocessingerrordialog.h
  geoprocessingtooldialog.h
  buffertooldialog.h
  convexhulltooldialog.h
  dissolvetooldialog.h
)

SET (vectoranalysis_UIS
  geoprocessingerrordialog.ui
  geoprocessingtooldialog.ui
)

SET (vectoranalysis_RCCS
    vectoranalysis.qrc
)

########################################################
# Build

QT5_WRAP_UI (vectoranalysis_UIS_H  ${vectoranalysis_UIS})

QT5_ADD_RESOURCES(vectoranalysis_RCC_SRCS ${vectoranalysis_RCCS})

ADD_LIBRARY (vectoranalysisplugin   MODULE ${vectoranalysis_HDRS} ${vectoranalysis_SRCS}  ${vectoranalysis_RCC_SRCS} ${vectoranalysis_UIS_H})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/providers/ogr
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/analysis/vector
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/ui

  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(vectoranalysisplugin
  qgis_core
  qgis_gui
  qgis_analysis
)

########################################################
# Install

INSTALL(TARGETS vectoranalysisplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

