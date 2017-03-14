/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/
#ifndef CORRELOGRAMPLUGIN_H
#define CORRELOGRAMPLUGIN_H

#include "mvmainwindow.h"

class CorrelogramPluginPrivate;
class CorrelogramPlugin : public MVAbstractPlugin {
public:
    friend class CorrelogramPluginPrivate;
    CorrelogramPlugin();
    virtual ~CorrelogramPlugin();

    QString name() Q_DECL_OVERRIDE;
    QString description() Q_DECL_OVERRIDE;
    void initialize(MVMainWindow* mw) Q_DECL_OVERRIDE;

private:
    CorrelogramPluginPrivate* d;
};

class AutoCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    AutoCorrelogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
private slots:
};

class SelectedAutoCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    SelectedAutoCorrelogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
};

class CrossCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    CrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
};

class MatrixOfCrossCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MatrixOfCrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
};

class SelectedCrossCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    SelectedCrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
};

#endif // CORRELOGRAMPLUGIN_H
