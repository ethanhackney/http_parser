#include <array>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <err.h>
#include <string>
#include <sysexits.h>
#include <unordered_map>
#include <vector>

static void usage(const char *fmt, ...)
{
        va_list va;
        va_start(va, fmt);
        errno = EINVAL;
        verr(EX_USAGE, fmt, va);
        va_end(va);
        exit(EXIT_FAILURE);
}

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
        };

        return names.at(_type);
}

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

struct media {
        std::string type;
        std::string subtype;
        float arg;
};

struct charset {
        std::string type;
        float arg;
};

struct request {
        std::string method;
        std::string path;
        float version;
        int len;
        std::vector<media> accept;
        std::vector<charset> charsets;
};

int main(void)
{
        request req;
        lexer lex {};

        lex.next();
        req.method = lex.lex();
        lex.next();
        while (lex.type() != TOK_HTTP) {
                if (lex.type() == TOK_SLASH)
                        req.path += "/";
                else
                        req.path += lex.lex();
                lex.next();
        }
        lex.skip(TOK_HTTP);
        lex.skip(TOK_SLASH);
        req.version = atof(lex.lex().c_str());
        lex.skip(TOK_NUM);
        lex.skip(TOK_EOL);

        while (lex.type() != TOK_EOL) {
                auto type = lex.type();
                lex.skip(type);
                lex.skip(TOK_COLON);

                if (type == TOK_CONTENT_LENGTH) {
                        req.len = atoi(lex.lex().c_str());
                        lex.skip(TOK_NUM);
                } else if (type == TOK_ACCEPT) {
                        while (lex.type() != TOK_EOL) {
                                media m {"", "", 0};
                                m.type = std::string{lex.lex()};
                                lex.skip(TOK_WORD);
                                lex.skip(TOK_SLASH);
                                m.subtype = std::string{lex.lex()};
                                lex.skip(TOK_WORD);
                                if (lex.type() == TOK_SEMI) {
                                        lex.skip(TOK_SEMI);
                                        lex.skip(TOK_WORD);
                                        lex.skip(TOK_EQ);
                                        m.arg = atof(lex.lex().c_str());
                                        lex.skip(TOK_NUM);
                                }
                                if (lex.type() == TOK_COMMA)
                                        lex.skip(TOK_COMMA);
                                req.accept.push_back(m);
                        }
                } else if (type == TOK_ACCEPT_CHARSET) {
                        while (lex.type() != TOK_EOL) {
                                charset set;
                                set.type = std::string{lex.lex()};
                                lex.skip(TOK_WORD);
                                if (lex.type() == TOK_SEMI) {
                                        lex.skip(TOK_SEMI);
                                        lex.skip(TOK_WORD);
                                        lex.skip(TOK_EQ);
                                        set.arg = atof(lex.lex().c_str());
                                        lex.skip(TOK_NUM);
                                }
                                if (lex.type() == TOK_COMMA)
                                        lex.skip(TOK_COMMA);
                                req.charsets.push_back(set);
                        }
                }

                lex.skip(TOK_EOL);
        }

        printf("method=%s\n", req.method.c_str());
        printf("path=%s\n", req.path.c_str());
        printf("version=%f\n", req.version);
        printf("Content-Length: %d\n", req.len);

        printf("Accept:\n");
        for (auto m : req.accept) {
                printf("\t%s, %s, %f\n", m.type.c_str(),
                                m.subtype.c_str(), m.arg);
        }

        printf("Accept-Charset:\n");
        for (auto s : req.charsets)
                printf("\t%s, %f\n", s.type.c_str(), s.arg);
}
