#include "localserver.h"

#include <QFile>

namespace LocalServer {
Server::Server(QObject* parent)
    : QObject(parent)
{
    m_socket = new QLocalServer(this);
    m_socket->setSocketOptions(QLocalServer::WorldAccessOption);
    connect(m_socket, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
}

bool Server::listen(const QString& path)
{
    QLocalServer::removeServer(path);
    bool res = socket()->listen(path);
    if (res) {
        QFile socketFile(socket()->fullServerName());
        socketFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
    }
    return res;
}

void Server::shutdown()
{
    socket()->close();
    foreach (Client* c, m_clients) {
        c->disconnect(this);
        c->close();
        c->deleteLater();
    }
    m_clients.clear();
}

Client::Client(QLocalSocket* sock, LocalServer::Server* parent)
    : QObject(parent)
    , m_socket(sock)
    , m_server(parent)
{
    connect(socket(), SIGNAL(disconnected()), this, SLOT(handleDisconnected()));
    connect(socket(), SIGNAL(readyRead()), this, SLOT(readData()));
}

void Client::handleDisconnected()
{
    //server()->handleClientDisconnected(this);
    emit disconnected();
}

void Client::readData()
{
    m_buffer.append(socket()->readAll());
    messageLoop();
}
}
