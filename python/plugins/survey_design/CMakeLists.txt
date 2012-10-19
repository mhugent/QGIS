SET(INSTALLER_FILES
__init__.py
surveydesignplugin.py
surveyinitdialog.py
surveydesigndialog.py
surveydigitizetool.py
surveyevaluationdialog.py
surveyevaluation.py
)

PYQT4_WRAP_UI(PYUI_FILES
surveyinitdialogbase.ui
surveydesigndialogbase.ui
surveyevaluationdialogbase.ui
)

PYQT4_ADD_RESOURCES( PYRC_FILES resources.qrc )

ADD_CUSTOM_TARGET(survey_design ALL DEPENDS ${PYUI_FILES} ${PYRC_FILES})

INSTALL(FILES ${PYUI_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/survey_design)
INSTALL(FILES ${PYRC_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/survey_design)
INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/survey_design)
