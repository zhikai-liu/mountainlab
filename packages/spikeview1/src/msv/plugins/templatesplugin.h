/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/
#ifndef TemplatesPlugin_H
#define TemplatesPlugin_H

#include "mvmainwindow.h"

#include <QThread>
#include <svcontext.h>

class TemplatesPluginPrivate;
class TemplatesPlugin : public MVAbstractPlugin {
public:
    friend class TemplatesPluginPrivate;
    TemplatesPlugin();
    virtual ~TemplatesPlugin();

    QString name() Q_DECL_OVERRIDE;
    QString description() Q_DECL_OVERRIDE;
    void initialize(MVMainWindow* mw) Q_DECL_OVERRIDE;

private:
    TemplatesPluginPrivate* d;
};

class TemplatesViewFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    TemplatesViewFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    virtual PreferredOpenLocation preferredOpenLocation() const;
private slots:
    //void openClipsForTemplate();
};

#endif // TemplatesPlugin_H
