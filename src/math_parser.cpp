#include "../include/math_parser/math_parser.hpp"
#include "../include/math_parser/tokens.hpp"
#include "../include/math_parser/token_evaluators.hpp"

namespace InputHandling {
    namespace _Internal {

        template <typename T> inline PToken make_ptoken(T &&t) requires std::is_base_of_v<Token, T> {
            return std::make_shared<T>(std::move(t));
        }

        ParsedFunction::ParsedFunction(std::queue<PToken> tokens) {
            std::stack<PTokenEvaluator> s;
            TokenEvaluatorFactory token_eval_factory;

            while (tokens.empty() == false) {
                auto token = tokens.front();
                tokens.pop();

                auto newtoken = token_eval_factory.build(token, s);
                s.push(newtoken);
            }

            if (s.size() > 1) { throw Exception::UnrecognizedSymbolException{}; }

            evaluator = s.top();
            s.pop();
        }
        double ParsedFunction::eval(double x, double y) const { return evaluator->eval(x, y); }

    } // namespace _Internal

} // namespace InputHandling

InputHandling::_Internal::ParsedFunction
InputHandling::_Internal::ShuntingYardAlgorithm::parse_text_input(std::string input) {
    std::stack<PToken> ops;
    std::queue<PToken> tokens;

    remove_spaces(input);
    format_minus_signs(input);

    PToken previous_token;
    while (input.size() > 0) {
        auto token = extract_token(input);

        if (is_type_of(token, Type::Operator) && to_oprt(token)->op == Operator::Comma) {
            --expected_number_of_commas;
            continue;
        }
        if (is_type_of(token, Type::Function) && to_func(token)->num_args > 1) {
            expected_number_of_commas += to_func(token)->num_args - 1;
        }

        if (is_type_of(token, Type::Operand)) {
            tokens.push(token);
        } else if (is_type_of(token, Type::Function)) {
            ops.push(token);
        } else if (is_type_of(token, Type::Operator) && to_oprt(token)->op == Operator::OpeningParenthesis) {
            ops.push(token);
        } else if (is_type_of(token, Type::Operator) && to_oprt(token)->op == Operator::ClosingParenthesis) {
            while (ops.empty() == false) {
                if (is_type_of(token, Type::Operator) && to_oprt(ops.top())->op == Operator::OpeningParenthesis) {
                    break;
                }

                if (ops.size() == 1) { throw Exception::MismatchedParenthesis{}; }

                tokens.push(ops.top());
                ops.pop();
            }

            if (ops.empty() == true || is_type_of(ops.top(), Type::Operator) == false
                || to_oprt(ops.top())->op != Operator::OpeningParenthesis) {
                throw Exception::MismatchedParenthesis{};
            }

            ops.pop();

            if (ops.empty() == false && is_type_of(ops.top(), Type::Function)) {
                tokens.push(ops.top());
                ops.pop();
            }
        } else if (*token == Type::Operator) {
            while (ops.empty() == false && ops.top()->type == Type::Operator) {
                const auto op1 = to_oprt(token);
                const auto op2 = to_oprt(ops.top());

                if (op2->op == Operator::OpeningParenthesis) { break; }
                if (op1->precedence > op2->precedence) { break; }
                if (op1->precedence == op2->precedence && op1->left_associative == false) { break; }

                tokens.push(ops.top());
                ops.pop();
            }

            ops.push(token);
        }

        previous_token = token;
    }

    while (ops.empty() == false) {
        if (is_type_of(ops.top(), Type::Operator) && to_oprt(ops.top())->op == Operator::OpeningParenthesis) {

            throw Exception::MismatchedParenthesis{};
        }

        tokens.push(ops.top());
        ops.pop();
    }

    if (expected_number_of_commas != 0u) { throw Exception::IncorrectNumberOfArgumentsException{}; }

    return ParsedFunction{tokens};
}

