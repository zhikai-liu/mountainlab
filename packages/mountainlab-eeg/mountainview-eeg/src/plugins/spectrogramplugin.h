/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/
#ifndef SpectrogramPLUGIN_H
#define SpectrogramPLUGIN_H

#include "mvmainwindow.h"

#include <QThread>

class SpectrogramPluginPrivate;
class SpectrogramPlugin : public MVAbstractPlugin {
public:
    friend class SpectrogramPluginPrivate;
    SpectrogramPlugin();
    virtual ~SpectrogramPlugin();

    QString name() Q_DECL_OVERRIDE;
    QString description() Q_DECL_OVERRIDE;
    void initialize(MVMainWindow* mw) Q_DECL_OVERRIDE;

private:
    SpectrogramPluginPrivate* d;
};

class SpectrogramFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    SpectrogramFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // SpectrogramPLUGIN_H
