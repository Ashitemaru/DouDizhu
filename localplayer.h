#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include "struct.h"
#include "card.h"

#include <vector>
#include <string>
#include <QtNetwork/QTcpSocket>

class MainWindow;
class localGameController;

class localPlayer {
public:
    localPlayer(MainWindow*, localGameController*);

    std::vector<card*>& getHandInCard();
    std::vector<int>& getChosenCardLocation();
    std::vector<card*>& getOutdrawCard();

    void setHandInCard(const std::pair<color, number>&);
    void setChosenCard(int);

    void clearHandIn();
    void clearChosen();
    void clearOutDraw();

    cardsCategory judgeOutDraw();
    void outDraw();

private:
    MainWindow* parent;
    localGameController* upper;

    int handInNumber;
    std::vector<card*> handInCard;
    std::vector<int> chosenCardLocation;
    std::vector<card*> outDrawCard;
};

#endif // PLAYER_H
