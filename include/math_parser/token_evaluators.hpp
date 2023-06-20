#pragma once

#include "../include/math_parser/math_parser.hpp"

namespace InputHandling {
    namespace _Internal {
        struct TokenEvaluator {

            const std::weak_ptr<Token> get_token() const { return token; }

            virtual double eval(double, double) const = 0;
            virtual ~TokenEvaluator(){};

          protected:
            PToken token;
        };

        struct TokenOperandEvaluator : public TokenEvaluator {
            TokenOperandEvaluator() = default;
            TokenOperandEvaluator(PToken op);

            double eval(double x, double y) const final;
        };

        struct TokenOperatorEvaluator : public TokenEvaluator {
            TokenOperatorEvaluator(PToken op, PTokenEvaluator op1, PTokenEvaluator op2);

            double eval(double x, double y) const final;

          private:
            PTokenEvaluator op1, op2;
        };

        struct TokenFunctionEvaluator : public TokenEvaluator {
            TokenFunctionEvaluator() = default;
            TokenFunctionEvaluator(PToken func, std::vector<PTokenEvaluator> &&arguments);

            double eval(double x, double y) const final;

          private:
            std::vector<PTokenEvaluator> args;
        };

        struct TokenEvaluatorFactory {
            std::shared_ptr<TokenEvaluator> build(PToken token, std::stack<PTokenEvaluator> &rpn_stack);
        };
    } // namespace _Internal
} // namespace InputHandling