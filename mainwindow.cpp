#include "mainwindow.h"
#include "constant.h"
#include "struct.h"

#include <QTimer>
#include <QObject>
#include <QPushButton>
#include <QLineEdit>
#include <QFont>
#include <QVBoxLayout>
#include <QWidget>
#include <QPainter>

#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , initDone(false)
    , passDisabled(false) {
    initUi();
    initSignalSlot();
}

MainWindow::~MainWindow() {}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (initDone) {
        int x = event->x();
        int y = event->y();

        if (y <= LOCAL_PLAYER_Y + CARD_HEIGHT + CARD_SHIFT && y >= LOCAL_PLAYER_Y) {
            int handInSize = game->getLocalPlayer()->getHandInCard().size();
            int internalShift = (LOCAL_AREA_WIDTH - ((handInSize - 1) * CARD_INTERVAL + CARD_WIDTH)) / 2;

            int pressedOnLocation = (x - LOCAL_PLAYER_X - internalShift) / CARD_SHIFT;

            if (pressedOnLocation >= 0 && pressedOnLocation <= handInSize - 2) game->getLocalPlayer()->setChosenCard(pressedOnLocation);
            if (
                    x >= LOCAL_PLAYER_X + internalShift + (handInSize - 1) * CARD_INTERVAL &&
                    x <= LOCAL_PLAYER_X + internalShift + (handInSize - 1) * CARD_INTERVAL + CARD_WIDTH
               ) game->getLocalPlayer()->setChosenCard(handInSize - 1);
        }
    }
}

