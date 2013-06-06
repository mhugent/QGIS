########################################################
# Files

SET (SOS_SRCS
     qgssosplugin.cpp
     qgssossourceselect.cpp
)

SET (SOS_UIS
    qgssossourceselectbase.ui
    )

SET (SOS_MOC_HDRS
qgssosplugin.h
qgssossourceselect.h
)

#SET (SOS_RCCS  resources.qrc)
########################################################
# Build

QT4_WRAP_UI (SOS_UIS_H  ${SOS_UIS})

QT4_WRAP_CPP( SOS_MOC_SRCS ${SOS_MOC_HDRS})

#QT4_ADD_RESOURCES(SOS_RCC_SRCS ${SOS_RCCS})

ADD_LIBRARY (sosplugin MODULE
  ${SOS_SRCS}
  ${SOS_MOC_SRCS}
  ${SOS_UIS_H}
   # ${SOS_RCC_SRCS}
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
