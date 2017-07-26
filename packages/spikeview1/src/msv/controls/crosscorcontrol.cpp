#include "crosscorcontrol.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>
#include <svcontext.h>
#include "mlcommon.h"

class CrosscorControlPrivate {
public:
    CrosscorControl* q;
};

CrosscorControl::CrosscorControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new CrosscorControlPrivate;
    d->q = this;

    QGridLayout* glayout = new QGridLayout;
    int row = 0;
    {
        QWidget* X = this->createDoubleControl("cc_max_dt_msec");
        X->setToolTip("Max maximum time offset in milliseconds");
        context->onOptionChanged("cc_max_dt_msec", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Max dt (msec):"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createDoubleControl("cc_bin_size_msec");
        X->setToolTip("Histogram bin size in milliseconds");
        context->onOptionChanged("cc_bin_size_msec", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Bin size (msec):"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    this->setLayout(glayout);

    updateControls();
}

CrosscorControl::~CrosscorControl()
{
    delete d;
}

QString CrosscorControl::title() const
{
    return "Cross-correlograms";
}

void CrosscorControl::updateContext()
{
    SVContext* c = qobject_cast<SVContext*>(mvContext());
    Q_ASSERT(c);
    c->setOption("cc_max_dt_msec", this->controlValue("cc_max_dt_msec").toDouble());
    c->setOption("cc_bin_size_msec", this->controlValue("cc_bin_size_msec").toDouble());
}

void CrosscorControl::updateControls()
{
    SVContext* c = qobject_cast<SVContext*>(mvContext());
    Q_ASSERT(c);

    this->setControlValue("cc_max_dt_msec", c->option("cc_max_dt_msec", 100).toDouble());
    this->setControlValue("cc_bin_size_msec", c->option("cc_bin_size_msec", 1).toDouble());
}
