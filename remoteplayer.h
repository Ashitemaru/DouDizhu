#ifndef REMOTEPLAYER_H
#define REMOTEPLAYER_H

#include "struct.h"
#include "card.h"

#include <vector>

class MainWindow;

class remotePlayer {
public:
    remotePlayer(MainWindow*);

    std::vector<card*>& getHandInCard();
    std::vector<card*>& getOutDrawCard();

    void setHandInCard(int);
    void setOutDrawCard(const std::pair<color, number>&);

    void clearOutDraw();

private:
    MainWindow* parent;

    std::vector<card*> handInCard;
    std::vector<card*> outDrawCard;
};

#endif // REMOTEPLAYER_H
