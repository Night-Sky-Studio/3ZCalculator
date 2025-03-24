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
    using rpn_t = std::vector<rpn_parser::token_t>;

    class RpnParser {
    public:
        static rpn_parser::token_list tokenize(std::string what);
        static rpn_t shunting_yard_algorithm(const rpn_parser::token_list& infix);
    };
}
