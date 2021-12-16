#include <QQmlEngine>

#include "plugin_p.h"
#include "windowstate_p.h"

void KConfigQMLPlugin::registerTypes(const char* uri)
{
    qmlRegisterType<WindowStateSaver>(uri, 1, 0, "WindowStateSaver");
}
