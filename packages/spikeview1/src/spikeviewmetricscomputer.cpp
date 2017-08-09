#include "spikeviewmetricscomputer.h"

#include <mountainprocessrunner.h>
#include <taskprogress.h>

SpikeviewMetricsComputer::SpikeviewMetricsComputer(SVContext* context)
{
    m_context = context;
    m_firings = context->firings().makePath();
    QObject::connect(this, SIGNAL(finished()), this, SLOT(slot_finished()));
}

SpikeviewMetricsComputer::~SpikeviewMetricsComputer()
{
    this->terminate();
    this->wait();
}

void SpikeviewMetricsComputer::run()
{
    TaskProgress task("Computing spikeview metrics");
    QString firings = m_firings;
    if (firings.isEmpty()) {
        task.error("No firings file found");
        return;
    }
    MountainProcessRunner MPR;
    MPR.setAllowGuiThread(false);
    MPR.setProcessorName("spikeview.metrics1");
    QVariantMap params;
    params["firings"] = firings;
    MPR.setInputParameters(params);
    QString metrics_out = MPR.makeOutputFilePath("metrics_out");
    MPR.runProcess();
    m_metrics_out = metrics_out;
}

void SpikeviewMetricsComputer::slot_finished()
{
    if (!m_metrics_out.isEmpty())
        m_context->loadClusterMetricsFromFile(m_metrics_out);
}
