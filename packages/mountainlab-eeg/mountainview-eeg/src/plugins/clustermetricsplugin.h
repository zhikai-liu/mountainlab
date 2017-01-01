/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/
#ifndef ClusterMetricsPLUGIN_H
#define ClusterMetricsPLUGIN_H

#include "mvmainwindow.h"

#include <QThread>

class ClusterMetricsPluginPrivate;
class ClusterMetricsPlugin : public MVAbstractPlugin {
public:
    friend class ClusterMetricsPluginPrivate;
    ClusterMetricsPlugin();
    virtual ~ClusterMetricsPlugin();

    QString name() Q_DECL_OVERRIDE;
    QString description() Q_DECL_OVERRIDE;
    void initialize(MVMainWindow* mw) Q_DECL_OVERRIDE;

private:
    ClusterMetricsPluginPrivate* d;
};

class ClusterMetricsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    ClusterMetricsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // ClusterMetricsPLUGIN_H
