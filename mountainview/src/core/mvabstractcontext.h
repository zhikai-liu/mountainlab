#ifndef MVABSTRACTCONTEXT_H
#define MVABSTRACTCONTEXT_H

#include <QObject>
#include <QVariant>

class MVAbstractContextPrivate;
class MVAbstractContext : public QObject {
    Q_OBJECT
public:
    friend class MVAbstractContextPrivate;
    MVAbstractContext();
    virtual ~MVAbstractContext();

    virtual void setFromMV2FileObject(QJsonObject obj) = 0;
    virtual QJsonObject toMV2FileObject() const = 0;

    void setMV2FileName(QString fname);
    QString mv2FileName();

    /////////////////////////////////////////////////
    QVariant option(QString name, QVariant default_val = QVariant()) const;
    void setOption(QString name, QVariant value);
    void onOptionChanged(QString name, const QObject* receiver, const char* member, Qt::ConnectionType type = Qt::DirectConnection);
    QVariantMap options() const;
    void setOptions(const QVariantMap& options);
    void clearOptions();

private slots:
    void slot_option_changed(QString name);

signals:
    void optionChanged(QString name);

private:
    MVAbstractContextPrivate* d;
};

#endif // MVABSTRACTCONTEXT_H
