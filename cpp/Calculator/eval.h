#pragma once

#include <calculator.h>
#include <string>
#include <sstream>

int64_t EvaluateExpression(const std::string& expression) {
    std::stringstream ss(expression);
    Tokenizer tok(&ss);
    auto expr = ParseExpression(&tok);
    return expr->Evaluate();
}
