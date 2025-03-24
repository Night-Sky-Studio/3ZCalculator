#pragma once

//std
#include <cstdint>
#include <queue>
#include <string>
#include <vector>

namespace math::parser {
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

namespace math {
    class Parser {
    public:
        static parser::token_list tokenize(std::string what);
        static parser::rpn_t shunting_yard_algorithm(const parser::token_list& infix);
    };
}
