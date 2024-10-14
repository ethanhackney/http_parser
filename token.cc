#include "token.h"

token::token(int type, const std::string& lex)
        : _lex {lex},
        _type {type}
{
        switch (_type) {
        case TOK_EOF:
        case TOK_EOL:
        case TOK_CONTENT_LENGTH:
        case TOK_GET:
        case TOK_PATH:
        case TOK_HTTP:
        case TOK_NUM:
        case TOK_SLASH:
        case TOK_WORD:
        case TOK_COLON:
        case TOK_DOT:
        case TOK_EQ:
        case TOK_COMMA:
        case TOK_SEMI:
        case TOK_ACCEPT:
        case TOK_ACCEPT_CHARSET:
        case TOK_ACCEPT_ENCODING:
        case TOK_ACCEPT_LANGUAGE:
                break;
        default:
                usage("bad token type: %d", _type);
        }
}

int token::type(void) const
{
        return _type;
}

const std::string& token::lex(void) const
{
        return _lex;
}

const std::string& token::name(void) const
{
        static std::array<std::string,TOK_COUNT> names {
                "TOK_EOF",
                "TOK_EOL",
                "TOK_CONTENT_LENGTH",
                "TOK_GET",
                "TOK_PATH",
                "TOK_HTTP",
                "TOK_NUM",
                "TOK_SLASH",
                "TOK_WORD",
                "TOK_COLON",
                "TOK_DOT",
                "TOK_EQ",
                "TOK_COMMA",
                "TOK_SEMI",
                "TOK_ACCEPT",
                "TOK_ACCEPT_CHARSET",
                "TOK_ACCEPT_ENCODING",
                "TOK_ACCEPT_LANGUAGE",
        };

        return names.at(_type);
}
