#ifndef SSLVCONTEXT_H
#define SSLVCONTEXT_H

#include "mvabstractcontext.h"

#include <QColor>
#include <diskreadmda.h>
#include <diskreadmda32.h>

class SSLVContextPrivate;
class SSLVContext : public MVAbstractContext {
    Q_OBJECT
public:
    friend class SSLVContextPrivate;
    SSLVContext();
    virtual ~SSLVContext();

    void clear();

    void setSampleRate(double rate);
    double sampleRate() const;

    /////////////////////////////////////////////////
    DiskReadMda32 timeseries() const;
    void setTimeseries(const DiskReadMda32& X);

    /////////////////////////////////////////////////
    DiskReadMda firings() const;
    void setFirings(const DiskReadMda& X);

    /////////////////////////////////////////////////
    QJsonObject clusterMetrics() const;
    void setClusterMetrics(const QJsonObject& obj);

    /////////////////////////////////////////////////
    double currentTimepoint() const;
    MVRange currentTimeRange() const;
    void setCurrentTimepoint(double tp);
    void setCurrentTimeRange(const MVRange& range);

    /////////////////////////////////////////////////
    int currentCluster() const;
    QList<int> selectedClusters() const;
    void setCurrentCluster(int k);
    void setSelectedClusters(const QList<int>& ks);
    void clickCluster(int k, Qt::KeyboardModifiers modifiers);
    bool clusterIsVisible(int k);
    QList<int> getAllClusterNumbers() const;

    /////////////////////////////////////////////////
    QColor clusterColor(int k) const;
    QColor channelColor(int m) const;
    QColor color(QString name, QColor default_color = Qt::black) const;
    QMap<QString, QColor> colors() const;
    QList<QColor> channelColors() const;
    QList<QColor> clusterColors() const;
    void setClusterColors(const QList<QColor>& colors);
    void setChannelColors(const QList<QColor>& colors);
    void setColors(const QMap<QString, QColor>& colors);

    double maxTimepoint() const;

    void setFromMV2FileObject(QJsonObject obj) Q_DECL_OVERRIDE;
    QJsonObject toMV2FileObject() const Q_DECL_OVERRIDE;

signals:
    void timeseriesChanged();
    void firingsChanged();
    void currentTimepointChanged();
    void currentTimeRangeChanged();
    void clusterMetricsChanged();
    void currentClusterChanged();
    void selectedClustersChanged();
    void clusterColorsChanged(QList<QColor> colors);

private:
    SSLVContextPrivate* d;
};

#endif // SSLVCONTEXT_H
