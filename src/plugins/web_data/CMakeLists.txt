########################################################
# Files

SET (WEB_DATA_SRCS
     webdataplugin.cpp
     webdatadialog.cpp
     addservicedialog.cpp
     webdatamodel.cpp
     webdatafiltermodel.cpp
)

SET (WEB_DATA_UIS 
     webdatadialogbase.ui
     addservicedialogbase.ui
    )

SET (WEB_DATA_MOC_HDRS
     webdataplugin.h
     webdatadialog.h
     webdatamodel.h
)

SET (WEB_DATA_RCCS  resources.qrc)
########################################################
# Build

QT4_WRAP_UI (WEB_DATA_UIS_H  ${WEB_DATA_UIS})

QT4_WRAP_CPP (WEB_DATA_MOC_SRCS  ${WEB_DATA_MOC_HDRS})

QT4_ADD_RESOURCES(WEB_DATA_RCC_SRCS ${WEB_DATA_RCCS})

ADD_LIBRARY (webdataplugin MODULE 
  ${WEB_DATA_SRCS} 
  ${WEB_DATA_UIS_H}
  ${WEB_DATA_MOC_SRCS} 
  ${WEB_DATA_RCC_SRCS}
)

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}/../../ui
     ${GDAL_INCLUDE_DIR}
     ../../core
     ../../core/raster
     ../../gui
     ..
     .
)

TARGET_LINK_LIBRARIES(webdataplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS webdataplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
