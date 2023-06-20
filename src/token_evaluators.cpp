#include "../include/math_parser/token_evaluators.hpp"

using namespace InputHandling::_Internal;

InputHandling::_Internal::TokenFunctionEvaluator::TokenFunctionEvaluator(PToken func,
                                                                         std::vector<PTokenEvaluator> &&arguments) {
    assert(*func == Type::Function);
    assert(arguments.size() == to_func(func)->num_args);

    this->token = func;
    args        = std::move(arguments);
}

double InputHandling::_Internal::TokenFunctionEvaluator::eval(double x, double y) const {
    switch (to_func(token)->func) {
    case Function::Sin: return sin(args[0]->eval(x, y));
    case Function::Cos: return cos(args[0]->eval(x, y));
    case Function::Tan: return tan(args[0]->eval(x, y));
    case Function::Sec: return 1.0 / cos(args[0]->eval(x, y));
    case Function::Csc: return 1.0 / sin(args[0]->eval(x, y));
    case Function::Cot: return 1.0 / tan(args[0]->eval(x, y));

    case Function::Sinh: return sinh(args[0]->eval(x, y));
    case Function::Cosh: return cosh(args[0]->eval(x, y));
    case Function::Tanh: return cosh(args[0]->eval(x, y));
    case Function::Sech: return 1.0 / cosh(args[0]->eval(x, y));
    case Function::Csch: return 1.0 / sinh(args[0]->eval(x, y));
    case Function::Coth: return 1.0 / tanh(args[0]->eval(x, y));

    case Function::Asin: return asin(args[0]->eval(x, y));
    case Function::Acos: return acos(args[0]->eval(x, y));
    case Function::Atan: return atan(args[0]->eval(x, y));

    case Function::Asinh: return asinh(args[0]->eval(x, y));
    case Function::Acosh: return acosh(args[0]->eval(x, y));
    case Function::Atanh: return atanh(args[0]->eval(x, y));

    case Function::Ln: return log(args[0]->eval(x, y));
    case Function::Log: return log10(args[0]->eval(x, y));
    case Function::Abs: return abs(args[0]->eval(x, y));
    case Function::Exp: return exp(args[0]->eval(x, y));
    case Function::Sign: {
        const auto evalres = args[0]->eval(x, y);
        return (evalres > 0.) - (evalres - 0.);
    }
    case Function::Floor: return floor(args[0]->eval(x, y));
    case Function::Ceil: return ceil(args[0]->eval(x, y));
    case Function::Trunc: return trunc(args[0]->eval(x, y));
    case Function::Fract: {
        float whole;
        double fract = modf(args[0]->eval(x, y), &whole);
        return fract;
    }

    case Function::Max: return std::max<double>(args[0]->eval(x, y), args[1]->eval(x, y));
    case Function::Min: return std::min<double>(args[0]->eval(x, y), args[1]->eval(x, y));
    case Function::Step: {
        const auto edge = args[0]->eval(x, y);
        const auto arg  = args[1]->eval(x, y);
        return arg < edge ? 0.0 : 1.0;
    }

    case Function::Clamp: return std::clamp(args[0]->eval(x, y), args[1]->eval(x, y), args[2]->eval(x, y));
    case Function::Mix: return std::lerp(args[0]->eval(x, y), args[1]->eval(x, y), args[2]->eval(x, y));
    case Function::Smoothstep: {
        const auto e1  = args[0]->eval(x, y);
        const auto e2  = args[1]->eval(x, y);
        const auto arg = args[2]->eval(x, y);

        const double t = std::clamp((arg - e1) / (e2 - e1), 0., 1.);
        return t * t * (3. - 2. * t);
    }
    }

    assert(("Unsupported type of function." && false));
}

std::shared_ptr<TokenEvaluator>
InputHandling::_Internal::TokenEvaluatorFactory::build(PToken token, std::stack<PTokenEvaluator> &rpn_stack) {
    switch (token->type) {
    case Type::Function: {

        std::vector<PTokenEvaluator> arguments;
        while (arguments.size() < to_func(token)->num_args) {
            if (rpn_stack.empty()) { throw Exception::IncorrectNumberOfArgumentsException{}; }

            arguments.insert(arguments.begin(), rpn_stack.top());
            rpn_stack.pop();
        }

        return std::make_shared<TokenFunctionEvaluator>(token, std::move(arguments));
    }
    case Type::Operand: return std::make_shared<TokenOperandEvaluator>(token);
    case Type::Operator: {

        if (rpn_stack.size() < 2) { throw Exception::TooManyOperatorsException{}; }

        auto arg1 = rpn_stack.top();
        rpn_stack.pop();

        auto arg2 = rpn_stack.top();
        rpn_stack.pop();
        return std::make_shared<TokenOperatorEvaluator>(token, arg1, arg2);
    }
    }
}

InputHandling::_Internal::TokenOperatorEvaluator::TokenOperatorEvaluator(PToken op,
                                                                         PTokenEvaluator op1,
                                                                         PTokenEvaluator op2) {
    assert(*op == Type::Operator);

    this->token = op;
    this->op1   = op2;
    this->op2   = op1;
}

double InputHandling::_Internal::TokenOperatorEvaluator::eval(double x, double y) const {
    switch (to_oprt(token)->op) {
    case Operator::Add: return op1->eval(x, y) + op2->eval(x, y);
    case Operator::Sub: return op1->eval(x, y) - op2->eval(x, y);
    case Operator::Mul: return op1->eval(x, y) * op2->eval(x, y);
    case Operator::Div: return op1->eval(x, y) / op2->eval(x, y);
    case Operator::Pow: return pow(op1->eval(x, y), op2->eval(x, y));
    default: assert(false); return 0.;
    }
}

InputHandling::_Internal::TokenOperandEvaluator::TokenOperandEvaluator(PToken op) {
    assert(*op == Type::Operand);
    this->token = op;
}

double InputHandling::_Internal::TokenOperandEvaluator::eval(double x, double y) const {
    switch (to_opnd(token)->o) {
    case Operand::X: return x;
    case Operand::Y: return y;
    case Operand::Number: return to_opnd(token)->number.value();
    }

    assert(("Execution should never reach this" && false));
    return -1.f;
}
