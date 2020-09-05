#include "server.h"
#include "struct.h"

#include <QObject>
#include <QByteArray>
#include <QtNetwork>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QStringList>

#include <vector>
#include <string>
#include <random>
#include <thread>
#include <chrono>

std::string cardToString(const std::pair<color, number>& p) {
    std::string s = "";

    if (p.first == color::JOKER) s.push_back('J');
    else if (p.first == color::SPADE) s.push_back('S');
    else if (p.first == color::HEART) s.push_back('H');
    else if (p.first == color::CLUB) s.push_back('C');
    else if (p.first == color::DIAMOND) s.push_back('D');

    if (p.second == number::_LK) s.push_back('B');
    else if (p.second == number::_BK) s.push_back('R');
    else if (p.second == number::_2) s.push_back('2');
    else if (p.second == number::_A) s.push_back('A');
    else if (p.second == number::_K) s.push_back('K');
    else if (p.second == number::_Q) s.push_back('Q');
    else if (p.second == number::_J) s.push_back('J');
    else if (p.second == number::_10) s.push_back('X');
    else s.push_back('3' + p.second);

    return s;
}

server::server(localGameController* u, QObject *parent)
    : QObject(parent)
    , localUpper(u)
    , firstReadWriteSocket(nullptr)
    , secondReadWriteSocket(nullptr)
    , remotePlayerNum(0)
    , demandNum(0)
    , totalRepost(0)
    , restartRepost(0) {

}

void server::initServer() {
    this->listenSocket = new QTcpServer;
    this->listenSocket->listen(QHostAddress::AnyIPv4, 1926);
    QObject::connect(listenSocket, &QTcpServer::newConnection, [&](){this->acceptConnection();});
}

void server::acceptConnection() {
    if (firstReadWriteSocket == nullptr) {
        this->firstReadWriteSocket = this->listenSocket->nextPendingConnection();
    } else if (secondReadWriteSocket == nullptr) {
        this->secondReadWriteSocket = this->listenSocket->nextPendingConnection();
    }
    else return;

    QObject::connect(this->firstReadWriteSocket, &QTcpSocket::readyRead, [&](){this->firstRecvMessage();});
    QObject::connect(this->secondReadWriteSocket, &QTcpSocket::readyRead, [&](){this->secondRecvMessage();});

    if (++remotePlayerNum == 2) startNewGame();
}

void server::firstRecvMessage() {
    QByteArray data = firstReadWriteSocket->read(4);

    int length = data.toHex().toUInt(nullptr, 16);
    analyzeClientInfo(players::FIRST_REMOTE, firstReadWriteSocket->read(length));

    if(!firstReadWriteSocket->atEnd())
        firstRecvMessage();
}

void server::secondRecvMessage() {
    QByteArray data = secondReadWriteSocket->read(4);

    int length = data.toHex().toUInt(nullptr, 16);
    analyzeClientInfo(players::SECOND_REMOTE, secondReadWriteSocket->read(length));

    if(!secondReadWriteSocket->atEnd())
        secondRecvMessage();
}

