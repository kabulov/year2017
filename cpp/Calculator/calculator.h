#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <cctype>

class Tokenizer {
public:
    Tokenizer(std::istream* in) : in_(in) {}

    enum TokenType {
        UNKNOWN,
        NUMBER,
        SYMBOL,
        END
    };

    void Consume() {
      if (type_ == END) {
        return;
      }
      if (type_ == UNKNOWN || type_ == SYMBOL) {
        while ((*in_).get(symbol_) && symbol_ == ' '); 
      } else {
        if (symbol_ == ' ') {
          while ((*in_).get(symbol_) && symbol_ == ' '); 
        }
      }  
      if ((*in_).eof()) {
        type_ = END;
        return;
      }   
      if (isdigit(symbol_)) {
        number_ = symbol_ - '0';
        while ((*in_).get(symbol_) && std::isdigit(symbol_)) {
          number_ = 10 * number_ + symbol_ - '0';
        }
        type_ = NUMBER;
      } else {
        type_ = SYMBOL;
      }  
    }

    TokenType GetType() {
      return type_;
    }

    int64_t GetNumber() {
      return number_;
    }

    char GetSymbol() {
      return symbol_;
    }

private:
    std::istream* in_;

    TokenType type_ = TokenType::UNKNOWN;
    int64_t number_;
    char symbol_;
};

class Expression {
public:
    virtual ~Expression() {}
    virtual int64_t Evaluate() = 0;
};

class Number : public Expression {
  public:
    explicit Number(int64_t number) : number_(number) {}
    virtual int64_t Evaluate() override {
      return number_;
    }
    int64_t number_;
};

class Operation : public Expression {
public:
  Operation(std::unique_ptr<Expression> left,
            std::unique_ptr<Expression> right)
    : left_(std::move(left)),
      right_(std::move(right)) {
      }
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;
};

class Addition : public Operation  {
  public:
    explicit Addition(std::unique_ptr<Expression> left,
                      std::unique_ptr<Expression> right)
      : Operation(std::move(left), std::move(right)) 
       
    {}
    virtual int64_t Evaluate() override {
      int64_t left = 0;
      if (left_ != nullptr) {
        left = left_->Evaluate();
      }
      int64_t right = right_->Evaluate();
      return left + right;
    }
};

class Subtraction : public Operation {
  public:
    explicit Subtraction(std::unique_ptr<Expression> left,
                         std::unique_ptr<Expression> right)
      : Operation(std::move(left), std::move(right)) 
    {}
    virtual int64_t Evaluate() override {
      int64_t left = 0;
      if (left_ != nullptr) {
        left = left_->Evaluate();
      }
      int64_t right = right_->Evaluate();
      return left - right;
    }
};


class Multiplication : public Operation {
  public:
    explicit Multiplication(std::unique_ptr<Expression> left,
                            std::unique_ptr<Expression> right)
      : Operation(std::move(left), std::move(right))
    {}
    virtual int64_t Evaluate() override {
      int64_t left = left_->Evaluate();
      int64_t right = right_->Evaluate();
      return left * right;
    }
};

class Division : public Operation {
  public:
    explicit Division(std::unique_ptr<Expression> left,
                      std::unique_ptr<Expression> right)
      : Operation(std::move(left), std::move(right))
    {}
    virtual int64_t Evaluate() override {
      int64_t left = left_->Evaluate();
      int64_t right = right_->Evaluate();
      return left / right;
    }
};

std::unique_ptr<Expression> BuildSum(Tokenizer* tok);

std::unique_ptr<Expression> BuildBase(Tokenizer* tok) {
  std::unique_ptr<Expression> expression(nullptr);
  bool unary_plus = false;
  bool unary_minus = false;
  if (tok->GetType() == Tokenizer::SYMBOL) {
    char sym = tok->GetSymbol();
    if (sym == '+') {
      unary_plus = true;      
      tok->Consume(); 
    } else
    if (sym == '-') {
      unary_minus = true;
      tok->Consume();
    }
  }
  if (tok->GetType() == Tokenizer::NUMBER) {
    expression.reset(new Number(tok->GetNumber()));
    tok->Consume();
  } else {
    // open brace
    tok->Consume();
    expression = std::move(BuildSum(tok));
    tok->Consume();
  }
  if (unary_plus) {
    std::unique_ptr<Expression> new_expression(new Addition(nullptr, std::move(expression)));
    expression = std::move(new_expression);
  }
  if (unary_minus) {
    std::unique_ptr<Expression> new_expression(new Subtraction(nullptr, std::move(expression)));
    expression = std::move(new_expression);
  }
  return expression;
}

std::unique_ptr<Expression> BuildMul(Tokenizer* tok) {
  std::unique_ptr<Expression> expression(std::move(BuildBase(tok)));
  
  while (true) {
    if (tok->GetType() == Tokenizer::END) {
      break;
    } else {
      char sym = tok->GetSymbol();
      if (sym == ')' || sym == '+' || sym == '-') {
        break;
      }  
      tok->Consume();
      std::unique_ptr<Expression> right_value(std::move(BuildBase(tok)));
      std::unique_ptr<Operation> operation(nullptr);
      if (sym == '/') {
        operation.reset(new Division(std::move(expression), std::move(right_value))); 
      } else
      if (sym == '*') {
        operation.reset(new Multiplication(std::move(expression), std::move(right_value)));
      } else {
        throw std::runtime_error("wtf");
      }
      expression = std::move(operation);
    }  
  }

  return expression;  
}

std::unique_ptr<Expression> BuildSum(Tokenizer* tok) {
  std::unique_ptr<Expression> expression(std::move(BuildMul(tok)));
 
  while (true) {
    if (tok->GetType() == Tokenizer::END) {
      break;
    } else {
      char sym = tok->GetSymbol();
      if (sym == ')') {
        break;
      }
      tok->Consume();
      std::unique_ptr<Expression> right_value(std::move(BuildMul(tok)));
      std::unique_ptr<Operation> operation(nullptr);
      if (sym == '-') {
        operation.reset(new Subtraction(std::move(expression), std::move(right_value)));
      } else
      if (sym == '+') {
        operation.reset(new Addition(std::move(expression), std::move(right_value)));
      } 
      expression = std::move(operation);
    }
  } 
  
  return expression;  
}

std::unique_ptr<Expression> ParseExpression(Tokenizer* tok) {
  tok->Consume();
  std::unique_ptr<Expression> expression = std::move(BuildSum(tok)); 
  return expression;
}
