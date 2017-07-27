/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/
#ifndef TimeseriesPLUGIN_H
#define TimeseriesPLUGIN_H

#include "mvmainwindow.h"

#include <QThread>

class TimeseriesPluginPrivate;
class TimeseriesPlugin : public MVAbstractPlugin {
public:
    friend class TimeseriesPluginPrivate;
    TimeseriesPlugin();
    virtual ~TimeseriesPlugin();

    QString name() Q_DECL_OVERRIDE;
    QString description() Q_DECL_OVERRIDE;
    void initialize(MVMainWindow* mw) Q_DECL_OVERRIDE;

private:
    TimeseriesPluginPrivate* d;
};

class TimeseriesFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    TimeseriesFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // TimeseriesPLUGIN_H
