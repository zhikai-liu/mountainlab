/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/
#ifndef SSLVClusterMetricsPLUGIN_H
#define SSLVClusterMetricsPLUGIN_H

#include "mvmainwindow.h"

#include <QThread>

class SSLVClusterMetricsPluginPrivate;
class SSLVClusterMetricsPlugin : public MVAbstractPlugin {
public:
    friend class SSLVClusterMetricsPluginPrivate;
    SSLVClusterMetricsPlugin();
    virtual ~SSLVClusterMetricsPlugin();

    QString name() Q_DECL_OVERRIDE;
    QString description() Q_DECL_OVERRIDE;
    void initialize(MVMainWindow* mw) Q_DECL_OVERRIDE;

private:
    SSLVClusterMetricsPluginPrivate* d;
};

class SSLVClusterMetricsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    SSLVClusterMetricsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // SSLVClusterMetricsPLUGIN_H
