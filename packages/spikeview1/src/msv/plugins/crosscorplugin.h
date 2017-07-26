/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/
#ifndef CrosscorPlugin_H
#define CrosscorPlugin_H

#include "mvmainwindow.h"

#include <QThread>
#include <svcontext.h>

class CrosscorPluginPrivate;
class CrosscorPlugin : public MVAbstractPlugin {
public:
    friend class CrosscorPluginPrivate;
    CrosscorPlugin();
    virtual ~CrosscorPlugin();

    QString name() Q_DECL_OVERRIDE;
    QString description() Q_DECL_OVERRIDE;
    void initialize(MVMainWindow* mw) Q_DECL_OVERRIDE;

private:
    CrosscorPluginPrivate* d;
};

class AutocorViewFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    AutocorViewFactory(MVMainWindow* mw, QObject* parent = 0, bool all_mode = false);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;

private:
    bool m_all_mode = false;
private slots:
};

class CrosscorMatrixViewFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    CrosscorMatrixViewFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // CrosscorPlugin_H
