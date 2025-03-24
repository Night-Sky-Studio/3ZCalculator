#include "library/rpn.hpp"

//std
#include <charconv>
#include <stack>

//frozen
#include "frozen/unordered_map.h"
#include "frozen/unordered_set.h"

//library
#include "library/format.hpp"
#include "library/string_funcs.hpp"

using namespace lib::rpn_parser;

namespace lib::rpn_details {
    constexpr frozen::unordered_set<TokenType, 14> primitive_tokens = {
        Plus, Minus, Star, Slash, Percent,
        Equal, Less, More, LParen, RParen, And, Or,
        LessEq, MoreEq
    };
    constexpr frozen::unordered_set<TokenType, 12> math_operators = {
        Plus, Minus, Star, Slash, Percent, Less, More, Equal, And, Or, LessEq, MoreEq
    };
    constexpr frozen::unordered_map<TokenType, size_t, 12> precedence = {
        { And, 1 }, { Or, 1 },
        { Plus, 2 }, { Minus, 2 },
        { Star, 3 }, { Slash, 3 }, { Percent, 3 },
        { Less, 4 }, { More, 4 }, { LessEq, 4 }, { MoreEq, 4 },
        { Equal, 5 }
    };

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

namespace lib {
    token_list RpnParser::tokenize(std::string what) {
        token_list result;
        what = remove_chars(what, " \t\n");

        size_t i = 0, di;
        while (i < what.size()) {
            if (what[i] == '<' && what[i + 1] == '=') {
                di = 2;
                result.emplace_back(LessEq, "<=");
            } else if (what[i] == '>' && what[i + 1] == '=') {
                di = 2;
                result.emplace_back(MoreEq, ">=");
            } else if (rpn_details::primitive_tokens.contains((TokenType) what[i])) {
                di = 1;
                result.emplace_back((TokenType) what[i], std::string(1, what[i]));
            } else if ((what[i] >= '0' && what[i] <= '9') || what[i] == '-') {
                token_t token;
                std::tie(token.number, token.literal, di) = rpn_details::parse_number(i, what);
                token.type = Number;
                result.emplace_back(std::move(token));
            } else if (isalpha(what[i])) {
                token_t token;
                std::tie(token.literal, di) = rpn_details::parse_literal(i, what);
                token.type = Variable;
                result.emplace_back(std::move(token));
            }

            i += di;
            di = 0;
        }

        return result;
    }
    rpn_t RpnParser::shunting_yard_algorithm(const token_list& infix) {
        rpn_t rpn;
        std::stack<token_t> stack;

        auto get_precedence = [](TokenType type) {
            auto it = rpn_details::precedence.find(type);
            return it != rpn_details::precedence.end() ? it->second : 0;
        };

        for (auto token : infix) {
            if (token.type == Number || token.type == Variable) {
                rpn.emplace_back(std::move(token));
            } else if (token.type == LParen) { // '('
                stack.emplace(std::move(token));
            } else if (token.type == RParen) { // ')'
                while (!stack.empty() && stack.top().type != LParen) {
                    rpn.emplace_back(std::move(stack.top()));
                    stack.pop();
                }
                // pop '('
                stack.pop();
            } else if (rpn_details::math_operators.contains(token.type)) {
                size_t own_precedence = get_precedence(token.type);
                while (!stack.empty()) {
                    auto& top = stack.top();
                    if (top.type == LParen || own_precedence > get_precedence(top.type))
                        break;

                    rpn.emplace_back(std::move(top));
                    stack.pop();
                }
                stack.push(std::move(token));
            } else
                throw RUNTIME_ERROR("bad infix to rpn parse");
        }

        while (!stack.empty()) {
            if (stack.top().type == LParen)
                break;

            rpn.emplace_back(std::move(stack.top()));
            stack.pop();
        }

        rpn.shrink_to_fit();
        return rpn;
    }
}
