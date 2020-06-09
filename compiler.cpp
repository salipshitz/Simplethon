//
// Created by geger on 5/11/2020.
//

#include "compiler.h"
#include "operator.h"
#include <map>
#include <array>
#include <sstream>

namespace compiler {
	FileReader *fr = nullptr;

	const char *TOK = R"("[^"]*"|[+-]?([0-9]*[.])?[0-9]+|'s|\w[\w\d]*|\S)";

	int openParentheses;

	std::string needsEnd;

	std::vector<std::string> lineTokens;

	std::vector<std::string> runtimeArgs;

	std::vector<std::string> runtimeMethods;

	int returners;

	std::string expression(std::vector<std::string>::iterator &start, std::vector<std::string>::iterator end);

	std::vector<std::string> splitTokens(const char *input) {
		std::regex rgx(TOK);
		std::string s = input;
		std::vector<std::string> toks;
		for (std::smatch match; regex_search(s, match, rgx); s = match.suffix().str())
			toks.push_back(match.str());
		return toks;
	}

	bool skipWhitespace(std::vector<std::string>::iterator &iter, std::vector<std::string>::iterator end) {
//		while (iter != end && isWhitespace(iter)) iter++;
		return iter != end;
	}

	std::string getTypeName(std::vector<std::string>::iterator &iter, std::vector<std::string>::iterator end) {
		std::string typeName;
		std::vector<std::string> subtypes;
		bool templ = false;
		for (; iter != end && *iter != ")" && *iter != "," && *iter != ";" && *iter != ":"; iter++)
			if (*iter == "(") {
				templ = true;
				for (iter++; iter != end && *iter != ")" && *iter != "," && *iter != ";" && *iter != ":"; iter++)
					subtypes.push_back(getTypeName(iter, end));
				iter--;
			} else if (iter->front() >= 'A' && iter->front() <= 'Z') {
				typeName = getIdentifier(iter, end);
				if (iter == end || *iter == ")" || *iter == "," || *iter == ";" || *iter == ":")
					break;
				else if (*iter == "(")
					iter--;
			}
		if (typeName.empty()) {
			std::cout << "Syntax Error: Type name cannot be empty." << std::endl;
			exit(1);
		}
		if (!subtypes.empty()) {
			typeName += "<";
			for (const auto &type : subtypes)
				typeName += type + ",";
			typeName.pop_back();
			typeName += ">";
		} else if (templ)
			typeName += "<>";

		return "_ptr<" + typeName + ">";
	}

	bool isWhitespace(std::vector<std::string>::iterator iter) {
		for (char ch : *iter)
			if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r')
				return false;
		return true;
	}

	std::string block(const std::string &type, const std::string &functionCall, std::string *outBlock,
	                  std::string *returnType = nullptr, std::string *outElse = nullptr,
	                  std::string *header = nullptr) {
		std::string prefix = header == nullptr ? "" : functionCall;
		std::string result;
		if (returnType != nullptr && !returnType->empty() && *returnType != "void ") {
			outBlock->append(*returnType + "result;");
			returners++;
		}
		while (fr->hasNextLine()) {
			fr->nextLine();
			auto tokStart = lineTokens.begin();
			if (!skipWhitespace(tokStart, lineTokens.end()))
				continue;

			if (*tokStart == "End") {
				tokStart++;
				if (!skipWhitespace(tokStart, lineTokens.end()))
					goto endblock;
				if (*tokStart == ";" || *tokStart == ",")
					result = *tokStart;
				std::string ident = getIdentifier(tokStart, lineTokens.end());
				if (!ident.empty() && ident != type && ident != functionCall)
					needsEnd = ident;
				goto endblock;
			} else if (*tokStart == "Else") {
				if (outElse == nullptr) {
					std::cout << "Cannot have else in this block." << std::endl;
					exit(1);
				} else {
					block(type, functionCall, outElse);
					goto endblock;
				}
			} else {
				if (*tokStart == "Set" && header == nullptr) {
					std::string ident;
					if ((ident = getIdentifier(++tokStart, lineTokens.end())) == "result") {
						if (returners > 0);
						else if (returnType != nullptr && returnType->empty()) {
							*returnType = "auto ";
							outBlock->append("auto ");
							returners++;
						} else if (returnType == nullptr || *returnType == "void ") {
							std::cout << "Reserved variable name 'result' cannot be written to in the current context." << std::endl;
							exit(1);
						}
					}
					for (const auto &arg : runtimeArgs)
						if (ident == arg) {
							std::cout << "Cannot modify runtime argument" << std::endl;
							exit(1);
						}
					tokStart--;
				}
				std::string parsed;
				if (header == nullptr)
					parseTokens(lineTokens, parsed, nullptr, prefix);
				else
					header->append(parseTokens(lineTokens, parsed, nullptr, prefix));
				outBlock->append(parsed);
				if (!needsEnd.empty()) {
					if (needsEnd == type || needsEnd == functionCall)
						needsEnd.clear();
					goto endblock;
				}
			}
		}
endblock:
		if (returnType != nullptr && !returnType->empty() && *returnType != "void ") {
			outBlock->append("return result;");
			returners--;
		}
		return result;
	}

	std::string
	method(std::vector<std::string>::iterator &iter, std::vector<std::string>::iterator &end, bool init = false) {
		std::string functionCall;
		if (init)
			functionCall = "_make" + getTypeName(iter, end);
		else
			functionCall = getIdentifier(iter, end);
		if (functionCall == "Ignore")
			return "";
		bool runtime = false;
		for (const auto &mth : runtimeMethods)
			if (mth == functionCall) {
				runtime = true;
				break;
			}

		std::vector<std::string> args;
		if (iter == end)
			return functionCall + "()";
		std::vector<std::string> types;
		std::list<std::string> blockArgs;
		for (; iter != end; iter++) {
			if (*iter == "(") {
				auto startIter = iter;
				std::string type = getIdentifier(startIter, end);
				if (*(iter - 1) == "," || *iter == ")" || *iter == ":")
					types.push_back(getTypeName(iter, end));
				else if (*startIter == "(") {
					iter = startIter;
					std::string arg = type;
					iter++;
					type = getTypeName(iter, end);
					types.push_back(type);
					blockArgs.push_back(type + arg);
					if (*(iter + 1) == ",")
						iter++;
				}
			} else if (openParentheses < 0) {
				std::cout << "Syntax Error: Extra close parentheses" << std::endl;
				exit(1);
			} else if (*iter == ":") {
				iter++;
				break;
			} else if (*iter == ";")
				break;
		}
		std::string elseBlock;
		std::string arg;
		auto startArg = iter;
		for (; iter != end; iter++) {
			if (*iter == "(")
				openParentheses++;
			else if (*iter == ")")
				openParentheses--;
			else if (*iter == ";") {
				if (!arg.empty())
					args.push_back(arg);
				if (startArg != iter)
					args.push_back(expression(startArg, iter));
				arg = "[&](";
				if (!blockArgs.empty()) {
					for (const std::string &blockArg : blockArgs)
						arg += blockArg + ",";
					arg.pop_back();
				}
				arg += "){";
				std::string returnType = !types.empty() ? types[0] : "";
				std::string next = block("Block", functionCall, &arg, &returnType, &elseBlock);
				arg += "}";
				if (functionCall != "Block") {
					arg.insert(0, "_make_ptr<Block<" + returnType + ">>(").append(")");
				} else {
//					if (types.empty())
//						types.insert(types.begin(), returnType);
//					else
//						types[0] = returnType;
				}
				startArg = lineTokens.end();
				if (next.empty())
					continue;
				fr->nextLine();
				lineTokens.insert(lineTokens.begin(), next);
				lineTokens.insert(lineTokens.begin(), "");
				iter = (startArg = lineTokens.begin());
				end = lineTokens.end();
			} else if (openParentheses == 0 && *iter == ",") {
				if (arg.empty())
					args.push_back(expression(startArg, iter));
				else
					args.push_back(arg);
				arg.clear();
				startArg = iter + 1;
			}
		}

		if (!arg.empty())
			args.push_back(arg);
		else if (startArg != end)
			args.push_back(expression(startArg, iter));

		if (!types.empty()) {
			functionCall += "<";
			for (const auto &type : types)
				functionCall += type + ",";
			functionCall.pop_back();

			functionCall += ">";
		}
		functionCall += "(";
		int numargs = 0;
		for (const auto &a : args)
			if (!a.empty()) {
				if (runtime)
					if (a.length() > 2 && !strcmp(a.substr(a.length() - 2, 2).c_str(), "()"))
						functionCall += a.substr(0, a.length() - 2) + ",";
					else
						functionCall += "[&](){" + a + "},";
				else
					functionCall += a + ",";
				numargs++;
			}
		functionCall.pop_back();

		if (!elseBlock.empty())
			return "if (!" + functionCall + ")->BooleanValue()->_value){" + elseBlock + "}";
		return functionCall + ")";
	}

	std::string getIdentifier(std::vector<std::string>::iterator &iter, std::vector<std::string>::iterator end) {
		std::string result;
		std::string spaceBuffer;
		skipWhitespace(iter, end);
		auto start = *iter;
		for (; iter != end; iter++) {
			if (isalnum((*iter)[0])) {
				result += spaceBuffer;
				result += *iter;
				spaceBuffer = "";
			} else if (*iter == " ")
				spaceBuffer += "_";
			else
				break;
		}
		return result;
	}

	std::string
	defineClass(std::vector<std::string>::iterator iter, std::vector<std::string>::iterator end, std::string &header) {
		std::string className = getIdentifier(++iter, end);
		std::string super = "Object";

		for (; iter != end && *iter != ";"; iter++) {
			if (*iter == "extends") {
				super = getIdentifier(++iter, end);
				iter--;
			} else if (*iter != "," && !isWhitespace(iter)) {
				std::cout << "Error: Unexpected token '" + *iter + "'." << std::endl;
				exit(1);
			}
		}

		if (!skipWhitespace(iter, end))
			std::cout << "Missing token ';'" << std::endl;

		std::string blk;
		if (!block("Class", className, &blk, nullptr, nullptr, &header).empty()) {
			std::cout << "Error: Extra tokens after Definition" << std::endl;
			exit(1);
		}

		header = "class " + className + ":public " + super + "{" + header + "};";

		return blk;
	}

	std::string
	defineMethod(std::vector<std::string>::iterator iter, std::vector<std::string>::iterator end, std::string &header,
	             const std::string &prefix = "") {
		std::string functionName = getIdentifier(++iter, end);

		std::vector<std::string> args;
		std::map<std::string, std::string> types;

		std::string currentArg;
		std::string currentType;

		if (iter == end || *iter == ";");
		else if (*iter == ":" || *iter == ",")
			iter++;
		else {
			std::cout << "Missing token ':'" << std::endl;
			exit(1);
		}

		for (; iter != end; iter++) {
			if (*iter == "returns" || *iter == ";" || *iter == "runtime" || *iter == "public" || *iter == "private" ||
			    *iter == "extends")
				break;
			else if (*iter == ",") {
				if (currentType.empty()) {
					std::cout << "Syntax Error: Type must be defined" << std::endl;
					exit(1);
				}
				if (openParentheses > 0) {
					std::cout << "Syntax Error: '(' token not closed." << std::endl;
					exit(1);
				}
				if (openParentheses < 0) {
					std::cout << "Syntax Error: Invalid ')' token" << std::endl;
					exit(1);
				}
				args.push_back(currentArg);
				types[currentArg] = currentType;
				currentArg = "";
				currentType = "";
			} else if (*iter == "(") {
				iter++;
				currentType += getTypeName(iter, end);
			} else if (*iter != ")" && !isWhitespace(iter)) {
				currentArg = getIdentifier(iter, end);
				iter--;
			}
		}

		if (!currentArg.empty()) {
			if (currentType.empty()) {
				std::cout << "Syntax Error: Type must be defined" << std::endl;
				exit(1);
			}
			if (openParentheses > 0) {
				std::cout << "Syntax Error: '(' token not closed." << std::endl;
				exit(1);
			}
			if (openParentheses < 0) {
				std::cout << "Syntax Error: Invalid ')' token" << std::endl;
				exit(1);
			}
			args.push_back(currentArg);
			types[currentArg] = currentType;
		}

		std::string returnType = functionName == "Init" || functionName == "Deinit" ? "" : "void ";
		bool semicolon;
		bool runtime;
		std::string visibility = "public";
		std::string super;
		for (; iter != end; iter++) {
			if (*iter == "returns") {
				if (returnType.empty()) {
					std::cout << "Cannot return a value in an initializer or deinitializer" << std::endl;
					exit(1);
				}
				if (returnType != "void ") {
					std::cout << "Syntax Error: 2 'returns' keywords" << std::endl;
					exit(1);
				}
				returnType = getTypeName(++iter, end);
				iter--;
			} else if (*iter == "extends")
				if (functionName == "Init") {
					auto tempEnd = end - 1;
					super = ":" + method(++iter, tempEnd);
					if (*iter == ";")
						semicolon = true;
				} else {
					std::cout << "Error: Cannot use keyword 'extend' not on an initializer" << std::endl;
					exit(1);
				}
			else if (*iter == "runtime") {
				runtimeMethods.push_back(functionName);
				if (args.empty()) {
					std::cout << "Cannot have 'runtime' keyword with 0 arguments" << std::endl;
					exit(1);
				}
				runtime = true;
			} else if (*iter == "public" || *iter == "private" || *iter == "protected") {
				if (prefix.empty()) {
					std::cout << "Error: Method outside of class cannot have a visibility modifier." << std::endl;
					exit(1);
				}
				visibility = *iter;
			} else if (*iter == ";")
				semicolon = true;
			else if (!isWhitespace(iter)) {
				std::cout << "Unexpected token: '" + *iter + "'" << std::endl;
				exit(1);
			}
		}
		if (!semicolon) {
			std::cout << "Syntax Error: Missing ';'" << std::endl;
			exit(1);
		}

		std::string result = functionName;
		if (functionName == "Init")
			result = prefix;
		else if (functionName == "Deinit")
			result = "~" + prefix;
		result += "(";
		if (!args.empty()) {
			if (runtime)
				for (const std::string &arg : args)
					result += "std::function<" + types[arg] + "()>" + arg + ",";
			else
				for (const std::string &arg : args)
					result += types[arg] + arg + ",";
			result.pop_back();
		}
		header = (prefix.empty() ? "" : visibility + ":") + returnType + result +
		         (super.empty() ? ");" : ")" + super + "{");
		result = returnType + (prefix.empty() ? "" : prefix + "::") + result + "){";

		if (runtime)
			for (const std::string &arg : args)
				runtimeArgs.push_back(arg);

		auto startIter = runtimeArgs.end() - args.size();

		if (!block("Method", functionName, super.empty() ? &result : &header, &returnType).empty()) {
			std::cout << "Error: Extra tokens after Definition" << std::endl;
			exit(1);
		}

		if (runtime)
			runtimeArgs.erase(startIter, runtimeArgs.end());

		if (!super.empty()) {
			header += "}";
			return "";
		}
		return result + "}";
	}

	std::string
	opPos(const Operator &op, std::vector<std::string>::iterator start, std::vector<std::string>::iterator end,
	      const std::vector<std::array<std::vector<std::string>::iterator, 2>> &groups,
	      std::vector<std::string>::iterator *trueStart = nullptr) {
		auto &refTrue = trueStart == nullptr ? start : *trueStart;
		if (start == end) return "";
		std::vector<std::string>::iterator first;
		if (op.getSingleSided())
			first = op.secondExpr(start, end);
		else
			first = op.firstExpr(start, end);
		bool within = false;
		if (first == end) return "";
		for (const auto &group : groups) {
			for (auto i = group[0]; i != group[1]; i++)
				if (first == i) {
					within = true;
					break;
				}
		}
		if (within) return opPos(op, first + 1, end, groups, trueStart == nullptr ? &start : trueStart);
		if (op.getSingleSided())
			return "(" + expression(first, end) + ")->" + op.getFunctionName() + "()";
		else {
			auto second = op.secondExpr(start, end);
			if (&op == &possessiveOperator)
				return expression(refTrue, first) + "->" + method(second, end);
			else
				return "(" + expression(refTrue, first) + ")->" + op.getFunctionName() + "(" + expression(second, end) +
				       ")";
		}
	}

	std::string
	setVariable(std::vector<std::string>::iterator iter, std::vector<std::string>::iterator end, bool init = false) {
		if (!skipWhitespace(iter, end)) {
			std::cout << "Syntax Error: Incomplete statement." << std::endl;
			exit(1);
		}
		std::string varName = getIdentifier(iter, end);

		std::string varType = "auto ";
		std::string setTo;
		for (; iter != end; iter++) {
			if (init && *iter == "(") {
				iter++;
				varType.clear();
				varType.append(getTypeName(iter, end));
				if (iter != end && *iter != ")") {
					std::cout << "Syntax Error: Unexpected token '" << *iter << "'." << std::endl;
					exit(1);
				}
			} else if (*iter == ":") {
				if (*(iter + 1) == ":") *iter = varName;
				else iter++;
				setTo = expression(iter, end);
				break;
			} else {
				auto i = iter - 1;
				*i = varName;
				for (const auto &op : operators) {
					std::string result = opPos(op, i, end,
					                           std::vector<std::array<std::vector<std::string>::iterator, 2>>());
					if (!result.empty()) {
						setTo = result;
						break;
					}
				}
				if (setTo.empty()) {
					std::cout << "Error: Unexpected token: '" << *iter << std::endl;
					exit(1);
				}
				break;
			}
		}
		if (init && varType == "auto " && setTo.empty()) {
			std::cout << "Syntax Error: Incomplete statement." << std::endl;
			exit(1);
		}
		std::stringstream result;
		if (init)
			result << varType << varName << (setTo.empty() ? "" : "=") << setTo << ";";
		else
			result << varName << "=" << setTo << ";";
		return result.str();
	}

	std::string expression(std::vector<std::string>::iterator &start, std::vector<std::string>::iterator end) {
		std::vector<std::array<std::vector<std::string>::iterator, 2>> groups;
		{
			int openParen = 0;
			std::vector<std::string>::iterator parenStart;
			for (auto i = start; i != end; i++) {
				if (*i == "(") {
					if (++openParen == 1) parenStart = i;
				} else if (*i == ")") {
					if (--openParen == 0)
						groups.push_back({parenStart, i});
					else if (openParen < 0) {
						std::cout << "Unexpected token ')'" << std::endl;
						exit(1);
					}
				}
			}
			if (openParen > 0) {
				std::cout << "Unclosed parentheses" << std::endl;
				exit(1);
			}
		}

		if (groups.size() == 1) {
			if (groups[0][0] == start) {
				if (groups[0][1] + 1 == end) {
					auto newStart = groups[0][0] + 1;
					return expression(newStart, groups[0][1]);
				}
			}
		}

		bool init = false;
		if (*start == "new") {
			init = true;
			start++;
		}
		if (init || start->front() >= 'A' && start->front() <= 'Z') {
			bool hasColon;
			auto i = start;
			if (init)
				getTypeName(i, end);
			else
				getIdentifier(i, end);
			if (i == end || *i == ":")
				return method(start, end, init);
		}

		for (const auto &op : operators) {
			std::string result = opPos(op, start, end, groups);
			if (!result.empty())
				return result;
		}
		std::string result = opPos(possessiveOperator, start, end, groups);
		if (!result.empty())
			return result;

		if (start->front() == '"' && start->back() == '"')
			if (start + 1 == end) return "_make_ptr<Text>(" + *start + ")";
			else {
				std::cout << "Error: Unexpected token after string definition" << std::endl;
				exit(1);
			}
		else if (*start == "true" || *start == "false") return "_make_ptr<Boolean>(" + *start + ")";
		bool isDouble;
		for (char ch : *start) {
			if (ch == '.' && !isDouble)
				isDouble = true;
			else if (ch < '0' || ch > '9') {
				std::string ident = getIdentifier(start, end);
				for (const auto &arg : runtimeArgs)
					if (ident == arg)
						return ident + "()";
				return ident;
			}
		}
		return "_make_ptr<Number>(" + *start + ")";
	}

	/**
 * Parses the tokens in <b>tokens</b>, and sets <b>result</b> to the parsed value.
 * @param tokens The tokens of the line being inputted
 * @param result A reference to the result string
 * @return What to add to the header, if anything.
 */
	std::string parseTokens(std::vector<std::string> tokens, std::string &result, std::string *implementation,
	                        const std::string &prefix) {
		auto beginningToken = tokens.begin();
		std::string header;
		if (!skipWhitespace(beginningToken, tokens.end()))
			return header;
		else if (*beginningToken == "Method") {
			if (implementation != nullptr)
				*implementation = defineMethod(beginningToken, tokens.end(), header, prefix);
			else
				result = defineMethod(beginningToken, tokens.end(), header, prefix);
		} else if (*beginningToken == "Class") {
			if (implementation != nullptr)
				*implementation = defineClass(beginningToken, tokens.end(), header);
			else
				result = defineClass(beginningToken, tokens.end(), header);
		} else if (*beginningToken == "Define") {
			if (implementation != nullptr)
				*implementation = setVariable(beginningToken + 1, tokens.end(), true);
			else
				header = setVariable(beginningToken + 1, tokens.end(), true);
		} else if (*beginningToken == "Init" || *beginningToken == "Deinit") {
			if (implementation != nullptr)
				*implementation = defineMethod(--beginningToken, tokens.end(), header, prefix);
			else
				result = defineMethod(--beginningToken, tokens.end(), header, prefix);
		} else if (*beginningToken == "Set")
			result = setVariable(beginningToken + 1, tokens.end());
		else {
			std::string expr = expression(beginningToken, tokens.end());
			result = expr.empty() ? "" : expr + ";";
		}

		return header;
	}
}