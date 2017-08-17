#ifndef PROCESS_REQUEST_H
#define PROCESS_REQUEST_H

#include <QJsonObject>

QJsonObject handle_request(const QJsonObject& request, QString prvbucket_path);

#endif // PROCESS_REQUEST_H
