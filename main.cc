#include "lexer.h"
#include <vector>

struct media {
        std::string type;
        std::string subtype;
        float arg;
};

struct charset {
        std::string type;
        float arg;
};

struct encoding {
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
        std::vector<encoding> encodings;
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
                                charset set {"", 0};
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
                } else if (type == TOK_ACCEPT_ENCODING) {
                        while (lex.type() != TOK_EOL) {
                                encoding e {"", 0};
                                e.type = std::string{lex.lex()};
                                lex.skip(TOK_WORD);
                                if (lex.type() == TOK_SEMI) {
                                        lex.skip(TOK_SEMI);
                                        lex.skip(TOK_WORD);
                                        lex.skip(TOK_EQ);
                                        e.arg = atof(lex.lex().c_str());
                                        lex.skip(TOK_NUM);
                                }
                                if (lex.type() == TOK_COMMA)
                                        lex.skip(TOK_COMMA);
                                req.encodings.push_back(e);
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

        printf("Accept-Encoding:\n");
        for (auto e : req.encodings)
                printf("\t%s, %f\n", e.type.c_str(), e.arg);
}
