########################################################
# Files

SET (SOS_SRCS
     qgssosplugin.cpp
     qgssossourceselect.cpp
     qgssoscapabilities.cpp
     qgsmaptoolsensorinfo.cpp
     qgssensorinfodialog.cpp
     qgswatermldata.cpp
)

SET (SOS_UIS
    qgssossourceselectbase.ui
    qgssensorinfodialogbase.ui
    )

SET (SOS_MOC_HDRS
qgssosplugin.h
qgssossourceselect.h
qgssoscapabilities.h
qgsmaptoolsensorinfo.h
qgssensorinfodialog.h
)

IF (WIN32)
	ADD_DEFINITIONS(-DQWT_DLL=1)
ENDIF (WIN32)

SET (SOS_RCCS  resources.qrc)
########################################################
# Build

QT4_WRAP_UI (SOS_UIS_H  ${SOS_UIS})

QT4_WRAP_CPP( SOS_MOC_SRCS ${SOS_MOC_HDRS})

QT4_ADD_RESOURCES(SOS_RCC_SRCS ${SOS_RCCS})

ADD_LIBRARY (sosplugin MODULE
  ${SOS_SRCS}
  ${SOS_MOC_SRCS}
  ${SOS_UIS_H}
  ${SOS_RCC_SRCS}
 )

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core
     ../../gui
     ${CMAKE_CURRENT_BINARY_DIR}/../../ui
     ${QWT_INCLUDE_DIR}
     ${EXPAT_INCLUDE_DIR}
     ..
     .
)

TARGET_LINK_LIBRARIES(sosplugin
  qgis_core
  qgis_gui
  ${QWT_LIBRARY}
)


########################################################
# Install

INSTALL(TARGETS sosplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
