#include "card.h"
#include "constant.h"

#include <QPainter>
#include <QImage>
#include <QRect>
#include <QDebug>

#include <string>
#include <iostream>

card::card(color c, number n, bool p, QWidget *parent)
    : QWidget(parent)
    , cardColor(c)
    , cardNumber(n)
    , isPadding(p) {
    this->resize(CARD_WIDTH, CARD_HEIGHT);
}

color card::getCardColor() {
    return cardColor;
}

number card::getCardNumber() {
    return cardNumber;
}

bool card::operator<(const card& other) {
    if (cardColor == color::JOKER && other.cardColor == color::JOKER) {
        if (cardNumber == number::_LK && other.cardNumber == number::_BK) return true;
        return false;
    }

    if (cardColor == color::JOKER) return false;
    if (other.cardColor == color::JOKER) return true;

    if (cardNumber < other.cardNumber) return true;
    if (cardNumber > other.cardNumber) return false;

    return cardColor < other.cardColor;
}

bool card::operator==(const card& other) {
    return cardColor == other.cardColor && cardNumber == other.cardNumber;
}

void card::paintEvent(QPaintEvent* event) {
    std::string name = ":/cards/";

    if (!isPadding) {
        switch (cardColor) {
            case color::CLUB: name.push_back('C'); break;
            case color::DIAMOND: name.push_back('D'); break;
            case color::SPADE: name.push_back('S'); break;
            case color::HEART: name.push_back('H'); break;
            case color::JOKER: name.push_back('K'); break;
        }

        switch (cardNumber) {
            case number::_BK: name.push_back('1'); break;
            case number::_LK: name.push_back('0'); break;
            case number::_2: name.push_back('2'); break;
            case number::_A: name.push_back('A'); break;
            case number::_K: name.push_back('K'); break;
            case number::_Q: name.push_back('Q'); break;
            case number::_J: name.push_back('J'); break;
            case number::_10: name.push_back('X'); break;
            default: name.push_back('3' + cardNumber); break;
        }
    } else name = ":/cards/padding";

    if (x() * y()) QPainter(this).drawImage(QRect(0, 0, CARD_WIDTH, CARD_HEIGHT), QImage(QString(name.c_str())));

    QWidget::paintEvent(event);
}
