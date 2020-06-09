//
// Created by geger on 5/11/2020.
//

#ifndef SIMPLETHON_OPERATOR_H
#define SIMPLETHON_OPERATOR_H

#include "compiler.h"

namespace compiler {
	class Operator {
	private:
		std::string delim;
		std::string functionName;
		bool singleSided;
		std::vector<std::string> delimTokens;
	public:
		[[nodiscard]] std::vector<std::string>::iterator
		firstExpr(std::vector<std::string>::iterator begin, std::vector<std::string>::iterator end) const;

		[[nodiscard]] std::vector<std::string>::iterator
		secondExpr(std::vector<std::string>::iterator begin, std::vector<std::string>::iterator end) const;

		Operator(std::string delim, std::string functionName, bool singleSided = false);

		[[nodiscard]] inline std::string getFunctionName() const { return functionName; }

		[[nodiscard]] inline bool getSingleSided() const { return singleSided; }
	};

	void setupOperators();

	extern std::vector<Operator> operators;
	extern const Operator possessiveOperator;
}

#endif //SIMPLETHON_OPERATOR_H