namespace InputHandling::_Internal {
    PToken ShuntingYardAlgorithm::extract_token(std::string &input) {
        assert(("Input string is empty." && input.empty() == false));

        if (isdigit(input.at(0))) {
            std::string number{input.at(0)};
            input.erase(0, 1);
            const auto pos = input.find_first_not_of("0123456789.");
            if (pos != std::string::npos) {
                number.append(input.begin(), input.begin() + pos);
                input.erase(0, pos);
            } else {
                number.append(input.begin(), input.end());
                input.erase(0, input.size());
            }

            return make_ptoken(OperandToken{Operand::Number, std::stof(number)});
        }

        std::string matched_symbol;
        auto token = find_token_in_map(input, &matched_symbol);

        if (token) {
            printf("[Debug: Matched symbol]: %s\n", matched_symbol.c_str());
            input.erase(0, matched_symbol.size());
            return token;
        } else {
            throw Exception::UnrecognizedSymbolException{};
        }

        assert(("Execution should never reach here" && false));
        return nullptr;
    }
    PToken ShuntingYardAlgorithm::find_token_in_map(std::string_view token, std::string *const str) {
        for (const auto &[k, v] : token_map) {
            if (token.starts_with(k)) {
                if (str) { *str = k.c_str(); }

                return v;
            }
        }

        return nullptr;
    }
    void ShuntingYardAlgorithm::remove_spaces(std::string &input) {
        input.erase(std::remove_if(input.begin(), input.end(), std::isspace), input.end());
    }
    void ShuntingYardAlgorithm::format_minus_signs(std::string &input) {

        enum class State { NotStarted, Number, Function };
        State s = State::NotStarted;
        std::string::size_type start;
        int function_paren_balance{0};
        for (std::string::size_type i = 0; i < input.size(); ++i) {
            const auto c = input.at(i);

            switch (s) {
            case State::NotStarted:
                if (c == '-' && (i == 0 || (i > 0 && isdigit(input.at(i - 1)) == false))) {
                    start = i;
                    if (isdigit(input.at(i + 1)) || input.at(i + 1) == 'x' || input.at(i + 1) == 'y') {
                        s = State::Number;
                    } else {
                        s = State::Function;
                    }
                    continue;
                }
                break;
            case State::Number: {
                if (i == input.size() - 1) {
                    ++i;
                } else if (isdigit(c) || c == '.' || c == 'x' || c == 'y') {
                    continue;
                }

                std::string new_substring{"(0"};
                new_substring.append(input.begin() + start, input.begin() + i);
                new_substring.append(")");
                input.replace(input.begin() + start, input.begin() + i, "");
                input.insert(start, new_substring);
                i = start + new_substring.length();
                s = State::NotStarted;
                continue;
            }

            case State::Function: {
                bool reached_last_paren = false;
                if (c == '(') {
                    ++function_paren_balance;
                } else if (c == ')') {
                    --function_paren_balance;
                    if (function_paren_balance == 0) {
                        reached_last_paren = true;
                    } else if (function_paren_balance < 0) {
                        throw Exception::MismatchedParenthesis{};
                    }
                }

                if (reached_last_paren) {
                    std::string new_substring{"((0-1)*"};
                    new_substring.append(input.begin() + start + 1, input.begin() + i + 1);
                    new_substring.append("))");
                    input.replace(start, i, new_substring);
                    i = start + new_substring.length();
                    s = State::NotStarted;
                    continue;
                }
            }
            }
        }
    }

    std::map<std::string, PToken, std::greater<>> ShuntingYardAlgorithm::token_map = {
        {"(", make_ptoken(OperatorToken{Operator::OpeningParenthesis})},
        {")", make_ptoken(OperatorToken{Operator::ClosingParenthesis})},
        {"+", make_ptoken(OperatorToken{Operator::Add})},
        {"-", make_ptoken(OperatorToken{Operator::Sub})},
        {"*", make_ptoken(OperatorToken{Operator::Mul})},
        {"/", make_ptoken(OperatorToken{Operator::Div})},
        {"^", make_ptoken(OperatorToken{Operator::Pow})},
        {",", make_ptoken(OperatorToken{Operator::Comma})},

        {"x", make_ptoken(OperandToken{Operand::X})},
        {"y", make_ptoken(OperandToken{Operand::Y})},

        {"sin", make_ptoken(FunctionToken{Function::Sin, 1})},
        {"cos", make_ptoken(FunctionToken{Function::Cos, 1})},
        {"tan", make_ptoken(FunctionToken{Function::Tan, 1})},
        {"sec", make_ptoken(FunctionToken{Function::Sec, 1})},
        {"csc", make_ptoken(FunctionToken{Function::Csc, 1})},
        {"cot", make_ptoken(FunctionToken{Function::Cot, 1})},

        {"sinh", make_ptoken(FunctionToken{Function::Sinh, 1})},
        {"cosh", make_ptoken(FunctionToken{Function::Cosh, 1})},
        {"tanh", make_ptoken(FunctionToken{Function::Tanh, 1})},
        {"sech", make_ptoken(FunctionToken{Function::Sech, 1})},
        {"csch", make_ptoken(FunctionToken{Function::Csch, 1})},
        {"coth", make_ptoken(FunctionToken{Function::Coth, 1})},

        {"asin", make_ptoken(FunctionToken{Function::Asin, 1})},
        {"acos", make_ptoken(FunctionToken{Function::Acos, 1})},
        {"atan", make_ptoken(FunctionToken{Function::Atan, 1})},

        {"asinh", make_ptoken(FunctionToken{Function::Asinh, 1})},
        {"acosh", make_ptoken(FunctionToken{Function::Acosh, 1})},
        {"atanh", make_ptoken(FunctionToken{Function::Atanh, 1})},

        {"ln", make_ptoken(FunctionToken{Function::Ln, 1})},
        {"abs", make_ptoken(FunctionToken{Function::Abs, 1})},
        {"exp", make_ptoken(FunctionToken{Function::Exp, 1})},
        {"log", make_ptoken(FunctionToken{Function::Log, 1})},
        {"sign", make_ptoken(FunctionToken{Function::Sign, 1})},
        {"floor", make_ptoken(FunctionToken{Function::Floor, 1})},
        {"ceil", make_ptoken(FunctionToken{Function::Ceil, 1})},
        {"trunc", make_ptoken(FunctionToken{Function::Trunc, 1})},
        {"fract", make_ptoken(FunctionToken{Function::Fract, 1})},

        {"max", make_ptoken(FunctionToken{Function::Max, 2})},
        {"min", make_ptoken(FunctionToken{Function::Min, 2})},
        {"step", make_ptoken(FunctionToken{Function::Step, 2})},

        {"clamp", make_ptoken(FunctionToken{Function::Clamp, 3})},
        {"mix", make_ptoken(FunctionToken{Function::Mix, 3})},
        {"smoothstep", make_ptoken(FunctionToken{Function::Smoothstep, 3})},
    };
} // namespace InputHandling::_Internal
