INSTALL(FILES srs.db qgis.db symbology-ng-style.db spatialite.db customization.xml customization.ini nqmap.png niwa_icon.png niwa_faq.html
        DESTINATION ${QGIS_DATA_DIR}/resources)
INSTALL(DIRECTORY cpt-city-qgis-min DESTINATION ${QGIS_DATA_DIR}/resources)

ADD_SUBDIRECTORY(context_help)
ADD_SUBDIRECTORY(function_help)
