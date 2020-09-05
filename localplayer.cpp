#include "localplayer.h"
#include "struct.h"
#include "mainwindow.h"
#include "localgamecontroller.h"

#include <QtNetwork/QTcpSocket>
#include <QObject>

#include <vector>

std::vector<int> counter(const std::vector<card*>& c) {
    std::vector<int> count;
    for (int i = 0; i < 15; ++i) count.push_back(0);
    for (auto ptr: c) ++count[ptr->getCardNumber()];
    return count;
}

std::string cardPtrToString(card* c) {
    std::string s = " ";

    if (c->getCardColor() == color::JOKER) s.push_back('J');
    else if (c->getCardColor() == color::SPADE) s.push_back('S');
    else if (c->getCardColor() == color::HEART) s.push_back('H');
    else if (c->getCardColor() == color::CLUB) s.push_back('C');
    else if (c->getCardColor() == color::DIAMOND) s.push_back('D');

    if (c->getCardNumber() == number::_LK) s.push_back('B');
    else if (c->getCardNumber() == number::_BK) s.push_back('R');
    else if (c->getCardNumber() == number::_2) s.push_back('2');
    else if (c->getCardNumber() == number::_A) s.push_back('A');
    else if (c->getCardNumber() == number::_K) s.push_back('K');
    else if (c->getCardNumber() == number::_Q) s.push_back('Q');
    else if (c->getCardNumber() == number::_J) s.push_back('J');
    else if (c->getCardNumber() == number::_10) s.push_back('X');
    else s.push_back('3' + c->getCardNumber());

    return s;
}

std::string numToString(int n) {
    int a = n / 10;
    int b = n % 10;
    std::string s = " ";
    s.push_back('0' + a);
    s.push_back('0' + b);
    return s;
}

bool checkABCDE(std::vector<card*>& cards) {
    bool isABCDE = true;
    for (int i = 0; i < (int)cards.size() - 1; ++i)
        if (cards[i]->getCardNumber() != cards[i + 1]->getCardNumber() + 1) {
            isABCDE = false;
            break;
        }
    if (isABCDE) isABCDE = cards[0]->getCardNumber() < number::_2;
    return isABCDE;
}

bool checkAABBCC(std::vector<card*>& cards) {
    if (cards.size() < 6 || cards.size() % 2 != 0) return false;
    bool isAABBCC = true;
    for (int i = 0; i < (int)cards.size() / 2 - 1; ++i)
        if (
                cards[2 * i]->getCardNumber() != cards[2 * i + 1]->getCardNumber() ||
                cards[2 * i + 1]->getCardNumber() != cards[2 * i + 2]->getCardNumber() + 1 ||
                cards[2 * i + 2]->getCardNumber() != cards[2 * i + 3]->getCardNumber()
           ) {
            isAABBCC = false;
            break;
        }
    if (isAABBCC) isAABBCC = cards[0]->getCardNumber() < number::_2;
    return isAABBCC;
}

bool checkAAABBB(const std::vector<card*>& cards) {
    if (cards.size() < 6 || cards.size() % 3 != 0) return false;
    bool isAAABBB = true;
    for (int i = 0; i < (int)cards.size() / 3 - 1; ++i)
        if (
                cards[3 * i]->getCardNumber() != cards[3 * i + 1]->getCardNumber() ||
                cards[3 * i + 1]->getCardNumber() != cards[3 * i + 2]->getCardNumber() ||
                cards[3 * i + 2]->getCardNumber() != cards[3 * i + 3]->getCardNumber() + 1 ||
                cards[3 * i + 3]->getCardNumber() != cards[3 * i + 4]->getCardNumber() ||
                cards[3 * i + 4]->getCardNumber() != cards[3 * i + 5]->getCardNumber()
           ) {
            isAAABBB = false;
            break;
        }
    if (isAAABBB) isAAABBB = cards[0]->getCardNumber() < number::_2;
    return isAAABBB;
}

bool checkAAAABBCC(const std::vector<card*>& cards) {
    if (cards.size() != 8) return false;
    std::vector<int> count = counter(cards);
    for (int x: count)
        if (x % 2) return false;
    bool isAAAABBCC = false;
    for (int x: count)
        if (x == 4) isAAAABBCC = true;
    return isAAAABBCC;
}

