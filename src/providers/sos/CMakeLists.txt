
########################################################
# Files

SET(SOS_SRCS
  qgssosdata.cpp
  qgssosfeatureiterator.cpp
  qgssosprovider.cpp
)

#SET (SOS_MOC_HDRS
#  qgssosdata.h
#)

########################################################
# Build

#QT4_WRAP_CPP(SOS_MOC_SRCS ${SOS_MOC_HDRS})

INCLUDE_DIRECTORIES (
  ../../core
#  ../../gui
#  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
#  ${GEOS_INCLUDE_DIR}
#  ${GEOS_INCLUDE_DIR}/geos
  ${EXPAT_INCLUDE_DIR}
)

ADD_LIBRARY (sosprovider MODULE ${SOS_SRCS} ) # ${SOS_MOC_SRCS} )

TARGET_LINK_LIBRARIES (sosprovider
  ${EXPAT_LIBRARY}
  qgis_core
  qgis_gui
)


########################################################
# Install
                
INSTALL(TARGETS sosprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
