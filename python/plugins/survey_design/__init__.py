from surveydesignplugin import SurveyDesignPlugin

def name():
    return "Survey design plugin"

def description():
    return "A plugin to design and evaluate biomass surveys"

def version():
    return "0.1"

def qgisMinimumVersion():
    return "1.0.0"

def classFactory(iface):
    return SurveyDesignPlugin( iface )

#def icon():

#def category():
