#include "localgamecontroller.h"
#include "remoteplayer.h"
#include "localplayer.h"
#include "mainwindow.h"
#include "server.h"

#include <QString>
#include <QStringList>
#include <QtNetwork>
#include <QDebug>
#include <QLabel>

#include <iostream>

inline number toNumber(char ch) {
    switch (ch) {
        case '3': return number::_3;
        case '4': return number::_4;
        case '5': return number::_5;
        case '6': return number::_6;
        case '7': return number::_7;
        case '8': return number::_8;
        case '9': return number::_9;
        case 'X': return number::_10;
        case 'J': return number::_J;
        case 'Q': return number::_Q;
        case 'K': return number::_K;
        case 'A': return number::_A;
        case '2': return number::_2;
        case 'B': return number::_LK;
        case 'R': return number::_BK;
        default: return number::_BK;
    }
}

std::vector<int> counter(const QStringList& l) {
    std::vector<int> count;
    for (int i = 0; i < 15; ++i) count.push_back(0);
    for (int i = 4; i < l.size(); ++i) ++count[toNumber(l[i].toStdString()[2])];
    return count;
}

number extract(const QStringList& l, cardsCategory c) {
    if (c == cardsCategory::AAABBBCD) {
        int n = (l.size() - 4) / 4;
        std::vector<int> count = counter(l);
        for (int i = number::_A; i >= number::_3; --i) {
            if (count[i] < 3) continue;

            bool isRight = true;
            for (int j = 0; j < n; ++j)
                if (count[i - j] < 3) isRight = false;

            if (isRight) return number(i);
        }
        return number::_2;
    } else if (c == cardsCategory::AAABBBCCDD) {
        int n = (l.size() - 4) / 5;
        std::vector<int> count = counter(l);
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
                if (ok) return number(i);
            }
        }
        return number::_2;
    } else return number::_2;
}

