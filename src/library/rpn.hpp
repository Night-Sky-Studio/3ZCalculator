#pragma once

//std
#include <cstdint>
#include <queue>
#include <string>
#include <vector>

namespace lib::rpn_parser {
    enum TokenType : uint8_t {
        None   = 0,
        Plus   = '+', Minus  = '-', Star = '*', Slash = '/', Percent = '%',
        Equal  = '=', Less   = '<', More = '>',
        LParen = '(', RParen = ')',

        Number = 0x80, Variable = 0x81
    };

    struct token_t {
        TokenType type;
        std::string literal;
        double number = 0.0;
    };

    using token_list = std::vector<token_t>;
    using rpn_t = std::queue<token_t>;
}

namespace lib {
    class RpnParser {
    public:
        static rpn_parser::token_list tokenize(std::string what);
        static rpn_parser::rpn_t shunting_yard_algorithm(const rpn_parser::token_list& infix);
    };
}
