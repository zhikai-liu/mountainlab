#include "exporttimeintervaldlg.h"
#include "sslvtoolscontrol.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include <mountainprocessrunner.h>
#include <sslvcontext.h>
#include <taskprogress.h>
#include "mlcommon.h"

class SSLVToolsControlPrivate {
public:
    SSLVToolsControl* q;
};

SSLVToolsControl::SSLVToolsControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new SSLVToolsControlPrivate;
    d->q = this;

    QGridLayout* glayout = new QGridLayout;
    int row = 0;

    {
        QPushButton* B = new QPushButton("Export time interval...");
        connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_export_time_interval()));
        glayout->addWidget(B, row, 0);
        row++;
    }

    /*
    {
        QWidget* X = this->createDoubleControl("spectrogram_time_resolution");
        X->setToolTip("Spectrogram time resolution in number of timepoints");
        context->onOptionChanged("spectrogram_time_resolution", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Spect time res:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createStringControl("spectrogram_freq_range");
        X->setToolTip("Spectrogram frequency range");
        context->onOptionChanged("spectrogram_freq_range", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Spect freq range (Hz):"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createStringControl("timeseries_freq_range");
        X->setToolTip("Spectrogram frequency range");
        context->onOptionChanged("timeseries_freq_range", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Timeseries freq range (Hz):"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    */

    /*
    {
        QWidget* X = this->createChoicesControl("discrim_hist_method");
        QStringList choices;
        choices << "centroid"
                << "svm";
        this->setChoices("discrim_hist_method", choices);
        context->onOptionChanged("discrim_hist_method", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Discrim hist method:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    */
    this->setLayout(glayout);

    updateControls();
}

SSLVToolsControl::~SSLVToolsControl()
{
    delete d;
}

QString SSLVToolsControl::title() const
{
    return "Preferences";
}

void SSLVToolsControl::updateContext()
{
    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    //c->setOption("spectrogram_time_resolution", this->controlValue("spectrogram_time_resolution").toDouble());
    //c->setOption("spectrogram_freq_range", this->controlValue("spectrogram_freq_range").toString());
    //c->setOption("timeseries_freq_range", this->controlValue("timeseries_freq_range").toString());
}

void SSLVToolsControl::updateControls()
{
    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    //this->setControlValue("spectrogram_time_resolution", c->option("spectrogram_time_resolution").toDouble());
    //this->setControlValue("spectrogram_freq_range", c->option("spectrogram_freq_range").toString());
    //this->setControlValue("timeseries_freq_range", c->option("timeseries_freq_range").toString());
}

class ExportTimeIntervalThread : public QThread {
public:
    //input
    QString timeseries_list;
    QString firings;
    double t1, t2;
    QString timeseries_out;
    QString firings_out;

    void run()
    {
        TaskProgress task("Exporting time interval");
        MountainProcessRunner MPR;
        MPR.setProcessorName("mountainsort.extract_time_interval");
        QVariantMap params;
        params["timeseries_list"] = timeseries_list;
        params["firings"] = firings;
        params["t1"] = t1;
        params["t2"] = t2;
        params["timeseries_out"] = timeseries_out;
        params["firings_out"] = firings_out;
        MPR.setInputParameters(params);
        task.log() << params;
        MPR.runProcess();
    }
};

void SSLVToolsControl::slot_export_time_interval()
{
    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    ExportTimeIntervalDlg dlg;
    if (dlg.exec() == QDialog::Accepted) {
        double t1 = dlg.startTimeSec() * c->sampleRate();
        double t2 = t1 + dlg.durationSec() * c->sampleRate() - 1;
        ExportTimeIntervalThread* A = new ExportTimeIntervalThread;
        A->timeseries_list = c->timeseries().makePath();
        A->firings = c->firings().makePath();
        A->t1 = t1;
        A->t2 = t2;
        A->timeseries_out = dlg.directory() + "/" + dlg.timeseriesFileName();
        A->firings_out = dlg.directory() + "/" + dlg.firingsFileName();
        QObject::connect(A, SIGNAL(finished()), A, SLOT(deleteLater()));
        A->start();
    }
}
