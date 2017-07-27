/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "sslvtimespanplugin.h"

#include "sslvtimespanview.h"

#include <QThread>
#include <mountainprocessrunner.h>
#include <sslvcontext.h>
#include <QMessageBox>

class SSLVTimeSpanPluginPrivate {
public:
    SSLVTimeSpanPlugin* q;
};

SSLVTimeSpanPlugin::SSLVTimeSpanPlugin()
{
    d = new SSLVTimeSpanPluginPrivate;
    d->q = this;
}

SSLVTimeSpanPlugin::~SSLVTimeSpanPlugin()
{
    delete d;
}

QString SSLVTimeSpanPlugin::name()
{
    return "SSLVTimeSpan";
}

QString SSLVTimeSpanPlugin::description()
{
    return "";
}

void compute_basic_metrics(MVAbstractContext* mv_context);
void SSLVTimeSpanPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new SSLVTimeSpanFactory(mw));
}

SSLVTimeSpanFactory::SSLVTimeSpanFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString SSLVTimeSpanFactory::id() const
{
    return QStringLiteral("open-time-span");
}

QString SSLVTimeSpanFactory::name() const
{
    return tr("Timespans");
}

QString SSLVTimeSpanFactory::title() const
{
    return tr("Timespans");
}

MVAbstractView* SSLVTimeSpanFactory::createView(MVAbstractContext* context)
{
    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    QList<int> ks = c->selectedClusters();
    if (ks.isEmpty())
        ks = c->getAllClusterNumbers();
    if (ks.isEmpty()) {
        QMessageBox::warning(0, "Unable to open time span view", "You must select at least one cluster.");
        return Q_NULLPTR;
    }
    SSLVTimeSpanView* X = new SSLVTimeSpanView(c);
    X->setClustersToShow(ks);
    X->setNumTimepoints(c->timeseries().N2());
    return X;
}
