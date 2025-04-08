#pragma once

//std
#include <cstdint>
#include <string>
#include <vector>

namespace lib::rpn_parser {
    enum TokenType : uint8_t {
        None   = 0,
        Plus   = '+', Minus  = '-', Star = '*', Slash = '/', Percent = '%',
        Equal  = '=', Less   = '<', More = '>', And   = '&', Or      = '|',
        LParen = '(', RParen = ')',

        LessEq = 0x80, MoreEq   = 0x81,
        Number = 0x82, Variable = 0x83
    };

    struct token_t {
        TokenType type;
        std::string literal;
        double number = 0.0;
    };

    using token_list = std::vector<token_t>;
}

namespace lib {
    using rpn_token_type = rpn_parser::TokenType;
    using rpn_token_t = rpn_parser::token_t;
    using tokenized_rpn_t = std::vector<rpn_token_t>;

    class RpnParser {
    public:
        static rpn_parser::token_list tokenize(std::string_view what);
        static tokenized_rpn_t shunting_yard_algorithm(const rpn_parser::token_list& infix);
    };
}
