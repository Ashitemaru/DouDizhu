#ifndef LOCALGAMECONTROLLER_H
#define LOCALGAMECONTROLLER_H

#include "localplayer.h"
#include "remoteplayer.h"
#include "struct.h"
#include "card.h"

#include <QString>

#include <vector>

class MainWindow;
class server;

class localGameController {
public:
    explicit localGameController(localType, MainWindow*);

    localPlayer* getLocalPlayer();
    remotePlayer* getFirstRemotePlayer();
    remotePlayer* getSecondRemotePlayer();
    gameState getState();
    players getLandLord();
    players getNowPlayer();
    std::vector<card*>& getLandlordCard();
    bool getHasCalled();

    void setState(gameState);
    void setLandlordCard(const std::vector<std::pair<color, number> >&);
    void setHasCalled(bool);

    void flipLandlordCard();

    void initConnection();
    void getMessage();
    void postMessage(const QString&);

    void nextPlayer();

    void analyzeServerInfo(const QString&);

    void clearLandlordCard();

    bool chosenIsGreater();

private:
    MainWindow* parent;

    gameState state;

    localPlayer* local;
    remotePlayer* firstRemote;
    remotePlayer* secondRemote;

    drop prev;

    players landlord;

    players nowPlayer;

    server* coreServer;

    std::vector<card*> landlordCard;

    QTcpSocket* readWriteSocket;

    bool hasCalled;
};

#endif // LOCALGAMECONTROLLER_H
