#ifndef FIRINGEVENTPLUGIN_H
#define FIRINGEVENTPLUGIN_H

#include <mvabstractplugin.h>

class FiringEventPluginPrivate;
class FiringEventPlugin : public MVAbstractPlugin {
public:
    friend class FiringEventPluginPrivate;
    FiringEventPlugin();
    virtual ~FiringEventPlugin();

    QString name() Q_DECL_OVERRIDE;
    QString description() Q_DECL_OVERRIDE;
    void initialize(MVMainWindow* mw) Q_DECL_OVERRIDE;

private:
    FiringEventPluginPrivate* d;
};

class MVFiringEventsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVFiringEventsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
};

#endif // FIRINGEVENTPLUGIN_H
