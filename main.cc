#include "lexer.h"
#include <vector>
#include <unordered_set>

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

struct language {
        std::string type;
        float arg;
};

struct cache_dir {
        std::string type;
        std::string s_arg;
        float n_arg {0};
};

struct cache_ctl {
        cache_dir reqdir;
        cache_dir resdir;
};

struct request {
        std::string method;
        std::string path;
        float version;
        int len;
        std::vector<media> accept;
        std::vector<charset> charsets;
        std::vector<encoding> encodings;
        std::vector<language> langs;
        std::string auth;
        std::string connect;
        std::string ctnt_encoding;
        cache_ctl cache;
        std::vector<language> ctnt_langs;
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
                } else if (type == TOK_ACCEPT_LANGUAGE) {
                        while (lex.type() != TOK_EOL) {
                                language l {"", 0};
                                l.type = std::string{lex.lex()};
                                lex.skip(TOK_WORD);
                                if (lex.type() == TOK_SEMI) {
                                        lex.skip(TOK_SEMI);
                                        lex.skip(TOK_WORD);
                                        lex.skip(TOK_EQ);
                                        l.arg = atof(lex.lex().c_str());
                                        lex.skip(TOK_NUM);
                                }
                                if (lex.type() == TOK_COMMA)
                                        lex.skip(TOK_COMMA);
                                req.langs.push_back(l);
                        }
                } else if (type == TOK_AUTHORIZATION) {
                        req.auth = std::string{lex.lex()};
                        lex.skip(TOK_WORD);
                } else if (type == TOK_CACHE_CONTROL) {
                        cache_dir reqdir;
                        reqdir.n_arg = 0;
                        reqdir.s_arg = "";
                        reqdir.type = std::string{lex.lex()};
                        lex.skip(TOK_WORD);
                        if (lex.type() == TOK_EQ) {
                                lex.skip(TOK_EQ);
                                if (lex.type() == TOK_NUM) {
                                        reqdir.n_arg = atof(lex.lex().c_str());
                                } else {
                                        reqdir.s_arg = std::string{lex.lex()};
                                }
                                lex.skip(lex.type());
                        }

                        cache_dir resdir;
                        resdir.type = "";
                        resdir.n_arg = 0;
                        resdir.s_arg = "";
                        if (lex.type() == TOK_COMMA) {
                                lex.skip(TOK_COMMA);
                                resdir.type = std::string{lex.lex()};
                                lex.skip(TOK_WORD);
                                if (lex.type() == TOK_EQ) {
                                        lex.skip(TOK_EQ);
                                        if (lex.type() == TOK_NUM) {
                                                resdir.n_arg = atof(
                                                        lex.lex().c_str());
                                        } else {
                                                resdir.s_arg = std::string{
                                                        lex.lex()};
                                        }
                                        lex.skip(lex.type());
                                }
                        }

                        req.cache.resdir = resdir;
                        req.cache.reqdir = reqdir;
                } else if (type == TOK_CONNECTION) {
                        req.connect = std::string{lex.lex()};
                        lex.skip(TOK_WORD);
                } else if (type == TOK_CONTENT_ENCODING) {
                        req.ctnt_encoding = std::string{lex.lex()};
                        lex.skip(TOK_WORD);
                } else if (type == TOK_CONTENT_LANGUAGE) {
                        while (lex.type() != TOK_EOL) {
                                language lang {"", 0};
                                lang.type = std::string{lex.lex()};
                                req.ctnt_langs.push_back(lang);
                                lex.skip(TOK_WORD);
                                if (lex.type() == TOK_COMMA)
                                        lex.skip(TOK_COMMA);
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

        printf("Accept-Language:\n");
        for (auto l : req.langs)
                printf("\t%s, %f\n", l.type.c_str(), l.arg);

        printf("Authorization:\n\t%s\n", req.auth.c_str());

        printf("Cache-Control:\n");
        auto type = req.cache.reqdir.type;
        printf("\t%s", type.c_str());
        std::unordered_set<std::string> reqdirmap {
                "no-cache",
                "no-store",
                "max-age",
                "max-stale",
                "min-fresh",
                "no-transform",
                "only-if-cached",
        };
        auto p = reqdirmap.find(type);
        if (p != reqdirmap.end()) {
                if (type == "max-age" || type == "min-fresh")
                        printf(", %f", req.cache.reqdir.n_arg);
                if (type == "max-stale" && req.cache.reqdir.n_arg != 0)
                        printf(", %f", req.cache.reqdir.n_arg);
        } else {
                if (req.cache.reqdir.s_arg.size() != 0)
                        printf(", %s", req.cache.reqdir.s_arg.c_str());
                if (req.cache.reqdir.n_arg != 0)
                        printf(", %f", req.cache.reqdir.n_arg);
        }
        printf("\n");

        type = req.cache.resdir.type;
        printf("\t%s", type.c_str());
        std::unordered_set<std::string> resdirmap {
                "public",
                "private",
                "no-cache",
                "no-store",
                "no-transform",
                "must-revalidate",
                "proxy-revalidate",
                "max-age",
                "s-maxage",

        };
        p = resdirmap.find(type);
        if (p != resdirmap.end()) {
                if (type == "max-age" || type == "s-maxage")
                        printf(", %f", req.cache.resdir.n_arg);
                if ((type == "private" || type == "no-cache") &&
                        req.cache.resdir.s_arg.size() != 0) {
                        printf(", %s", req.cache.resdir.s_arg.c_str());
                }
        } else {
                if (req.cache.resdir.s_arg.size() != 0)
                        printf(", %s", req.cache.resdir.s_arg.c_str());
                if (req.cache.resdir.n_arg != 0)
                        printf(", %f", req.cache.resdir.n_arg);
        }
        printf("\n");

        printf("Connection:\n");
        printf("\t%s\n", req.connect.c_str());

        printf("Content-Encoding:\n");
        printf("\t%s\n", req.ctnt_encoding.c_str());

        printf("Content-Language:\n");
        for (auto l : req.ctnt_langs)
                printf("\t%s\n", l.type.c_str());
}
