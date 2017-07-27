#include "jsonmerger.h"
#include <QJsonObject>
#include <QJsonValue>

#include <QtDebug>

JsonMerger::JsonMerger(QObject* parent)
    : QObject(parent)
{
}

QString JsonMerger::defaultJson() const
{
    return m_default.toJson();
}

QString JsonMerger::userJson() const
{
    return m_user.toJson();
}

QString JsonMerger::resultJson() const
{
    return m_result.toJson();
}

void JsonMerger::setDefaultJson(const QString& contents)
{
    QJsonDocument doc = parseJson(contents);
    if (doc.isNull())
        return;
    m_default = doc;
    merge();
}

void JsonMerger::setUserJson(const QString& contents)
{
    QJsonDocument doc = parseJson(contents);
    if (doc.isNull())
        return;
    m_user = doc;
    merge();
}

QJsonDocument JsonMerger::parseJson(const QString& contents)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(contents.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Error while parsing JSON: " << err.errorString();
        emit parsingFailed(err.errorString());
        return QJsonDocument();
    }
    return doc;
}

void JsonMerger::merge()
{
    m_result = m_default;
    QJsonObject resultObj = m_result.object();
    QJsonObject userObj = m_user.object();
    for (const QString& group : userObj.keys()) {
        QJsonObject resultGroupObj = resultObj[group].toObject();
        QJsonObject userGroupObj = userObj[group].toObject();
        for (const QString& value : userGroupObj.keys()) {
            resultGroupObj[value] = userGroupObj[value];
        }
        resultObj[group] = resultGroupObj;
    }
    m_result.setObject(resultObj);
    emit resultChanged(QString::fromUtf8(m_result.toJson()));
}
