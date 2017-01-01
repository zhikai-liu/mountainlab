/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "timeseriesplugin.h"

#include "timeseriesview.h"

#include <QThread>
#include <mountainprocessrunner.h>

class TimeseriesPluginPrivate {
public:
    TimeseriesPlugin* q;
};

TimeseriesPlugin::TimeseriesPlugin()
{
    d = new TimeseriesPluginPrivate;
    d->q = this;
}

TimeseriesPlugin::~TimeseriesPlugin()
{
    delete d;
}

QString TimeseriesPlugin::name()
{
    return "Timeseries";
}

QString TimeseriesPlugin::description()
{
    return "";
}

void compute_basic_metrics(MVAbstractContext* mv_context);
void TimeseriesPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new TimeseriesFactory(mw));
}

TimeseriesFactory::TimeseriesFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString TimeseriesFactory::id() const
{
    return QStringLiteral("open-timeseries");
}

QString TimeseriesFactory::name() const
{
    return tr("Timeseries");
}

QString TimeseriesFactory::title() const
{
    return tr("Timeseries");
}

MVAbstractView* TimeseriesFactory::createView(MVAbstractContext* context)
{
    TimeseriesView* X = new TimeseriesView(context);
    return X;
}
