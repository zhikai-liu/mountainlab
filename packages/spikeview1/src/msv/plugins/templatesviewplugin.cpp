/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "templatesviewplugin.h"
#include "templatesview.h"

#include <QThread>

class TemplatesViewPluginPrivate {
public:
    TemplatesViewPlugin* q;
};

TemplatesViewPlugin::TemplatesViewPlugin()
{
    d = new TemplatesViewPluginPrivate;
    d->q = this;
}

TemplatesViewPlugin::~TemplatesViewPlugin()
{
    delete d;
}

QString TemplatesViewPlugin::name()
{
    return "Templates";
}

QString TemplatesViewPlugin::description()
{
    return "";
}

void TemplatesViewPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new TemplatesViewFactory(mw));
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
    TemplatesView* X = new TemplatesView(context);
    return X;
}