bool checkAAABBBCD(const std::vector<card*>& cards) {
    if (cards.size() < 8 || cards.size() % 4 != 0) return false;
    int n = cards.size() / 4;
    std::vector<int> count = counter(cards);
    for (int i = number::_A; i >= number::_3; --i) {
        if (count[i] < 3) continue;

        bool isRight = true;
        for (int j = 0; j < n; ++j)
            if (count[i - j] < 3) isRight = false;

        if (isRight) return true;
    }
    return false;
}

bool checkAAABBBCCDD(const std::vector<card*>& cards) {
    if (cards.size() < 10 || cards.size() % 5 != 0) return false;
    int n = cards.size() / 5;
    std::vector<int> count = counter(cards);
    for (int i = number::_A; i >= number::_3; --i) {
        if (count[i] < 3) continue;

        bool isRight = true;
        for (int j = 0; j < n; ++j)
            if (count[i - j] < 3) isRight = false;

        if (isRight) {
            bool ok = true;
            for (int k = 0; k < 15; ++k) {
                if (k > i - n && k <= i) ok = count[k] % 2;
                else ok = !(count[k] % 2);
            }
            if (ok) return true;
        }
    }
    return false;
}

localPlayer::localPlayer(MainWindow* p, localGameController* g)
    : parent(p)
    , upper(g) {
    chosenCardLocation.clear();
}

std::vector<card*>& localPlayer::getHandInCard() {
    return handInCard;
}

std::vector<int>& localPlayer::getChosenCardLocation() {
    return chosenCardLocation;
}

std::vector<card*>& localPlayer::getOutdrawCard() {
    return outDrawCard;
}

void localPlayer::setHandInCard(const std::pair<color, number>& c) {
    handInCard.push_back(new card(c.first, c.second, false, parent));
    handInCard[handInCard.size() - 1]->show();
    std::sort(handInCard.begin(), handInCard.end(), [&](card* a, card* b){return !(*a < *b);});
}

void localPlayer::setChosenCard(int pos) {
    std::vector<int>::iterator iter = std::find(chosenCardLocation.begin(), chosenCardLocation.end(), pos);
    if (iter != chosenCardLocation.end()) {
        chosenCardLocation.erase(iter);
    } else {
        chosenCardLocation.push_back(pos);
        std::sort(chosenCardLocation.begin(), chosenCardLocation.end());
    }
}

void localPlayer::clearHandIn() {
    for (auto ptr: handInCard) delete ptr;
    handInCard.clear();
}

void localPlayer::clearChosen() {
    chosenCardLocation.clear();
}

void localPlayer::clearOutDraw() {
    for (auto ptr: outDrawCard) delete ptr;
    outDrawCard.clear();
}

