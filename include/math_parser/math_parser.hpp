#pragma once
#include <stdexcept>
#include <memory>
#include <optional>
#include <vector>
#include <stack>
#include <queue>
#include <cassert>
#include <cmath>
#include <map>
#include <string>

#include "tokens.hpp"

namespace InputHandling {

    namespace Exception {
        struct MismatchedParenthesis : public std::runtime_error {
          public:
            MismatchedParenthesis() : std::runtime_error{"You have mismatched parenthesis."} {}
        };

        struct UnrecognizedSymbolException : public std::runtime_error {
          public:
            UnrecognizedSymbolException() : std::runtime_error{"Unrecognized symbol is in the equation."} {}
        };

        struct TooManyOperatorsException : public std::runtime_error {
          public:
            TooManyOperatorsException() : std::runtime_error{"You have too many operators."} {}
        };

        struct IncorrectNumberOfArgumentsException : public std::runtime_error {
          public:
            IncorrectNumberOfArgumentsException() : std::runtime_error{"Incorrect number of arguments."} {}
        };
    } // namespace Exception

    namespace _Internal {
        using PToken = std::shared_ptr<Token>;
        struct TokenEvaluator;
        using PTokenEvaluator = std::shared_ptr<TokenEvaluator>;

        constexpr auto to_oprt    = [](PToken pt) { return static_cast<OperatorToken *>(pt.get()); };
        constexpr auto to_opnd    = [](PToken pt) { return static_cast<OperandToken *>(pt.get()); };
        constexpr auto to_func    = [](PToken pt) { return static_cast<FunctionToken *>(pt.get()); };
        constexpr auto is_type_of = [](PToken pt, Type t) -> bool { return pt.get()->type == t; };

        struct ParsedFunction {
            ParsedFunction(std::queue<PToken> tokens);
            double eval(double x, double y) const;

          private:
            PTokenEvaluator evaluator;
        };

        class ShuntingYardAlgorithm {

          public:
            ParsedFunction parse_text_input(std::string input);

          private:
            PToken extract_token(std::string &input);
            PToken find_token_in_map(std::string_view token, std::string *const str = nullptr);
            void remove_spaces(std::string &input);
            void format_minus_signs(std::string &input);

          private:
            static std::map<std::string, PToken, std::greater<>> token_map;

            int expected_number_of_commas = 0u;
        };
    } // namespace _Internal

    inline namespace V2 {
        using _Internal::ParsedFunction;
        using _Internal::ShuntingYardAlgorithm;
        using namespace Exception;
    } // namespace V2

} // namespace InputHandling