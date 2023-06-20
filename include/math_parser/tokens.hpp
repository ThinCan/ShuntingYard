#pragma once

#include <optional>
#include <cstdint>

namespace InputHandling {
    namespace _Internal {
        enum class Function {
            Sin,
            Cos,
            Tan,
            Sec,
            Csc,
            Cot,
            Asin,
            Acos,
            Atan,
            Sinh,
            Cosh,
            Tanh,
            Sech,
            Csch,
            Coth,
            Asinh,
            Acosh,
            Atanh,
            Max,
            Min,
            Log,
            Ln,
            Abs,
            Exp,
            Sign,
            Floor,
            Ceil,
            Trunc,
            Fract,
            Clamp,
            Mix,
            Step,
            Smoothstep
        };

        enum class Operator {
            OpeningParenthesis,
            ClosingParenthesis,
            Comma,
            Add,
            Sub,
            Mul,
            Div,
            Pow,
        };
        enum class Operand { Number, X, Y };
        enum class Type { Function, Operator, Operand };

        struct Token {
            bool operator==(Type t) const { return t == type; }

            Type type;
        };

        struct FunctionToken : public Token {
            FunctionToken(Function f, uint32_t num_args) : num_args{num_args}, func{f} { type = Type::Function; }

            uint32_t num_args = 0;
            Function func;
        };

        struct OperatorToken : public Token {
            OperatorToken(Operator o) : op{o} {
                type = Type::Operator;

                switch (o) {
                case Operator::Add:
                case Operator::Sub: precedence = 2u; break;
                case Operator::Mul:
                case Operator::Div: precedence = 3u; break;
                case Operator::Pow:
                    precedence       = 4u;
                    left_associative = false;
                    break;
                default: break;
                }
            }

            Operator op;
            uint32_t precedence{0u};
            bool left_associative = true;
        };

        struct OperandToken : public Token {
            OperandToken(Operand o) : o{o} { type = Type::Operand; }
            OperandToken(Operand o, double number) : o{o} {
                this->number = number;
                type         = Type::Operand;
            }

            Operand o;
            std::optional<double> number;
        };
    } // namespace _Internal
} // namespace InputHandling
