#ifndef SPIKEVIEWMETRICSCOMPUTER_H
#define SPIKEVIEWMETRICSCOMPUTER_H

#include "svcontext.h"
#include <QThread>

class SpikeviewMetricsComputer : public QThread {
    Q_OBJECT
public:
    SpikeviewMetricsComputer(SVContext* context);
    virtual ~SpikeviewMetricsComputer();
    void run() Q_DECL_OVERRIDE;
private slots:
    void slot_finished();

private:
    SVContext* m_context;

    //input
    QString m_firings;
    double m_samplerate;

    //output
    QString m_metrics_out;
};

#endif // SPIKEVIEWMETRICSCOMPUTER_H
