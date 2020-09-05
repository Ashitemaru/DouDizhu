#ifndef STRUCT_H
#define STRUCT_H

enum color {
    DIAMOND,
    CLUB,
    HEART,
    SPADE,
    JOKER,
};

enum number {
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,
    _10,
    _J,
    _Q,
    _K,
    _A,
    _2,
    _LK,
    _BK,
};

enum cardsCategory {
    A,
    AA,
    AAA,
    AAAB,
    AAABB,
    ABCDE,
    AAAABC,
    AABBCC,
    AAABBB,
    AAABBBCD,
    AAABBBCCDD,
    AAAABBCC,
    AAAA,
    KK,
    ILLEGAL,
};

enum players {
    LOCAL_CLIENT,
    FIRST_REMOTE,
    SECOND_REMOTE,
};

enum localType {
    CLIENT,
    SERVER,
};

enum gameState {
    READY_FOR_CONNECTION,
    DEALING,
    DETERMINING,
    RUNNING,
    WAIT_FOR_RESTART,
    TERMINATED,
};

struct drop {
    cardsCategory cat;
    number index;
    int helper;
    bool any;
};

#endif // STRUCT_H
