#ifndef NIWAPLUGIN_H
#define NIWAPLUGIN_H

#include "qgisplugin.h"
class QgisInterface;

class NiwaPlugin: public QgisPlugin
{
    public:
    NiwaPlugin( QgisInterface* iface );
    ~NiwaPlugin();

    void initGui();
    void unload();

    private:
    QgisInterface* mIface;
};

#endif // NIWAPLUGIN_H
