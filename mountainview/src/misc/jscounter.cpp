#include "jscounter.h"

JSCounter::JSCounter(const QString &name) : IAggregateCounter(name)
{
    m_engine.installExtensions(QJSEngine::ConsoleExtension);
}
