#include "templatescontrol.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>
#include <svcontext.h>
#include "mlcommon.h"

class TemplatesControlPrivate {
public:
    TemplatesControl* q;
};

TemplatesControl::TemplatesControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new TemplatesControlPrivate;
    d->q = this;

    QGridLayout* glayout = new QGridLayout;
    int row = 0;
    {
        QWidget* X = this->createDoubleControl("templates_clip_size");
        X->setToolTip("Clip size (timepoints)");
        context->onOptionChanged("templates_clip_size", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Clip size:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget *X = this->createCheckBoxControl("templates_apply_filter","Apply bandpass filter to templates");
        X->setToolTip("Apply bandpass filter to templates");
        context->onOptionChanged("templates_apply_filter", this, SLOT(updateControls()));
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createDoubleControl("templates_freq_min");
        X->setToolTip("Minimum frequency (Hz) for bandpass filter");
        context->onOptionChanged("templates_freq_min", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Freq. min:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createDoubleControl("templates_freq_max");
        X->setToolTip("Maximum frequency (Hz) for bandpass filter");
        context->onOptionChanged("templates_freq_max", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Freq. max:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget *X = this->createCheckBoxControl("templates_subtract_temporal_mean","Subtract temporal mean from templates");
        X->setToolTip("Apply bandpass filter to templates");
        context->onOptionChanged("templates_subtract_temporal_mean", this, SLOT(updateControls()));
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createDoubleControl("templates_max_events_per_template");
        X->setToolTip("Maximum number of events used to compute each template. Use 0 for unlimited.");
        context->onOptionChanged("templates_max_events_per_template", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Max. events per template:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    this->setLayout(glayout);

    updateControls();
}

TemplatesControl::~TemplatesControl()
{
    delete d;
}

QString TemplatesControl::title() const
{
    return "Templates";
}

void TemplatesControl::updateContext()
{
    SVContext* c = qobject_cast<SVContext*>(mvContext());
    Q_ASSERT(c);

    c->setOption("templates_clip_size",this->controlValue("templates_clip_size").toInt());
    c->setOption("templates_apply_filter",this->controlValue("templates_apply_filter").toBool() ? "true" : "false");
    c->setOption("templates_freq_min",this->controlValue("templates_freq_min").toDouble());
    c->setOption("templates_freq_max",this->controlValue("templates_freq_max").toDouble());
    c->setOption("templates_subtract_temporal_mean",this->controlValue("templates_subtract_temporal_mean").toBool() ? "true" : "false");
    c->setOption("templates_max_events_per_template",this->controlValue("templates_max_events_per_template").toDouble());
}

void TemplatesControl::updateControls()
{
    SVContext* c = qobject_cast<SVContext*>(mvContext());
    Q_ASSERT(c);

    this->setControlValue("templates_clip_size",c->option("templates_clip_size",100).toInt());
    this->setControlValue("templates_apply_filter",(c->option("templates_apply_filter","false")=="true"));
    this->setControlValue("templates_freq_min",c->option("templates_freq_min",300).toDouble());
    this->setControlValue("templates_freq_max",c->option("templates_freq_max",6000).toDouble());
    this->setControlValue("templates_subtract_temporal_mean",(c->option("templates_subtract_temporal_mean","true")=="true"));
    this->setControlValue("templates_max_events_per_template",c->option("templates_max_events_per_template",500).toDouble());
}
