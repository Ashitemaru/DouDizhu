#include "remoteplayer.h"
#include "struct.h"
#include "card.h"
#include "mainwindow.h"

remotePlayer::remotePlayer(MainWindow* p) {
    parent = p;
}

std::vector<card*>& remotePlayer::getHandInCard() {
    return handInCard;
}

std::vector<card*>& remotePlayer::getOutDrawCard() {
    return outDrawCard;
}

void remotePlayer::setHandInCard(int n) {
    if (n == (int)handInCard.size()) return;
    if (n > (int)handInCard.size()) {
        int size = handInCard.size();
        for (int i = 0; i < n - size; ++i) handInCard.push_back(new card(color::JOKER, number::_BK, true, parent));
    } else {
        for (int i = n; i < (int)handInCard.size(); ++i) delete handInCard[i];
        handInCard.resize(n);
    }
}

void remotePlayer::setOutDrawCard(const std::pair<color, number>& c) {
    outDrawCard.push_back(new card(c.first, c.second, false, parent));
    outDrawCard[outDrawCard.size() - 1]->show();
    std::sort(outDrawCard.begin(), outDrawCard.end(), [&](card* a, card* b){return !(*a < *b);});
}

void remotePlayer::clearOutDraw() {
    for (auto ptr: outDrawCard) delete ptr;
    outDrawCard.clear();
}
