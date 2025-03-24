#include "math/parser.hpp"

//std
#include <charconv>
#include <stack>

//frozen
#include "frozen/unordered_map.h"
#include "frozen/unordered_set.h"

//library
#include "library/format.hpp"
#include "library/string_funcs.hpp"

using namespace math::parser;

namespace math::details {
    constexpr frozen::unordered_set<TokenType, 10> primitive_tokens = {
        Plus, Minus, Star, Slash, Percent,
        Equal, Less, More, LParen, RParen
    };
    constexpr frozen::unordered_set<TokenType, 8> math_operators = {
        Plus, Minus, Star, Slash, Percent, Less, More, Equal
    };
    constexpr frozen::unordered_map<TokenType, size_t, 8> precedence = {
        { Plus, 1 }, { Minus, 1 },
        { Star, 2 }, { Slash, 2 }, { Percent, 2 },
        { Less, 3 }, { More, 3 }, { Equal, 4 }
    };

    bool is_floating(size_t index, const std::string& src) {
        bool result = true;
        char c;

        do {
            c = src[index];
            index++;

            if (c == '.')
                result = true;
        } while (isdigit(c) || c == '-');

        return result;
    }

    std::tuple<double, std::string, size_t> parse_number(size_t index, const std::string& src) {
        double number;
        auto [ptr, ec] = std::from_chars(src.data() + index, src.data() + src.size(), number);

        if (ec != std::errc())
            throw FMT_RUNTIME_ERROR("bad number parse: {}", (size_t) ec);

        return { number, std::string(src.data() + index, ptr), (uintptr_t) ptr - ((uintptr_t) src.data() + index) };
    }
    std::tuple<std::string, size_t> parse_literal(size_t start, const std::string& src) {
        // we already know that index is first alpha character
        size_t i = start + 1;

        while (isalnum(src[i]) || src[i] == '_')
            i++;

        return { std::string(src.data() + start, src.data() + i), i - start };
    }
}

namespace math {
    token_list Parser::tokenize(std::string what) {
        token_list result;
        what = lib::remove_chars(what, " \t\n");

        size_t i = 0, di;
        while (i < what.size()) {
            char c = what[i];

            if (details::primitive_tokens.contains((TokenType) c)) {
                di = 1;
                result.emplace_back((TokenType) c, std::string(1, c));
            } else if ((c >= '0' && c <= '9') || c == '-') {
                token_t token;
                std::tie(token.number, token.literal, di) = details::parse_number(i, what);
                token.type = Number;
                result.emplace_back(std::move(token));
            } else if (isalpha(c)) {
                token_t token;
                std::tie(token.literal, di) = details::parse_literal(i, what);
                token.type = Variable;
                result.emplace_back(std::move(token));
            }

            i += di;
            di = 0;
        }

        return result;
    }
    rpn_t Parser::shunting_yard_algorithm(const token_list& infix) {
        rpn_t queue;
        std::stack<token_t> stack;

        auto get_precedence = [](TokenType type) {
            auto it = details::precedence.find(type);
            return it != details::precedence.end() ? it->second : 0;
        };

        for (auto token : infix) {
            if (token.type == Number || token.type == Variable) {
                queue.emplace(std::move(token));
            } else if (token.type == LParen) { // '('
                stack.emplace(std::move(token));
            } else if (token.type == RParen) { // ')'
                while (!stack.empty() && stack.top().type != LParen) {
                    queue.push(std::move(stack.top()));
                    stack.pop();
                }
                // pop '('
                stack.pop();
            } else if (details::math_operators.contains(token.type)) {
                size_t own_precedence = get_precedence(token.type);
                while (!stack.empty()) {
                    auto& top = stack.top();
                    if (top.type == LParen || own_precedence > get_precedence(top.type))
                        break;

                    queue.push(std::move(top));
                    stack.pop();
                }
                stack.push(std::move(token));
            } else
                throw RUNTIME_ERROR("bad infix to rpn parse");
        }

        while (!stack.empty()) {
            if (stack.top().type == LParen)
                break;

            queue.emplace(std::move(stack.top()));
            stack.pop();
        }

        return queue;
    }
}
