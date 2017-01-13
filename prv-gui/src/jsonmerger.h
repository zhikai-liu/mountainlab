#ifndef JSONMERGER_H
#define JSONMERGER_H

#include <QObject>
#include <QJsonDocument>

class JsonMerger : public QObject
{
    Q_OBJECT
public:
    JsonMerger(QObject *parent = 0);

    QString defaultJson() const;
    QString userJson() const;
    QString resultJson() const;

public slots:
    void setDefaultJson(const QString &contents);
    void setUserJson(const QString &contents);

signals:
    void resultChanged(const QString &result);
    void parsingFailed(const QString &error);

private:
    QJsonDocument parseJson(const QString &contents);
    void merge();

private:
    QJsonDocument m_default;
    QJsonDocument m_user;
    QJsonDocument m_result;
};

#endif // JSONMERGER_H
