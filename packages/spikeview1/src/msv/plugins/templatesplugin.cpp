/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "templatesplugin.h"
#include "templatesview.h"

#include <QMessageBox>
#include <QThread>
#include <templatescontrol.h>

class TemplatesPluginPrivate {
public:
    TemplatesPlugin* q;
};

TemplatesPlugin::TemplatesPlugin()
{
    d = new TemplatesPluginPrivate;
    d->q = this;
}

TemplatesPlugin::~TemplatesPlugin()
{
    delete d;
}

QString TemplatesPlugin::name()
{
    return "Templates";
}

QString TemplatesPlugin::description()
{
    return "";
}

void TemplatesPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new TemplatesViewFactory(mw));
    mw->addControl(new TemplatesControl(mw->mvContext(), mw), true);
}

TemplatesViewFactory::TemplatesViewFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString TemplatesViewFactory::id() const
{
    return QStringLiteral("open-templates-view");
}

QString TemplatesViewFactory::name() const
{
    return tr("Templates");
}

QString TemplatesViewFactory::title() const
{
    return tr("Templates");
}

MVAbstractView* TemplatesViewFactory::createView(MVAbstractContext* context)
{
    SVContext* c = qobject_cast<SVContext*>(context);
    Q_ASSERT(c);

    //bool all_mode=false;

    QList<int> keys = c->selectedClusters();
    //if ((keys.isEmpty()) || (all_mode))
    //    keys = c->clusterAttributesKeys();

    if (keys.isEmpty()) {
        QMessageBox::information(0, "Please select one or more clusters", "You must first select one or more clusters");
        return 0;
    }

    QList<int> ks;
    for (int i = 0; i < keys.count(); i++) {
        ks << keys[i];
    }

    TemplatesView* X = new TemplatesView(context);
    X->setKs(ks);

    return X;
}

MVAbstractViewFactory::PreferredOpenLocation TemplatesViewFactory::preferredOpenLocation() const
{
    return MVAbstractViewFactory::North;
}
