/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "crosscorplugin.h"
#include "templatesview.h"

#include <QThread>
#include <crosscorview.h>
#include <templatescontrol.h>

class CrosscorPluginPrivate {
public:
    CrosscorPlugin* q;
};

CrosscorPlugin::CrosscorPlugin()
{
    d = new CrosscorPluginPrivate;
    d->q = this;
}

CrosscorPlugin::~CrosscorPlugin()
{
    delete d;
}

QString CrosscorPlugin::name()
{
    return "Crosscor";
}

QString CrosscorPlugin::description()
{
    return "";
}

void CrosscorPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new AutocorViewFactory(mw));
    //mw->addControl(new CrosscorControl(mw->mvContext(),mw), true);
}

AutocorViewFactory::AutocorViewFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString AutocorViewFactory::id() const
{
    return QStringLiteral("open-autocor-view");
}

QString AutocorViewFactory::name() const
{
    return tr("Autocor");
}

QString AutocorViewFactory::title() const
{
    return tr("Autocor");
}

MVAbstractView* AutocorViewFactory::createView(MVAbstractContext* context)
{
    SVContext* c = qobject_cast<SVContext*>(context);
    Q_ASSERT(c);

    QList<int> keys=c->clusterAttributesKeys();
    QList<int> k1s,k2s;
    for (int i=0; i<keys.count(); i++) {
        k1s << keys[i];
        k2s << keys[i];
    }

    CrosscorView* X = new CrosscorView(context);
    X->setKs(k1s,k2s,k1s);

    return X;
}

