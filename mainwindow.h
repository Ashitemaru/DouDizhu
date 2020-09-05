#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "localgamecontroller.h"
#include "struct.h"

#include <QMainWindow>
#include <QMouseEvent>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include <vector>

class MainWindow: public QMainWindow {

    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    virtual void mousePressEvent(QMouseEvent*) override;

    void updateUi();

    void setLocalLabel(const QString&);
    void setFirstLabel(const QString&);
    void setSecondLabel(const QString&);

    void setLocalIdLabel(const QString&);
    void setFirstIdLabel(const QString&);
    void setSecondIdLabel(const QString&);

    void setOverLabel(const QString&);

    void setPassDisabled(bool);

    void readyForRestart();

    void resetRestartBtn();

private:
    QTimer* timer;

    localGameController* game;

    QPushButton* serverConnectBtn;
    QPushButton* clientConnectBtn;
    QPushButton* dropBtn;
    QPushButton* passBtn;
    QPushButton* callLandlordBtn;
    QPushButton* passDeterBtn;

    QLabel* localLabel;
    QLabel* firstLabel;
    QLabel* secondLabel;

    QLabel* localIdLabel;
    QLabel* firstIdLabel;
    QLabel* secondIdLabel;

    QLabel* localNumLabel;
    QLabel* firstNumLabel;
    QLabel* secondNumLabel;

    QLabel* localWaitLabel;
    QLabel* firstWaitLabel;
    QLabel* secondWaitLabel;

    QLabel* overLabel;
    QPushButton* restartBtn;
    QPushButton* quitBtn;

    bool initDone;
    bool passDisabled;

    void initUi();
    void initSignalSlot();
    void initNetwork(localType);

};
#endif // MAINWINDOW_H