void MainWindow::updateUi() {
    std::vector<card*>& handIn = game->getLocalPlayer()->getHandInCard();
    std::vector<int>& chosen = game->getLocalPlayer()->getChosenCardLocation();

    int internalShift = (LOCAL_AREA_WIDTH - ((handIn.size() - 1) * CARD_INTERVAL + CARD_WIDTH)) / 2;

    for (int i = 0; i < (int)handIn.size(); ++i) {
        std::vector<int>::iterator iter = std::find(chosen.begin(), chosen.end(), i);
        if (iter == chosen.end()) {
            handIn[i]->move(LOCAL_PLAYER_X + internalShift + i * CARD_INTERVAL, LOCAL_PLAYER_Y + CARD_SHIFT);
        } else {
            handIn[i]->move(LOCAL_PLAYER_X + internalShift + i * CARD_INTERVAL, LOCAL_PLAYER_Y);
        }
    }

    for (auto ptr: handIn) ptr->raise();

    // Local cards

    std::vector<card*>& firstHandIn = game->getFirstRemotePlayer()->getHandInCard();
    std::vector<card*>& secondHandIn = game->getSecondRemotePlayer()->getHandInCard();

    int firstSize = firstHandIn.size();
    int secondSize = secondHandIn.size();

    int firstInterval = (REMOTE_HEIGHT - ((firstSize - 1) * CARD_SHIFT + CARD_HEIGHT)) / 2;
    int secondInterval = (REMOTE_HEIGHT - ((secondSize - 1) * CARD_SHIFT + CARD_HEIGHT)) / 2;

    for (int i = 0; i < (int)firstHandIn.size(); ++i) {
        firstHandIn[i]->move(WINDOW_WIDTH - CARD_WIDTH - REMOTE_GAP, REMOTE_Y + firstInterval + i * CARD_SHIFT);
        firstHandIn[i]->show();
    }

    for (int i = 0; i < (int)secondHandIn.size(); ++i) {
        secondHandIn[i]->move(REMOTE_GAP, REMOTE_Y + secondInterval + i * CARD_SHIFT);
        secondHandIn[i]->show();
    }

    // Remote cards

    std::vector<card*>& outdraw = game->getLocalPlayer()->getOutdrawCard();
    int upInternalShift = (OUTDRAW_WIDTH - ((outdraw.size() - 1) * CARD_INTERVAL + CARD_WIDTH)) / 2;

    for (int i = 0; i < (int)outdraw.size(); ++i) {
        outdraw[i]->move(OUTDRAW_X + upInternalShift + i * CARD_INTERVAL, OUTDRAW_Y);
    }

    // Local outdraw cards

    std::vector<card*>& firstOut = game->getFirstRemotePlayer()->getOutDrawCard();
    std::vector<card*>& secondOut = game->getSecondRemotePlayer()->getOutDrawCard();

    for (int i = 0; i < (int)firstOut.size(); ++i) {
        firstOut[i]->move(WINDOW_WIDTH - REMOTE_OUTDRAW_GAP - CARD_WIDTH - i * CARD_INTERVAL, REMOTE_OUTDRAW_Y);
    }

    for (int i = (int)firstOut.size() - 1; i >= 0; --i) {
        firstOut[i]->raise();
    }

    for (int i = 0; i < (int)secondOut.size(); ++i) {
        secondOut[i]->move(REMOTE_OUTDRAW_GAP + i * CARD_INTERVAL, REMOTE_OUTDRAW_Y);
    }

    for (int i = 0; i < (int)secondOut.size(); ++i) {
        secondOut[i]->raise();
    }

    // Remote outdraw cards

    serverConnectBtn->setVisible(game->getState() == gameState::READY_FOR_CONNECTION);
    clientConnectBtn->setVisible(game->getState() == gameState::READY_FOR_CONNECTION);

    dropBtn->setVisible(game->getState() == gameState::RUNNING);
    passBtn->setVisible(game->getState() == gameState::RUNNING);

    if (game->getHasCalled()) {
        callLandlordBtn->setText(tr("抢地主"));
        passDeterBtn->setText(tr("不抢"));
    } else {
        callLandlordBtn->setText(tr("叫地主"));
        passDeterBtn->setText(tr("不叫"));
    }

    callLandlordBtn->setVisible(game->getState() == gameState::DETERMINING && game->getNowPlayer() == players::LOCAL_CLIENT);
    passDeterBtn->setVisible(game->getState() == gameState::DETERMINING && game->getNowPlayer() == players::LOCAL_CLIENT);

    dropBtn->setDisabled(game->getLocalPlayer()->judgeOutDraw() == cardsCategory::ILLEGAL || !game->chosenIsGreater());
    passBtn->setDisabled(passDisabled);

    dropBtn->setVisible(game->getNowPlayer() == players::LOCAL_CLIENT && game->getState() == gameState::RUNNING);
    passBtn->setVisible(game->getNowPlayer() == players::LOCAL_CLIENT && game->getState() == gameState::RUNNING);

    // Button

    if (
            game->getState() != gameState::READY_FOR_CONNECTION &&
            game->getState() != gameState::DEALING
       ) {
        std::vector<card*>& landCards = game->getLandlordCard();

        // qDebug() << "SIZE: " << landCards.size() << Qt::endl;

        for (int i = 0; i < std::min(3, (int)landCards.size()); ++i) {
            landCards[i]->move(LAND_X + i * LAND_CARD_INTERVAL, LAND_TOP_MARGIN);
            landCards[i]->show();
        }
    }

    // Landlord cards

    localNumLabel->setText(QString::number(game->getLocalPlayer()->getHandInCard().size()));
    firstNumLabel->setText(QString::number(game->getFirstRemotePlayer()->getHandInCard().size()));
    secondNumLabel->setText(QString::number(game->getSecondRemotePlayer()->getHandInCard().size()));

    localNumLabel->setVisible(game->getState() == gameState::RUNNING);
    firstNumLabel->setVisible(game->getState() == gameState::RUNNING);
    secondNumLabel->setVisible(game->getState() == gameState::RUNNING);

    // Num

    localWaitLabel->setVisible(game->getState() == gameState::RUNNING && game->getNowPlayer() == players::LOCAL_CLIENT);
    firstWaitLabel->setVisible(game->getState() == gameState::RUNNING && game->getNowPlayer() == players::FIRST_REMOTE);
    secondWaitLabel->setVisible(game->getState() == gameState::RUNNING && game->getNowPlayer() == players::SECOND_REMOTE);

    // Wait

    overLabel->setVisible(game->getState() == gameState::WAIT_FOR_RESTART || game->getState() == gameState::TERMINATED);
    restartBtn->setVisible(game->getState() == gameState::WAIT_FOR_RESTART);
    quitBtn->setVisible(game->getState() == gameState::WAIT_FOR_RESTART);

    // Restart

    localIdLabel->setVisible(game->getState() == gameState::RUNNING);
    firstIdLabel->setVisible(game->getState() == gameState::RUNNING);
    secondIdLabel->setVisible(game->getState() == gameState::RUNNING);

    // Id

    localLabel->setVisible(game->getState() == gameState::RUNNING || game->getState() == gameState::DETERMINING);
    firstLabel->setVisible(game->getState() == gameState::RUNNING || game->getState() == gameState::DETERMINING);
    secondLabel->setVisible(game->getState() == gameState::RUNNING || game->getState() == gameState::DETERMINING);

    // Text

    if (game->getLocalPlayer()->getHandInCard().size() == 0 && game->getState() == gameState::RUNNING) game->postMessage("WIN");

}