cardsCategory localPlayer::judgeOutDraw() {
    std::vector<int>& position = this->chosenCardLocation;
    std::vector<card*> outDraw;

    for (int x: position) outDraw.push_back(handInCard[x]);

    if (outDraw.size() == 0) return cardsCategory::ILLEGAL;

    if (outDraw.size() == 1) return cardsCategory::A;

    if (outDraw.size() == 2) {
        if (
                *outDraw[0] == card(color::JOKER, number::_BK, false) &&
                *outDraw[1] == card(color::JOKER, number::_LK, false)
           ) return cardsCategory::KK;

        if (outDraw[0]->getCardNumber() == outDraw[1]->getCardNumber()) return cardsCategory::AA;

        return cardsCategory::ILLEGAL;
    }

    if (outDraw.size() == 3) {
        if (
                outDraw[0]->getCardNumber() == outDraw[1]->getCardNumber() &&
                outDraw[1]->getCardNumber() == outDraw[2]->getCardNumber()
           ) return cardsCategory::AAA;

        return cardsCategory::ILLEGAL;
    }

    if (outDraw.size() == 4) {
        if (
                outDraw[0]->getCardNumber() == outDraw[1]->getCardNumber() &&
                outDraw[1]->getCardNumber() == outDraw[2]->getCardNumber() &&
                outDraw[2]->getCardNumber() == outDraw[3]->getCardNumber()
           ) return cardsCategory::AAAA;

        if (
                outDraw[0]->getCardNumber() == outDraw[1]->getCardNumber() &&
                outDraw[1]->getCardNumber() == outDraw[2]->getCardNumber()
           ) return cardsCategory::AAAB;

        if (
                outDraw[1]->getCardNumber() == outDraw[2]->getCardNumber() &&
                outDraw[2]->getCardNumber() == outDraw[3]->getCardNumber()
           ) return cardsCategory::AAAB;

        return cardsCategory::ILLEGAL;
    }

    if (outDraw.size() == 5) {
        if (
                outDraw[0]->getCardNumber() == outDraw[1]->getCardNumber() &&
                outDraw[1]->getCardNumber() == outDraw[2]->getCardNumber() &&
                outDraw[3]->getCardNumber() == outDraw[4]->getCardNumber()
           ) return cardsCategory::AAABB;

        if (
                outDraw[2]->getCardNumber() == outDraw[3]->getCardNumber() &&
                outDraw[3]->getCardNumber() == outDraw[4]->getCardNumber() &&
                outDraw[0]->getCardNumber() == outDraw[1]->getCardNumber()
           ) return cardsCategory::AAABB;
    }

    if (outDraw.size() == 6) {
        if (
                outDraw[0]->getCardNumber() == outDraw[1]->getCardNumber() &&
                outDraw[1]->getCardNumber() == outDraw[2]->getCardNumber() &&
                outDraw[2]->getCardNumber() == outDraw[3]->getCardNumber()
           ) return cardsCategory::AAAABC;

        if (
                outDraw[1]->getCardNumber() == outDraw[2]->getCardNumber() &&
                outDraw[2]->getCardNumber() == outDraw[3]->getCardNumber() &&
                outDraw[3]->getCardNumber() == outDraw[4]->getCardNumber()
           ) return cardsCategory::AAAABC;

        if (
                outDraw[2]->getCardNumber() == outDraw[3]->getCardNumber() &&
                outDraw[3]->getCardNumber() == outDraw[4]->getCardNumber() &&
                outDraw[4]->getCardNumber() == outDraw[5]->getCardNumber()
           ) return cardsCategory::AAAABC;
    }

    if (checkABCDE(outDraw)) return cardsCategory::ABCDE;
    if (checkAABBCC(outDraw)) return cardsCategory::AABBCC;
    if (checkAAABBB(outDraw)) return cardsCategory::AAABBB;
    if (checkAAAABBCC(outDraw)) return cardsCategory::AAAABBCC;
    if (checkAAABBBCD(outDraw)) return cardsCategory::AAABBBCD;
    if (checkAAABBBCCDD(outDraw)) return cardsCategory::AAABBBCCDD;

    return cardsCategory::ILLEGAL;
}

void localPlayer::outDraw() {
    cardsCategory c = judgeOutDraw();
    if (c != cardsCategory::ILLEGAL) {
        std::string info = "drop ";

        if (c == cardsCategory::A) info += "Dan";
        else if (c == cardsCategory::AA) info += "Dui";
        else if (c == cardsCategory::AAA || c == cardsCategory::AAAB || c == cardsCategory::AAABB) info += "SanDai";
        else if (c == cardsCategory::ABCDE) info += "Shun";
        else if (c == cardsCategory::AABBCC) info += "ShuangShun";
        else if (c == cardsCategory::AAABBB) info += "SanShun";
        else if (c == cardsCategory::AAABBBCD) info += "DanFeiJi";
        else if (c == cardsCategory::AAABBBCCDD) info += "DuiFeiJi";
        else if (c == cardsCategory::AAAABC || c == cardsCategory::AAAABBCC) info += "SiDai";
        else if (c == cardsCategory::AAAA) info += "ZhaDan";

        info += numToString(chosenCardLocation.size());

        for (int i = 0; i < (int)chosenCardLocation.size(); ++i) {
            info += cardPtrToString(handInCard[chosenCardLocation[i] - i]);
            outDrawCard.push_back(handInCard[chosenCardLocation[i] - i]);
            handInCard.erase(handInCard.begin() + chosenCardLocation[i] - i);
        }
        chosenCardLocation.clear();
        upper->postMessage(QString(info.c_str()));
    }
}
