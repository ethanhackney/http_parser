#include "lexer.h"

lexer::lexer(FILE *fp)
        : _fp {fp}
{
        if (_fp == nullptr) {
                usage("bad FILE*");
        }
}

const token& lexer::next(void)
{
        for (;;) {
                auto c = _putback;
                if (c != 0)
                        _putback = 0;
                else
                        c = fgetc(_fp);

                if ((_inval || _first) && c == ' ')
                        continue;

                switch (c) {
                case EOF:
                        return _curr = token{};
                case '\r':
                        if (fgetc(_fp) != '\n')
                                usage("\\r not followed by \\n");
                        _first = false;
                        _inhdr = true;
                        _inval = false;
                        return _curr = token{TOK_EOL};
                case '/':
                        return _curr = token{TOK_SLASH};
                case ':':
                        _inhdr = false;
                        _inval = true;
                        return _curr = token{TOK_COLON};
                case '.':
                        return _curr = token{TOK_DOT};
                case '=':
                        return _curr = token{TOK_EQ};
                case ',':
                        return _curr = token{TOK_COMMA};
                case ';':
                        return _curr = token{TOK_SEMI};
                }

                if (_inhdr && (isalpha(c) || c == '-')) {
                        std::string s {};
                        while (isalpha(c) || c == '-') {
                                s += c;
                                c = fgetc(_fp);
                        }
                        _putback = c;
                        if (s == "Content-Length")
                                return _curr = token{TOK_CONTENT_LENGTH, s};
                        if (s == "Accept")
                                return _curr = token{TOK_ACCEPT, s};
                        if (s == "Accept-Charset")
                                return _curr = token{TOK_ACCEPT_CHARSET, s};
                        if (s == "Accept-Encoding")
                                return _curr = token{TOK_ACCEPT_ENCODING, s};
                        usage("bad header: %s", s.c_str());
                }

                if (_first && isalpha(c)) {
                        std::string s {};
                        while (isalpha(c)) {
                                s += c;
                                c = fgetc(_fp);
                        }
                        _putback = c;
                        if (s == "GET")
                                return _curr = token{TOK_GET, s};
                        if (s == "HTTP")
                                return _curr = token{TOK_HTTP, s};
                        return _curr = token{TOK_WORD, s};
                }

                if (_inval && (isalpha(c) || c == '-' || c == '*')) {
                        std::string s {};
                        while (isalpha(c) || isdigit(c) ||
                                        c == '-' || c == '*') {
                                s += c;
                                c = fgetc(_fp);
                        }
                        _putback = c;
                        return _curr = token{TOK_WORD, s};
                }

                if (isdigit(c)) {
                        std::string s {};
                        while (isdigit(c)) {
                                s += c;
                                c = fgetc(_fp);
                        }
                        if (c == '.') {
                                s += c;
                                c = fgetc(_fp);
                                while (isdigit(c)) {
                                        s += c;
                                        c = fgetc(_fp);
                                }
                        }
                        _putback = c;
                        return _curr = token{TOK_NUM, s};
                }

                usage("bad character: %c", c);
        }
}

const token& lexer::curr(void) const
{
        return _curr;
}

void lexer::skip(int type)
{
        if (_curr.type() == type) {
                next();
                return;
        }
        usage("expected %s, got %s",
            token{type}.name().c_str(), _curr.name().c_str());
}

int lexer::type(void) const
{
        return _curr.type();
}

const std::string& lexer::lex(void) const
{
        return _curr.lex();
}

const std::string& lexer::name(void) const
{
        return _curr.name();
}

