########################################################
# Files

SET (SOS_SRCS
     qgssosplugin.cpp
)

#SET (SOS_UIS
#    qgsrasterterrainanalysisdialogbase.ui
#    )

SET (SOS_MOC_HDRS
qgssosplugin.h
)

#SET (SOS_RCCS  resources.qrc)
########################################################
# Build

#QT4_WRAP_UI (SOS_UIS_H  ${RASTER_TERRAIN_UIS})

QT4_WRAP_CPP( SOS_MOC_SRCS ${SOS_MOC_HDRS})

#QT4_ADD_RESOURCES(SOS_RCC_SRCS ${SOS_RCCS})

ADD_LIBRARY (sosplugin MODULE
  ${SOS_SRCS}
  ${SOS_MOC_SRCS}
 # ${SOS_RCC_SRCS}
 # ${SOS_UIS_H}
 )

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core
     ../../gui
     ..
     .
)

TARGET_LINK_LIBRARIES(sosplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS sosplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
