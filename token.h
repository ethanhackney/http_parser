#ifndef TOKEN_H
#define TOKEN_H

#include "error.h"
#include <array>
#include <string>

enum {
        TOK_EOF,
        TOK_EOL,
        TOK_CONTENT_LENGTH,
        TOK_GET,
        TOK_PATH,
        TOK_HTTP,
        TOK_NUM,
        TOK_SLASH,
        TOK_WORD,
        TOK_COLON,
        TOK_DOT,
        TOK_EQ,
        TOK_COMMA,
        TOK_SEMI,
        TOK_ACCEPT,
        TOK_ACCEPT_CHARSET,
        TOK_ACCEPT_ENCODING,
        TOK_COUNT,
};

class token {
private:
        std::string _lex {};
        int _type {TOK_EOF};
public:
        token(int type = TOK_EOF, const std::string& lex = "");
        int type(void) const;
        const std::string& lex(void) const;
        const std::string& name(void) const;
};

#endif
