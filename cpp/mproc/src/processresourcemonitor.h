#ifndef PROCESSRESOURCEMONITOR_H
#define PROCESSRESOURCEMONITOR_H

#include "processormanager.h"

class ProcessResourceMonitorPrivate;
class ProcessResourceMonitor {
public:
    friend class ProcessResourceMonitorPrivate;
    ProcessResourceMonitor();
    virtual ~ProcessResourceMonitor();
    void setQProcess(QProcess* qprocess);
    void setProcessor(const MLProcessor& MLP);
    void setCLP(const QVariantMap& clp);

    bool withinLimits();

private:
    ProcessResourceMonitorPrivate* d;
};

#endif // PROCESSRESOURCEMONITOR_H