void MainWindow::setLocalLabel(const QString& msg) {
    localLabel->setText(msg);
    update();
    repaint();
}

void MainWindow::setFirstLabel(const QString& msg) {
    firstLabel->setText(msg);
    update();
    repaint();
}

void MainWindow::setSecondLabel(const QString& msg) {
    secondLabel->setText(msg);
    update();
    repaint();
}

void MainWindow::setOverLabel(const QString& msg) {
    overLabel->setText(msg);
    update();
    repaint();
}

void MainWindow::setLocalIdLabel(const QString& msg) {
    localIdLabel->setText(msg);
    update();
    repaint();
}

void MainWindow::setFirstIdLabel(const QString& msg) {
    firstIdLabel->setText(msg);
    update();
    repaint();
}

void MainWindow::setSecondIdLabel(const QString& msg) {
    secondIdLabel->setText(msg);
    update();
    repaint();
}

void MainWindow::setPassDisabled(bool b) {
    passDisabled = b;
}

void MainWindow::initUi() {
    this->resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    this->setFixedSize(width(), height());

    // Basic

    timer = new QTimer;

    // Timer

    serverConnectBtn = new QPushButton(tr("作为服务器连接"), this);
    serverConnectBtn->setGeometry(BTN_X, BTN_Y, BTN_WIDTH, BTN_HEIGHT);
    serverConnectBtn->setVisible(true);

    clientConnectBtn = new QPushButton(tr("作为客户端连接"), this);
    clientConnectBtn->setGeometry(BTN_X, BTN_Y + BTN_INTERVAL, BTN_WIDTH, BTN_HEIGHT);
    clientConnectBtn->setVisible(true);

    // Login button

    dropBtn = new QPushButton(tr("出牌"), this);
    dropBtn->setGeometry(DROP_X, DROP_Y, DROP_WIDTH, DROP_HEIGHT);
    dropBtn->setVisible(false);

    passBtn = new QPushButton(tr("不出"), this);
    passBtn->setGeometry(PASS_X, PASS_Y, DROP_WIDTH, DROP_HEIGHT);
    passBtn->setVisible(false);

    // Drop & pass

    callLandlordBtn = new QPushButton(tr("叫地主"), this);
    callLandlordBtn->setGeometry(DROP_X, DROP_Y, DROP_WIDTH, DROP_HEIGHT);
    callLandlordBtn->setVisible(false);

    passDeterBtn = new QPushButton(tr("不叫"), this);
    passDeterBtn->setGeometry(PASS_X, PASS_Y, DROP_WIDTH, DROP_HEIGHT);
    passDeterBtn->setVisible(false);

    // Call & pass

    localLabel = new QLabel(this);
    localLabel->setText(tr(""));

    firstLabel = new QLabel(this);
    firstLabel->setText(tr(""));

    secondLabel = new QLabel(this);
    secondLabel->setText(tr(""));

    QFont* font = new QFont;
    font->setFamily("FZQingKeBenYueSongS-R-GB");
    font->setPixelSize(20);

    localLabel->setFont(*font);
    localLabel->setGeometry(TEXT_X, TEXT_Y, TEXT_WIDTH, TEXT_HEIGHT);
    localLabel->setAlignment(Qt::AlignCenter);
    localLabel->setVisible(true);

    firstLabel->setFont(*font);
    firstLabel->setGeometry(WINDOW_WIDTH - TEXT_WIDTH - REMOTE_TEXT_GAP, REMOTE_TEXT_Y, TEXT_WIDTH, TEXT_HEIGHT);
    firstLabel->setAlignment(Qt::AlignCenter);
    firstLabel->setVisible(true);

    secondLabel->setFont(*font);
    secondLabel->setGeometry(REMOTE_TEXT_GAP, REMOTE_TEXT_Y, TEXT_WIDTH, TEXT_HEIGHT);
    secondLabel->setAlignment(Qt::AlignCenter);
    secondLabel->setVisible(true);

    // Labels

    localIdLabel = new QLabel(this);
    firstIdLabel = new QLabel(this);
    secondIdLabel = new QLabel(this);

    localIdLabel->setGeometry(LOCAL_ID_X, LOCAL_ID_Y, ID_WIDTH, ID_HEIGHT);
    firstIdLabel->setGeometry(WINDOW_WIDTH - ID_WIDTH - REMOTE_ID_GAP, REMOTE_ID_Y, ID_WIDTH, ID_HEIGHT);
    secondIdLabel->setGeometry(REMOTE_ID_GAP, REMOTE_ID_Y, ID_WIDTH, ID_HEIGHT);

    localIdLabel->setFont(*font);
    firstIdLabel->setFont(*font);
    secondIdLabel->setFont(*font);

    localIdLabel->setAlignment(Qt::AlignCenter);
    firstIdLabel->setAlignment(Qt::AlignCenter);
    secondIdLabel->setAlignment(Qt::AlignCenter);

    localIdLabel->setText("");
    firstIdLabel->setText("");
    secondIdLabel->setText("");

    // Identity

    localNumLabel = new QLabel(this);
    firstNumLabel = new QLabel(this);
    secondNumLabel = new QLabel(this);

    localNumLabel->setGeometry(LOCAL_NUM_X, LOCAL_NUM_Y, ID_WIDTH, ID_HEIGHT);
    firstNumLabel->setGeometry(WINDOW_WIDTH - ID_WIDTH - REMOTE_ID_GAP, REMOTE_NUM_Y, ID_WIDTH, ID_HEIGHT);
    secondNumLabel->setGeometry(REMOTE_ID_GAP, REMOTE_NUM_Y, ID_WIDTH, ID_HEIGHT);

    localNumLabel->setFont(*font);
    firstNumLabel->setFont(*font);
    secondNumLabel->setFont(*font);

    localNumLabel->setAlignment(Qt::AlignCenter);
    firstNumLabel->setAlignment(Qt::AlignCenter);
    secondNumLabel->setAlignment(Qt::AlignCenter);

    localNumLabel->setText("");
    firstNumLabel->setText("");
    secondNumLabel->setText("");

    // Num

    localWaitLabel = new QLabel(this);
    firstWaitLabel = new QLabel(this);
    secondWaitLabel = new QLabel(this);

    localWaitLabel->setGeometry(TEXT_X, LOCAL_WAIT_Y, TEXT_WIDTH, ID_HEIGHT);
    firstWaitLabel->setGeometry(WINDOW_WIDTH - ID_WIDTH - REMOTE_ID_GAP - 10, REMOTE_WAIT_Y, TEXT_WIDTH, ID_HEIGHT);
    // A mild shift
    secondWaitLabel->setGeometry(REMOTE_ID_GAP, REMOTE_WAIT_Y, TEXT_WIDTH, ID_HEIGHT);

    localWaitLabel->setFont(*font);
    firstWaitLabel->setFont(*font);
    secondWaitLabel->setFont(*font);

    localWaitLabel->setAlignment(Qt::AlignCenter);
    firstWaitLabel->setAlignment(Qt::AlignCenter);
    secondWaitLabel->setAlignment(Qt::AlignCenter);

    localWaitLabel->setText("<font color=red>等待中</font>");
    firstWaitLabel->setText("<font color=red>等待中</font>");
    secondWaitLabel->setText("<font color=red>等待中</font>");

    localWaitLabel->setVisible(false);
    firstWaitLabel->setVisible(false);
    secondWaitLabel->setVisible(false);

    // Waiting

    QFont* bigFont = new QFont;
    bigFont->setPixelSize(50);
    bigFont->setFamily("FZQingKeBenYueSongS-R-GB");

    overLabel = new QLabel(this);
    overLabel->setGeometry(OVER_X, OVER_Y, OVER_WIDTH, OVER_HEIGHT);
    overLabel->setFont(*bigFont);
    overLabel->setVisible(false);
    overLabel->setAlignment(Qt::AlignCenter);

    restartBtn = new QPushButton(tr("重新开始"), this);
    quitBtn = new QPushButton(tr("退出游戏"), this);

    restartBtn->setGeometry(BTN_X, BTN_Y, BTN_WIDTH, BTN_HEIGHT);
    quitBtn->setGeometry(BTN_X, BTN_Y + BTN_INTERVAL, BTN_WIDTH, BTN_HEIGHT);

    restartBtn->setVisible(false);
    quitBtn->setVisible(false);

    // Over

}

