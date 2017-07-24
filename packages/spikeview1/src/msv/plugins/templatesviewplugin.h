/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/
#ifndef TemplatesViewPLUGIN_H
#define TemplatesViewPLUGIN_H

#include "mvmainwindow.h"

#include <QThread>
#include <svcontext.h>

class TemplatesViewPluginPrivate;
class TemplatesViewPlugin : public MVAbstractPlugin {
public:
    friend class TemplatesViewPluginPrivate;
    TemplatesViewPlugin();
    virtual ~TemplatesViewPlugin();

    QString name() Q_DECL_OVERRIDE;
    QString description() Q_DECL_OVERRIDE;
    void initialize(MVMainWindow* mw) Q_DECL_OVERRIDE;

private:
    TemplatesViewPluginPrivate* d;
};

class TemplatesViewFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    TemplatesViewFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // TemplatesViewPLUGIN_H
