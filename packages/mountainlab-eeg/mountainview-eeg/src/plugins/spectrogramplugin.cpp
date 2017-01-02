/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "spectrogramplugin.h"

#include "spectrogramview.h"

#include <QThread>
#include <mountainprocessrunner.h>

class SpectrogramPluginPrivate {
public:
    SpectrogramPlugin* q;
};

SpectrogramPlugin::SpectrogramPlugin()
{
    d = new SpectrogramPluginPrivate;
    d->q = this;
}

SpectrogramPlugin::~SpectrogramPlugin()
{
    delete d;
}

QString SpectrogramPlugin::name()
{
    return "Spectrogram";
}

QString SpectrogramPlugin::description()
{
    return "";
}

void compute_basic_metrics(MVAbstractContext* mv_context);
void SpectrogramPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new SpectrogramFactory(mw));
}

SpectrogramFactory::SpectrogramFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString SpectrogramFactory::id() const
{
    return QStringLiteral("open-Spectrogram");
}

QString SpectrogramFactory::name() const
{
    return tr("Spectrogram");
}

QString SpectrogramFactory::title() const
{
    return tr("Spectrogram");
}

MVAbstractView* SpectrogramFactory::createView(MVAbstractContext* context)
{
    SpectrogramView* X = new SpectrogramView(context);
    return X;
}
