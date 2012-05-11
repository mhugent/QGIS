########################################################
# Files

SET (NIWA_SRCS
     niwaplugin.cpp
     niwaplugindialog.cpp
)

SET (NIWA_UIS 
     niwaplugindialogbase.ui
    )

SET (NIWA_MOC_HDRS
     niwaplugin.h
     niwaplugindialog.h
)

#SET (NIWA_RCCS  resources.qrc)
########################################################
# Build

QT4_WRAP_UI (NIWA_UIS_H  ${NIWA_UIS})

QT4_WRAP_CPP (NIWA_MOC_SRCS  ${NIWA_MOC_HDRS})

QT4_ADD_RESOURCES(NIWA_RCC_SRCS ${NIWA_RCCS})

ADD_LIBRARY (niwaplugin MODULE 
  ${NIWA_SRCS} 
  ${NIWA_UIS_H}
  ${NIWA_MOC_SRCS} 
#  ${NIWA_RCC_SRCS}
)

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core  
     ../../gui
     ..
     .
)

TARGET_LINK_LIBRARIES(niwaplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS niwaplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
