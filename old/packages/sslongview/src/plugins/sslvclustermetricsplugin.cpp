/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "sslvclustermetricsplugin.h"

#include "sslvclustermetricsview.h"

#include <QThread>
#include <mountainprocessrunner.h>

class SSLVClusterMetricsPluginPrivate {
public:
    SSLVClusterMetricsPlugin* q;
};

SSLVClusterMetricsPlugin::SSLVClusterMetricsPlugin()
{
    d = new SSLVClusterMetricsPluginPrivate;
    d->q = this;
}

SSLVClusterMetricsPlugin::~SSLVClusterMetricsPlugin()
{
    delete d;
}

QString SSLVClusterMetricsPlugin::name()
{
    return "SSLVClusterMetrics";
}

QString SSLVClusterMetricsPlugin::description()
{
    return "";
}

void compute_basic_metrics(MVAbstractContext* mv_context);
void SSLVClusterMetricsPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new SSLVClusterMetricsFactory(mw));
}

SSLVClusterMetricsFactory::SSLVClusterMetricsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString SSLVClusterMetricsFactory::id() const
{
    return QStringLiteral("open-cluster-metrics");
}

QString SSLVClusterMetricsFactory::name() const
{
    return tr("Cluster Metrics");
}

QString SSLVClusterMetricsFactory::title() const
{
    return tr("Cluster Metrics");
}

MVAbstractView* SSLVClusterMetricsFactory::createView(MVAbstractContext* context)
{
    SSLVClusterMetricsView* X = new SSLVClusterMetricsView(context);
    return X;
}