void MainWindow::initSignalSlot() {
    QObject::connect(timer, &QTimer::timeout, [&]() {
        this->updateUi();
        this->update();
    });

    // Timer

    QObject::connect(serverConnectBtn, &QPushButton::clicked, [&]() {
        initNetwork(localType::SERVER);
        game->setState(gameState::DEALING);
    });
    QObject::connect(clientConnectBtn, &QPushButton::clicked, [&]() {
        initNetwork(localType::CLIENT);
        game->setState(gameState::DEALING);
    });

    QObject::connect(dropBtn, &QPushButton::clicked, [&]() {
        this->game->nextPlayer();
        this->game->getLocalPlayer()->outDraw();
    });
    QObject::connect(passBtn, &QPushButton::clicked, [&]() {
        this->game->nextPlayer();
        this->localLabel->setText("不出");
        this->game->getLocalPlayer()->clearChosen();
        this->game->postMessage("drop Pass 00");
    });

    QObject::connect(callLandlordBtn, &QPushButton::clicked, [&]() {
        this->game->nextPlayer();
        if (game->getHasCalled()) localLabel->setText(tr("抢地主"));
        else localLabel->setText(tr("叫地主"));
        game->setHasCalled(true);
        localLabel->show();
        this->game->postMessage("demand Yes");
    });
    QObject::connect(passDeterBtn, &QPushButton::clicked, [&]() {
        this->game->nextPlayer();
        if (game->getHasCalled()) localLabel->setText(tr("不抢"));
        else localLabel->setText(tr("不叫"));
        localLabel->show();
        this->game->postMessage("demand No");
    });

    QObject::connect(restartBtn, &QPushButton::clicked, [&]() {
        this->game->postMessage("restart");
        quitBtn->setDisabled(true);
        restartBtn->setDisabled(true);
    });
    QObject::connect(quitBtn, &QPushButton::clicked, [&]() {
        this->game->postMessage("quit");
        quitBtn->setDisabled(true);
        restartBtn->setDisabled(true);
    });

    // Button
}

void MainWindow::initNetwork(localType t) {
    game = new localGameController(t, this);
    initDone = true;
    timer->start(TIMER_INTERVAL);
}

void MainWindow::readyForRestart() {
    game->setState(gameState::WAIT_FOR_RESTART);
    game->getLocalPlayer()->clearOutDraw();
    game->getLocalPlayer()->clearHandIn();
    game->getFirstRemotePlayer()->clearOutDraw();
    game->getSecondRemotePlayer()->clearOutDraw();
    game->getFirstRemotePlayer()->setHandInCard(0);
    game->getSecondRemotePlayer()->setHandInCard(0);

    game->clearLandlordCard();

    localLabel->setText("");
    firstLabel->setText("");
    secondLabel->setText("");

    localIdLabel->setText("");
    firstIdLabel->setText("");
    secondIdLabel->setText("");

    localNumLabel->setText("");
    firstNumLabel->setText("");
    secondNumLabel->setText("");

    update();
    repaint();
}

void MainWindow::resetRestartBtn() {
    restartBtn->setDisabled(false);
    quitBtn->setDisabled(false);
}
