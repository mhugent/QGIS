#ifndef WEBDATAPLUGIN_H
#define WEBDATAPLUGIN_H

#include "qgisplugin.h"
#include <QObject>

class QgisInterface;
class QAction;
class WebDataDialog;

class WebDataPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    WebDataPlugin( QgisInterface* iface );
    ~WebDataPlugin();

    void initGui();
    void unload();

  private slots:
    void showWebDataDialog();

  private:
    QgisInterface* mIface;
    QAction* mAction;
    WebDataDialog* mDialog;
};

#endif // WEBDATAPLUGIN_H
