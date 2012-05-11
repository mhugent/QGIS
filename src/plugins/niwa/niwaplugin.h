#ifndef NIWAPLUGIN_H
#define NIWAPLUGIN_H

#include "qgisplugin.h"
#include <QObject>

class QgisInterface;
class QAction;

class NiwaPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    NiwaPlugin( QgisInterface* iface );
    ~NiwaPlugin();

    void initGui();
    void unload();

  private slots:
    void showNiwaDialog();

  private:
    QgisInterface* mIface;
    QAction* mAction;
};

#endif // NIWAPLUGIN_H
