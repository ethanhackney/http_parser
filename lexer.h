#ifndef LEXER_H
#define LEXER_H

#include "error.h"
#include "token.h"
#include <cstdio>
#include <string>

class lexer {
private:
        token _curr {};
        FILE* _fp {stdin};
        bool _first {true};
        bool _inhdr {false};
        bool _inval {false};
        int _putback {0};
public:
        lexer(FILE *fp = stdin);
        const token& next(void);
        const token& curr(void) const;
        void skip(int type);
        int type(void) const;
        const std::string& lex(void) const;
        const std::string& name(void) const;
};

#endif
