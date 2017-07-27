#include "sslvprefscontrol.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>
#include <sslvcontext.h>
#include "mlcommon.h"

class SSLVPrefsControlPrivate {
public:
    SSLVPrefsControl* q;
};

SSLVPrefsControl::SSLVPrefsControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new SSLVPrefsControlPrivate;
    d->q = this;

    QGridLayout* glayout = new QGridLayout;
    //int row = 0;

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

SSLVPrefsControl::~SSLVPrefsControl()
{
    delete d;
}

QString SSLVPrefsControl::title() const
{
    return "Preferences";
}

void SSLVPrefsControl::updateContext()
{
    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    //c->setOption("spectrogram_time_resolution", this->controlValue("spectrogram_time_resolution").toDouble());
    //c->setOption("spectrogram_freq_range", this->controlValue("spectrogram_freq_range").toString());
    //c->setOption("timeseries_freq_range", this->controlValue("timeseries_freq_range").toString());
}

void SSLVPrefsControl::updateControls()
{
    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    //this->setControlValue("spectrogram_time_resolution", c->option("spectrogram_time_resolution").toDouble());
    //this->setControlValue("spectrogram_freq_range", c->option("spectrogram_freq_range").toString());
    //this->setControlValue("timeseries_freq_range", c->option("timeseries_freq_range").toString());
}
