#ifndef SERVER_H
#define SERVER_H

#include "card.h"
#include "localgamecontroller.h"

#include <QObject>
#include <QByteArray>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

class localGameController;

class server: public QObject {

    Q_OBJECT

public:
    explicit server(localGameController*, QObject* parent = nullptr);

    void initServer();

public:
    void acceptConnection();

    void firstRecvMessage();
    void secondRecvMessage();
    void analyzeClientInfo(players, const QString&);

    void postMessage(players, const QString&);

    void startNewGame();

    void handleRestart();

    void clearServer();

    localGameController* localUpper;

    QTcpServer* listenSocket;

    QTcpSocket* firstReadWriteSocket;
    QTcpSocket* secondReadWriteSocket;

private:
    int remotePlayerNum;
    int demandNum;
    players prevPlayer;
    int totalRepost;
    int restartRepost;

    players landlord;

    std::pair<color, number> land_1;
    std::pair<color, number> land_2;
    std::pair<color, number> land_3;

};

#endif // SERVER_H
