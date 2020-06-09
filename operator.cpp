//
// Created by geger on 5/11/2020.
//

#include "operator.h"

namespace compiler {
	std::vector<Operator> operators;
	const Operator possessiveOperator(":", "", false);

	std::vector<std::string>::iterator
	Operator::firstExpr(std::vector<std::string>::iterator begin, std::vector<std::string>::iterator end) const {
		for (auto i = begin; i != end; i++) {
			bool isValid = true;
			for (int j = 0; j < delimTokens.size() && i + j != end; j++) {
				if (*(i + j) != delimTokens[j]) {
					isValid = false;
					break;
				}
			}
			if (isValid && i + delimTokens.size() - 1 != end) {
				return i;
			}
		}
		return end;
	}

	std::vector<std::string>::iterator
	Operator::secondExpr(std::vector<std::string>::iterator begin, std::vector<std::string>::iterator end) const {
		auto result = firstExpr(begin, end);
		if (result == end)
			return end;
		for (int i = 0; i < delimTokens.size(); i++)
			if (++result == end)
				return end;
		return result;
	}

	Operator::Operator(std::string delim, std::string functionName, bool singleSided) {
		this->delim = std::move(delim);
		this->functionName = std::move(functionName);
		this->singleSided = singleSided;
		delimTokens = splitTokens(this->delim.c_str());
	}

	void setupOperators() {
		operators.emplace_back("or", "Or");
		operators.emplace_back("and", "And");
		operators.emplace_back("!=", "NotEquals");
		operators.emplace_back("=", "Equals");
		operators.emplace_back(">", "GreaterThan");
		operators.emplace_back("<", "LessThan");
		operators.emplace_back("-", "Subtract");
		operators.emplace_back("+", "Add");
		operators.emplace_back("/", "Divide");
		operators.emplace_back("*", "Multiply");
		operators.emplace_back("%", "Modulo");
		operators.emplace_back("not", "Not", true);
	}
}