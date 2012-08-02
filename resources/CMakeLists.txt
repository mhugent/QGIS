INSTALL(FILES niwa_faq.html add_to_list.png mActionPan.png mActionZoomOut.png mActionIdentify.png mActionZoomIn.png niwa_icon.png nqmap.png srs.db qgis.db qgis_help.db symbology-ng-style.xml spatialite.db customization.xml
        DESTINATION ${QGIS_DATA_DIR}/resources)

ADD_SUBDIRECTORY(context_help)
ADD_SUBDIRECTORY(function_help)