void server::analyzeClientInfo(players t, const QString& msg) {

    // qDebug() << "SERVER: " << t << " " << msg << Qt::endl;

    QStringList list = msg.split(' ');

    if (list[0] == "demand") {
        if (t == players::LOCAL_CLIENT) {
            postMessage(players::FIRST_REMOTE, "demand ShangJia " + list[1]);
            postMessage(players::SECOND_REMOTE, "demand XiaJia " + list[1]);
            if (list[1] == "Yes") landlord = players::LOCAL_CLIENT;
        }
        if (t == players::FIRST_REMOTE) {
            postMessage(players::SECOND_REMOTE, "demand ShangJia " + list[1]);
            postMessage(players::LOCAL_CLIENT, "demand XiaJia " + list[1]);
            if (list[1] == "Yes") landlord = players::FIRST_REMOTE;
        }
        if (t == players::SECOND_REMOTE) {
            postMessage(players::LOCAL_CLIENT, "demand ShangJia " + list[1]);
            postMessage(players::FIRST_REMOTE, "demand XiaJia " + list[1]);
            if (list[1] == "Yes") landlord = players::SECOND_REMOTE;
        }

        if (++demandNum == 3) {
            postMessage(landlord, QString(("deal " + cardToString(land_1)).c_str()));
            postMessage(landlord, QString(("deal " + cardToString(land_2)).c_str()));
            postMessage(landlord, QString(("deal " + cardToString(land_3)).c_str()));


            postMessage(players::LOCAL_CLIENT, QString(("dizhuCard " + cardToString(land_1) + " " + cardToString(land_2) + " " + cardToString(land_3)).c_str()));
            postMessage(players::FIRST_REMOTE, QString(("dizhuCard " + cardToString(land_1) + " " + cardToString(land_2) + " " + cardToString(land_3)).c_str()));
            postMessage(players::SECOND_REMOTE, QString(("dizhuCard " + cardToString(land_1) + " " + cardToString(land_2) + " " + cardToString(land_3)).c_str()));

            postMessage(players::LOCAL_CLIENT, [&]() {
                if (landlord == players::LOCAL_CLIENT) return "done You";
                if (landlord == players::FIRST_REMOTE) return "done XiaJia";
                if (landlord == players::SECOND_REMOTE) return "done ShangJia";
                return "";
            }());
            postMessage(players::FIRST_REMOTE, [&]() {
                if (landlord == players::LOCAL_CLIENT) return "done ShangJia";
                if (landlord == players::FIRST_REMOTE) return "done You";
                if (landlord == players::SECOND_REMOTE) return "done XiaJia";
                return "";
            }());
            postMessage(players::SECOND_REMOTE, [&]() {
                if (landlord == players::LOCAL_CLIENT) return "done XiaJia";
                if (landlord == players::FIRST_REMOTE) return "done ShangJia";
                if (landlord == players::SECOND_REMOTE) return "done You";
                return "";
            }());

            // std::vector<card*>& tmp = localUpper->getLandlordCard();

            prevPlayer = landlord;

            if (landlord == players::LOCAL_CLIENT) {
                postMessage(players::LOCAL_CLIENT, "yourTurn First");
                postMessage(players::FIRST_REMOTE, "turn ShangJia");
                postMessage(players::SECOND_REMOTE, "turn XiaJia");
            }
            if (landlord == players::FIRST_REMOTE) {
                postMessage(players::FIRST_REMOTE, "yourTurn First");
                postMessage(players::SECOND_REMOTE, "turn ShangJia");
                postMessage(players::LOCAL_CLIENT, "turn XiaJia");
            }
            if (landlord == players::SECOND_REMOTE) {
                postMessage(players::SECOND_REMOTE, "yourTurn First");
                postMessage(players::LOCAL_CLIENT, "turn ShangJia");
                postMessage(players::FIRST_REMOTE, "turn XiaJia");
            }
        } else {
            if (t == players::LOCAL_CLIENT) {
                postMessage(players::FIRST_REMOTE, "determine You");
                postMessage(players::SECOND_REMOTE, "determine ShangJia");
            }
            if (t == players::FIRST_REMOTE) {
                postMessage(players::SECOND_REMOTE, "determine You");
                postMessage(players::LOCAL_CLIENT, "determine ShangJia");
            }
            if (t == players::SECOND_REMOTE) {
                postMessage(players::LOCAL_CLIENT, "determine You");
                postMessage(players::FIRST_REMOTE, "determine ShangJia");
            }
        }

    } else if (list[0] == "drop") {

        if (t == players::LOCAL_CLIENT) {
            list.insert(1, "ShangJia");
            postMessage(players::FIRST_REMOTE, list.join(" "));
            list[1] = "XiaJia";
            postMessage(players::SECOND_REMOTE, list.join(" "));
        }
        if (t == players::FIRST_REMOTE) {
            list.insert(1, "ShangJia");
            postMessage(players::SECOND_REMOTE, list.join(" "));
            list[1] = "XiaJia";
            postMessage(players::LOCAL_CLIENT, list.join(" "));
        }
        if (t == players::SECOND_REMOTE) {
            list.insert(1, "ShangJia");
            postMessage(players::LOCAL_CLIENT, list.join(" "));
            list[1] = "XiaJia";
            postMessage(players::FIRST_REMOTE, list.join(" "));
        }

        if (list[2] != "Pass") {
            prevPlayer = t;
        }

        if (t == players::LOCAL_CLIENT) {
            postMessage(players::LOCAL_CLIENT, "turn XiaJia");
            postMessage(players::FIRST_REMOTE, QString((std::string("yourTurn ") + [&]() {
                if (prevPlayer != players::FIRST_REMOTE) return "NotFirst";
                return "First";
            }()).c_str()));
            postMessage(players::SECOND_REMOTE, "turn ShangJia");
        }
        if (t == players::FIRST_REMOTE) {
            postMessage(players::FIRST_REMOTE, "turn XiaJia");
            postMessage(players::SECOND_REMOTE, QString((std::string("yourTurn ") + [&]() {
                if (prevPlayer != players::SECOND_REMOTE) return "NotFirst";
                return "First";
            }()).c_str()));
            postMessage(players::LOCAL_CLIENT, "turn ShangJia");
        }
        if (t == players::SECOND_REMOTE) {
            postMessage(players::SECOND_REMOTE, "turn XiaJia");
            postMessage(players::LOCAL_CLIENT, QString((std::string("yourTurn ") + [&]() {
                if (prevPlayer != players::LOCAL_CLIENT) return "NotFirst";
                return "First";
            }()).c_str()));
            postMessage(players::FIRST_REMOTE, "turn ShangJia");
        }
    } else if (list[0] == "WIN") {
        if (t == landlord) {
            for (players p: {players::LOCAL_CLIENT, players::FIRST_REMOTE, players::SECOND_REMOTE})
                if (p != t) postMessage(p, "lose");
            postMessage(t, "win");
        } else {
            for (players p: {players::LOCAL_CLIENT, players::FIRST_REMOTE, players::SECOND_REMOTE})
                if (p != landlord) postMessage(p, "win");
            postMessage(landlord, "lose");
        }
    } else if (list[0] == "restart") {
        restartRepost++;
        totalRepost++;
        if (totalRepost == 3) handleRestart();
    } else if (list[0] == "quit") {
        totalRepost++;
        if (totalRepost == 3) handleRestart();
    }
}

