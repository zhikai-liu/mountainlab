/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "clustermetricsplugin.h"

#include "clustermetricsview.h"

#include <QThread>
#include <mountainprocessrunner.h>

class ClusterMetricsPluginPrivate {
public:
    ClusterMetricsPlugin* q;
};

ClusterMetricsPlugin::ClusterMetricsPlugin()
{
    d = new ClusterMetricsPluginPrivate;
    d->q = this;
}

ClusterMetricsPlugin::~ClusterMetricsPlugin()
{
    delete d;
}

QString ClusterMetricsPlugin::name()
{
    return "ClusterMetrics";
}

QString ClusterMetricsPlugin::description()
{
    return "";
}

void compute_basic_metrics(MVAbstractContext* mv_context);
void ClusterMetricsPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new ClusterMetricsFactory(mw));
}

ClusterMetricsFactory::ClusterMetricsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString ClusterMetricsFactory::id() const
{
    return QStringLiteral("open-cluster-metrics");
}

QString ClusterMetricsFactory::name() const
{
    return tr("Cluster Metrics");
}

QString ClusterMetricsFactory::title() const
{
    return tr("Cluster Metrics");
}

MVAbstractView* ClusterMetricsFactory::createView(MVAbstractContext* context)
{
    ClusterMetricsView* X = new ClusterMetricsView(context);
    return X;
}
