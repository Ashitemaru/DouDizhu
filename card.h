#ifndef CARD_H
#define CARD_H

#include "struct.h"

#include <QWidget>
#include <QObject>

class card: public QWidget {

    Q_OBJECT

public:
    explicit card(color, number, bool, QWidget *parent = nullptr);
    card(const card&) = delete;

    color getCardColor();
    number getCardNumber();

    bool operator<(const card& other);
    bool operator==(const card& other);

    virtual void paintEvent(QPaintEvent*) override;

private:
    color cardColor;
    number cardNumber;

    bool isPadding;

};

#endif // CARD_H