void server::postMessage(players t, const QString& msg) {

    // qDebug() << "SERVER POST: " << t << ' ' << msg << Qt::endl;

    int size = qToBigEndian(msg.size());

    if (t == players::LOCAL_CLIENT) this->localUpper->analyzeServerInfo(msg);
    else if (t == players::FIRST_REMOTE) {
        this->firstReadWriteSocket->write((char*)&size, 4);
        this->firstReadWriteSocket->write(msg.toLatin1());
    } else {
        this->secondReadWriteSocket->write((char*)&size, 4);
        this->secondReadWriteSocket->write(msg.toLatin1());
    }
}

void server::startNewGame() {
    postMessage(players::FIRST_REMOTE, "begin");
    postMessage(players::SECOND_REMOTE, "begin");
    postMessage(players::LOCAL_CLIENT, "begin");
    srand(time(0));

    std::vector<std::pair<color, number> > deck;
    deck.push_back(std::make_pair(color::JOKER, number::_LK));
    deck.push_back(std::make_pair(color::JOKER, number::_BK));

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 13; ++j)
                deck.push_back(std::make_pair((color)i, (number)j));

    unsigned seed = time(0);
    std::shuffle(deck.begin(), deck.end(), std::default_random_engine(seed));

    for (int i = 0; i < 51; ++i) {
        if (i % 3 == 0) postMessage(players::LOCAL_CLIENT, QString(("deal " + cardToString(deck[i])).c_str()));
        if (i % 3 == 1) postMessage(players::FIRST_REMOTE, QString(("deal " + cardToString(deck[i])).c_str()));
        if (i % 3 == 2) postMessage(players::SECOND_REMOTE, QString(("deal " + cardToString(deck[i])).c_str()));
    }

    localUpper->getFirstRemotePlayer()->setHandInCard(17);
    localUpper->getSecondRemotePlayer()->setHandInCard(17);

    land_1 = deck[51];
    land_2 = deck[52];
    land_3 = deck[53];

    /*
    postMessage(players::LOCAL_CLIENT, QString(("dizhuCard " + cardToString(deck[51]) + " " + cardToString(deck[52]) + " " + cardToString(deck[53])).c_str()));
    postMessage(players::FIRST_REMOTE, QString(("dizhuCard " + cardToString(deck[51]) + " " + cardToString(deck[52]) + " " + cardToString(deck[53])).c_str()));
    postMessage(players::SECOND_REMOTE, QString(("dizhuCard " + cardToString(deck[51]) + " " + cardToString(deck[52]) + " " + cardToString(deck[53])).c_str()));
    */

    srand(time(0));
    int i = rand() % 3;

    if (i % 3 == 0) {
        landlord = players::LOCAL_CLIENT;
        postMessage(players::LOCAL_CLIENT, "determine You");
        postMessage(players::FIRST_REMOTE, "determine ShangJia");
        postMessage(players::SECOND_REMOTE, "determine XiaJia");
    }
    if (i % 3 == 1) {
        landlord = players::FIRST_REMOTE;
        postMessage(players::LOCAL_CLIENT, "determine XiaJia");
        postMessage(players::FIRST_REMOTE, "determine You");
        postMessage(players::SECOND_REMOTE, "determine ShangJia");
    }
    if (i % 3 == 2) {
        landlord = players::SECOND_REMOTE;
        postMessage(players::LOCAL_CLIENT, "determine ShangJia");
        postMessage(players::FIRST_REMOTE, "determine XiaJia");
        postMessage(players::SECOND_REMOTE, "determine You");
    }
}

void server::handleRestart() {
    if (restartRepost != 3) {
        postMessage(players::FIRST_REMOTE, "gameend");
        postMessage(players::SECOND_REMOTE, "gameend");
        postMessage(players::LOCAL_CLIENT, "gameend");
    } else {
        postMessage(players::FIRST_REMOTE, "rebegin");
        postMessage(players::SECOND_REMOTE, "rebegin");
        postMessage(players::LOCAL_CLIENT, "rebegin");
        startNewGame();
    }
}

void server::clearServer() {
    remotePlayerNum = 0;
    demandNum = 0;
    totalRepost = 0;
    restartRepost = 0;
}
