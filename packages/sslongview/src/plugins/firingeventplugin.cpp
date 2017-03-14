#include "firingeventplugin.h"

#include <mvmainwindow.h>
#include <sslvcontext.h>
#include <QMessageBox>
#include <mvfiringeventview2.h>

class FiringEventPluginPrivate {
public:
    FiringEventPlugin* q;
};

FiringEventPlugin::FiringEventPlugin()
{
    d = new FiringEventPluginPrivate;
    d->q = this;
}

FiringEventPlugin::~FiringEventPlugin()
{
    delete d;
}

QString FiringEventPlugin::name()
{
    return "FiringEvent";
}

QString FiringEventPlugin::description()
{
    return "";
}

void FiringEventPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new MVFiringEventsFactory(mw));
}

MVFiringEventsFactory::MVFiringEventsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVFiringEventsFactory::id() const
{
    return QStringLiteral("open-firing-events");
}

QString MVFiringEventsFactory::name() const
{
    return tr("Firing Events");
}

QString MVFiringEventsFactory::title() const
{
    return tr("Firing Events");
}

MVAbstractView* MVFiringEventsFactory::createView(MVAbstractContext* context)
{
    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    QList<int> ks = c->selectedClusters();
    //if (ks.isEmpty())
    //    ks = c->clusterVisibilityRule().subset.toList();
    if (ks.isEmpty()) {
        QMessageBox::warning(0, "Unable to open firing events", "You must select at least one cluster.");
        return Q_NULLPTR;
    }
    MVFiringEventView2* X = new MVFiringEventView2(c);
    X->setLabelsToUse(ks.toSet());
    X->setNumTimepoints(c->timeseries().N2());
    return X;
}
