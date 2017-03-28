#include "mccontext.h"
#include "mcviewfactories.h"

#include "clusterdetailview.h"
#include <views/compareclusterview.h>
#include <mvtimeseriesview2.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClusterDetail1Factory::ClusterDetail1Factory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString ClusterDetail1Factory::id() const
{
    return QStringLiteral("open-cluster-details-1");
}

QString ClusterDetail1Factory::name() const
{
    return tr("Cluster Details 1");
}

QString ClusterDetail1Factory::title() const
{
    return tr("Details 1");
}

MVAbstractView* ClusterDetail1Factory::createView(MVAbstractContext* context)
{
    MCContext* mc_context = qobject_cast<MCContext*>(context);
    if (!mc_context)
        return 0;

    ClusterDetailView* X = new ClusterDetailView(mc_context->mvContext1());
    return X;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClusterDetail2Factory::ClusterDetail2Factory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString ClusterDetail2Factory::id() const
{
    return QStringLiteral("open-cluster-details-2");
}

QString ClusterDetail2Factory::name() const
{
    return tr("Cluster Details 2");
}

QString ClusterDetail2Factory::title() const
{
    return tr("Details 2");
}

MVAbstractView* ClusterDetail2Factory::createView(MVAbstractContext* context)
{
    MCContext* mc_context = qobject_cast<MCContext*>(context);
    if (!mc_context)
        return 0;

    ClusterDetailView* X = new ClusterDetailView(mc_context->mvContext2());
    return X;
}

////////////////////////////////////////////////////////////////////////
MVTimeSeriesView1Factory::MVTimeSeriesView1Factory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVTimeSeriesView1Factory::id() const
{
    return QStringLiteral("open-timeseries-data-1");
}

QString MVTimeSeriesView1Factory::name() const
{
    return tr("Timeseries Data 1");
}

QString MVTimeSeriesView1Factory::title() const
{
    return tr("Timeseries 1");
}

MVAbstractView* MVTimeSeriesView1Factory::createView(MVAbstractContext* context)
{
    MCContext* c = qobject_cast<MCContext*>(context);
    Q_ASSERT(c);

    MVTimeSeriesView2* X = new MVTimeSeriesView2(c->mvContext1());
    QList<int> ks = c->mvContext1()->selectedClusters();
    X->setLabelsToView(ks.toSet());
    return X;
}

////////////////////////////////////////////////////////////////////////
MVTimeSeriesView2Factory::MVTimeSeriesView2Factory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVTimeSeriesView2Factory::id() const
{
    return QStringLiteral("open-timeseries-data-2");
}

QString MVTimeSeriesView2Factory::name() const
{
    return tr("Timeseries Data 2");
}

QString MVTimeSeriesView2Factory::title() const
{
    return tr("Timeseries 2");
}

MVAbstractView* MVTimeSeriesView2Factory::createView(MVAbstractContext* context)
{
    MCContext* c = qobject_cast<MCContext*>(context);
    Q_ASSERT(c);

    MVTimeSeriesView2* X = new MVTimeSeriesView2(c->mvContext2());
    QList<int> ks = c->mvContext2()->selectedClusters();
    X->setLabelsToView(ks.toSet());
    return X;
}

////////////////////////////////////////////////////////////////////////
CompareClustersFactory::CompareClustersFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString CompareClustersFactory::id() const
{
    return QStringLiteral("open-compare-clusters");
}

QString CompareClustersFactory::name() const
{
    return tr("Compare Clusters");
}

QString CompareClustersFactory::title() const
{
    return tr("Compare Clusters");
}

MVAbstractView* CompareClustersFactory::createView(MVAbstractContext* context)
{
    MCContext* mc_context = qobject_cast<MCContext*>(context);
    if (!mc_context)
        return 0;

    CompareClusterView* X = new CompareClusterView(mc_context);

    return X;
}