std::string cardPtrToString_(card* c) {
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


std::pair<color, number> stringToCard(const QString& src) {
    color c = color::JOKER;
    number n = number::_BK;

    if (src[0] == 'H') c = color::HEART;
    else if (src[0] == 'D') c = color::DIAMOND;
    else if (src[0] == 'S') c = color::SPADE;
    else if (src[0] == 'C') c = color::CLUB;

    if (src[1] == 'B') n = number::_LK;
    else if (src[1] == 'R') n = number::_BK;
    else if (src[1] == '2') n = number::_2;
    else if (src[1] == 'A') n = number::_A;
    else if (src[1] == 'K') n = number::_K;
    else if (src[1] == 'Q') n = number::_Q;
    else if (src[1] == 'J') n = number::_J;
    else if (src[1] == 'X') n = number::_10;
    else n = (number)((int)src.toStdString()[1] - (int)'3');

    return std::make_pair(c, n);
}

localGameController::localGameController(localType t, MainWindow* p)
    : parent(p)
    , state(gameState::READY_FOR_CONNECTION)
    , local(new localPlayer(p, this))
    , firstRemote(new remotePlayer(p))
    , secondRemote(new remotePlayer(p))
    , coreServer(t == localType::SERVER ? new server(this) : nullptr)
    , readWriteSocket(t == localType::CLIENT ? new QTcpSocket : nullptr)
    , hasCalled(false) {

    if (t == localType::CLIENT) initConnection();
    else coreServer->initServer();

    landlordCard.push_back(new card(color::JOKER, number::_BK, true, p));
    landlordCard.push_back(new card(color::JOKER, number::_BK, true, p));
    landlordCard.push_back(new card(color::JOKER, number::_BK, true, p));

    prev.any = true;

}

localPlayer* localGameController::getLocalPlayer() {
    return local;
}

remotePlayer* localGameController::getFirstRemotePlayer() {
    return firstRemote;
}

remotePlayer* localGameController::getSecondRemotePlayer() {
    return secondRemote;
}

gameState localGameController::getState() {
    return state;
}

players localGameController::getLandLord() {
    return landlord;
}

players localGameController::getNowPlayer() {
    return nowPlayer;
}

std::vector<card*>& localGameController::getLandlordCard() {
    return landlordCard;
}

bool localGameController::getHasCalled() {
    return hasCalled;
}

void localGameController::setState(gameState s) {
    state = s;
}

void localGameController::setLandlordCard(const std::vector<std::pair<color, number> >& s) {
    for (auto x: s) landlordCard.push_back(new card(x.first, x.second, false, parent));
}

void localGameController::setHasCalled(bool b) {
    hasCalled = b;
}

void localGameController::flipLandlordCard() {
    for (int i = 0; i < 3; ++i) delete landlordCard[i];

    landlordCard.erase(landlordCard.begin());
    landlordCard.erase(landlordCard.begin());
    landlordCard.erase(landlordCard.begin());
}

void localGameController::initConnection() {
    this->readWriteSocket->connectToHost(QHostAddress("127.0.0.1"), 1926);
    QObject::connect(this->readWriteSocket, &QTcpSocket::readyRead, [&](){this->getMessage();});
}

void localGameController::getMessage() {
    QByteArray data = readWriteSocket->read(4);

    int length = data.toHex().toUInt(nullptr, 16);
    analyzeServerInfo(readWriteSocket->read(length));

    if(!readWriteSocket->atEnd())
        getMessage();
}

void localGameController::postMessage(const QString& msg) {
    if (readWriteSocket != nullptr) {
        int size = qToBigEndian(msg.size());
        this->readWriteSocket->write((char*)&size, 4);
        this->readWriteSocket->write(msg.toLatin1());
    } else {
        coreServer->analyzeClientInfo(players::LOCAL_CLIENT, msg);
    }
}

void localGameController::nextPlayer() {
    if (nowPlayer == players::LOCAL_CLIENT) nowPlayer = players::FIRST_REMOTE;
    else if (nowPlayer == players::FIRST_REMOTE) nowPlayer = players::SECOND_REMOTE;
    else nowPlayer = players::LOCAL_CLIENT;
}

void localGameController::analyzeServerInfo(const QString& info) {

    // qDebug() << "CLIENT: " << info << Qt::endl;

    QStringList list = info.split(' ');

    if (list[0] == "begin") {
        this->parent->repaint();
    }

    else if (list[0] == "deal") {
        this->local->setHandInCard(stringToCard(list[1]));
        this->firstRemote->setHandInCard(17);
        this->secondRemote->setHandInCard(17);
    }

    else if (list[0] == "dizhuCard") {
        std::vector<std::pair<color, number> > tmp;
        for (int i = 1; i < list.size(); ++i) tmp.push_back(stringToCard(list[i]));
        this->setLandlordCard(tmp);

        flipLandlordCard();
    }

    else if (list[0] == "determine") {
        if (state != gameState::DETERMINING) {
            if (list[1] == "You") nowPlayer = players::LOCAL_CLIENT;
            else if (list[1] == "XiaJia") nowPlayer = players::FIRST_REMOTE;
            else if (list[1] == "ShangJia") nowPlayer = players::SECOND_REMOTE;
        }
        // Unitify signal

        this->setState(gameState::DETERMINING);
    }

    else if (list[0] == "demand") {
        if (list[2] == "Yes") {
            if (list[1] == "ShangJia") parent->setSecondLabel(hasCalled ? "抢地主" : "叫地主");
            else parent->setFirstLabel(hasCalled ? "抢地主" : "叫地主");
            setHasCalled(true);
        } else {
            if (list[1] == "ShangJia") parent->setSecondLabel(hasCalled ? "不抢" : "不叫");
            else parent->setFirstLabel(hasCalled ? "不抢" : "不叫");
        }
        this->nextPlayer();
    }

    else if (list[0] == "done") {
        if (list[1] == "You") landlord = players::LOCAL_CLIENT;
        if (list[1] == "XiaJia") landlord = players::FIRST_REMOTE;
        if (list[1] == "ShangJia") landlord = players::SECOND_REMOTE;
        nowPlayer = landlord;

        if (landlord == players::FIRST_REMOTE) firstRemote->setHandInCard(20);
        else if (landlord == players::SECOND_REMOTE) secondRemote->setHandInCard(20);

        parent->setLocalLabel("");
        parent->setFirstLabel("");
        parent->setSecondLabel("");

        if (landlord == players::LOCAL_CLIENT) {
            parent->setLocalIdLabel("地主");
            parent->setFirstIdLabel("农民");
            parent->setSecondIdLabel("农民");
        }
        if (landlord == players::FIRST_REMOTE) {
            parent->setLocalIdLabel("农民");
            parent->setFirstIdLabel("地主");
            parent->setSecondIdLabel("农民");
        }
        if (landlord == players::SECOND_REMOTE) {
            parent->setLocalIdLabel("农民");
            parent->setFirstIdLabel("农民");
            parent->setSecondIdLabel("地主");
        }

        this->setState(gameState::RUNNING);
    }

    else if (list[0] == "drop") {

        if (list[2] == "Dan") {
            prev.any = false;
            prev.cat = cardsCategory::A;
            prev.index = stringToCard(list[4]).second;
            prev.helper = 0;
        } else if (list[2] == "Dui") {
            prev.any = false;
            prev.cat = cardsCategory::AA;
            prev.index = stringToCard(list[4]).second;
            prev.helper = 0;
        } else if (list[2] == "SanDai") {
            prev.any = false;
            prev.cat = list.size() == 8 ? cardsCategory::AAAB : cardsCategory::AAABB;
            prev.index = stringToCard(list[6]).second;
            prev.helper = 0;
        } else if (list[2] == "Shun") {
            prev.any = false;
            prev.cat = cardsCategory::ABCDE;
            prev.index = stringToCard(list[4]).second;
            prev.helper = list.size() - 4;
        } else if (list[2] == "ShuangShun") {
            prev.any = false;
            prev.cat = cardsCategory::AABBCC;
            prev.index = stringToCard(list[4]).second;
            prev.helper = (list.size() - 4) / 2;
        } else if (list[2] == "SanShun") {
            prev.any = false;
            prev.cat = cardsCategory::AAABBB;
            prev.index = stringToCard(list[4]).second;
            prev.helper = (list.size() - 4) / 3;
        } else if (list[2] == "DanFeiJi") {
            prev.any = false;
            prev.cat = cardsCategory::AAABBBCD;
            prev.index = extract(list, cardsCategory::AAABBBCD);
            // ???
            prev.helper = (list.size() - 4) / 4;
        } else if (list[2] == "DuiFeiJi") {
            prev.any = false;
            prev.cat = cardsCategory::AAABBBCCDD;
            prev.index = extract(list, cardsCategory::AAABBBCCDD);
            // ???
            prev.helper = (list.size() - 4) / 5;
        } else if (list[2] == "SiDai") {
            prev.any = false;
            prev.cat = list.size() == 10 ? cardsCategory::AAAABC : cardsCategory::AAAABBCC;
            prev.index = list.size() == 10 ? stringToCard(list[6]).second : (list[4] == list[6] ? stringToCard(list[4]).second : stringToCard(list[8]).second);
            prev.helper = list.size() == 10 ? 1 : 2;
        } else if (list[2] == "ZhaDan") {
            prev.any = false;
            prev.cat = list.size() == 6 ? cardsCategory::KK : cardsCategory::AAAA;
            prev.index = list.size() == 6 ? number::_BK : stringToCard(list[4]).second;
            prev.helper = 0;
        }

        int n = list[3].toInt();
        if (list[2] != "Pass") {
            for (int i = 4; i < 4 + n; ++i) {
                if (list[1] == "ShangJia") secondRemote->setOutDrawCard(stringToCard(list[i]));
                else firstRemote->setOutDrawCard(stringToCard(list[i]));
            }
            if (list[1] == "ShangJia") secondRemote->setHandInCard(secondRemote->getHandInCard().size() - n);
            else firstRemote->setHandInCard(firstRemote->getHandInCard().size() - n);
        } else {
            if (list[1] == "ShangJia") parent->setSecondLabel("不出");
            else parent->setFirstLabel("不出");
        }
        this->nextPlayer();
    }

    else if (list[0] == "yourTurn") {
        local->clearOutDraw();
        parent->setLocalLabel("");
        if (list[1] == "First") {
            parent->setPassDisabled(true);
            prev.any = true;
        } else {
            parent->setPassDisabled(false);
            prev.any = false;
        }
    }

    else if (list[0] == "turn") {
        if (list[1] == "ShangJia") secondRemote->clearOutDraw();
        else firstRemote->clearOutDraw();
    }

    else if (list[0] == "win") {
        parent->readyForRestart();
        parent->setOverLabel("你赢了！");
    }

    else if (list[0] == "lose") {
        parent->readyForRestart();
        parent->setOverLabel("你输了！");
    }

    else if (list[0] == "gameend") {
        setState(gameState::TERMINATED);
        parent->setOverLabel("游戏结束");
    }

    else if (list[0] == "rebegin") {
        state = gameState::READY_FOR_CONNECTION;

        delete local;
        local = new localPlayer(parent, this);

        delete firstRemote;
        firstRemote = new remotePlayer(parent);

        delete secondRemote;
        secondRemote = new remotePlayer(parent);

        hasCalled = false;

        if (coreServer != nullptr) coreServer->clearServer();

        for (auto ptr: landlordCard) delete ptr;
        landlordCard.clear();

        landlordCard.push_back(new card(color::JOKER, number::_BK, true, parent));
        landlordCard.push_back(new card(color::JOKER, number::_BK, true, parent));
        landlordCard.push_back(new card(color::JOKER, number::_BK, true, parent));

        parent->resetRestartBtn();
    }

}

void localGameController::clearLandlordCard() {
    for (auto ptr: landlordCard) ptr->hide();
    for (auto ptr: landlordCard) delete ptr;
    landlordCard.clear();
}

bool localGameController::chosenIsGreater() {
    cardsCategory pre = prev.cat;
    cardsCategory now = local->judgeOutDraw();

    if (prev.any) return true;

    if (now == cardsCategory::ILLEGAL) return false;
    if (pre != cardsCategory::AAAA && pre != cardsCategory::KK && (now == cardsCategory::AAAA || now == cardsCategory::KK)) return true;
    if (now != cardsCategory::AAAA && now != cardsCategory::KK && (pre == cardsCategory::AAAA || pre == cardsCategory::KK)) return false;
    if (pre == cardsCategory::KK && now == cardsCategory::AAAA) return false;
    if (pre == cardsCategory::AAAA && now == cardsCategory::KK) return true;
    if (pre != now) return false;

    std::vector<card*>& h = local->getHandInCard();
    std::vector<int>& loc = local->getChosenCardLocation();

    std::vector<card*> chosen;
    for (int x: loc) chosen.push_back(h[x]);

    if (now == cardsCategory::AAABBBCD || now == cardsCategory::AAABBBCCDD) {
        QStringList l;
        for (int i = 0; i < 4; ++i) l.push_back(" ");

        for (auto ptr: chosen) l.push_back(cardPtrToString_(ptr).c_str());

        number ind = extract(l, now);
        return ind > prev.index;
    } else {
        number ind = number::_2;
        int hel = 0;

        if (now == cardsCategory::A) ind = chosen[0]->getCardNumber();
        else if (now == cardsCategory::AA) ind = chosen[0]->getCardNumber();
        else if (now == cardsCategory::AAA) ind = chosen[0]->getCardNumber();
        else if (now == cardsCategory::AAAB) ind = chosen[1]->getCardNumber();
        else if (now == cardsCategory::AAABB) ind = chosen[2]->getCardNumber();
        else if (now == cardsCategory::ABCDE) ind = chosen[0]->getCardNumber();
        else if (now == cardsCategory::AAAABC) ind = chosen[2]->getCardNumber();
        else if (now == cardsCategory::AABBCC) ind = chosen[0]->getCardNumber();
        else if (now == cardsCategory::AAABBB) ind = chosen[0]->getCardNumber();
        else if (now == cardsCategory::AAAABBCC) ind = chosen[0]->getCardNumber() == chosen[2]->getCardNumber() ? chosen[2]->getCardNumber() : chosen[4]->getCardNumber();

        if (now == cardsCategory::ABCDE) hel = chosen.size();
        else if (now == cardsCategory::AABBCC) hel = chosen.size() / 2;
        else if (now == cardsCategory::AAABBB) hel = chosen.size() / 3;
        else if (now == cardsCategory::AAAABC) hel = 1;
        else if (now == cardsCategory::AAAABBCC) hel = 2;

        return hel == prev.helper && ind > prev.index;
    }
}
