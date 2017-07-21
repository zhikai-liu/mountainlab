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
    //mw->registerViewFactory(new ClusterPairMetricsFactory(mw));
    //compute_basic_metrics(mw->mvContext());
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

void compute_basic_metrics(MVAbstractContext* mv_context)
{
    SVContext* c = qobject_cast<SVContext*>(mv_context);
    Q_ASSERT(c);
    basic_metrics_calculator* thread = new basic_metrics_calculator;
    thread->timeseries = c->currentTimeseries().makePath();
    thread->firings = c->firings().makePath();
    thread->samplerate = c->sampleRate();
    thread->mv_context = c;
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(slot_on_finished()));
    thread->start();
}

void basic_metrics_calculator::run()
{
    MountainProcessRunner MPR;
    MPR.setProcessorName("basic_metrics");
    QMap<QString, QVariant> params;
    params["timeseries"] = timeseries;
    params["firings"] = firings;
    params["samplerate"] = samplerate;
    MPR.setInputParameters(params);
    cluster_metrics_path = MPR.makeOutputFilePath("cluster_metrics");
    MPR.runProcess();
}

void basic_metrics_calculator::slot_on_finished()
{
    //mv_context->loadClusterMetricsFromFile(cluster_metrics_path);
    //QString output = CurationProgramView::applyCurationProgram(mv_context);
    //printf("%s\n", output.toUtf8().data());
}
