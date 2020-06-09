//
// Created by geger on 5/11/2020.
//

#ifndef SIMPLETHON_COMPILER_H
#define SIMPLETHON_COMPILER_H

#include <string>
#include <regex>
#include <iostream>
#include <list>
#include "filereader.h"

namespace compiler {
	class FileReader;

	extern FileReader *fr;
	extern std::vector<std::string> lineTokens;
	extern std::vector<std::string> runtimeMethods;

	std::string getTypeName(std::vector<std::string>::iterator &iter, std::vector<std::string>::iterator end);

	std::vector<std::string> splitTokens(const char *input);

	std::string parseTokens(std::vector<std::string> tokens, std::string &result, std::string *implementation,
	                        const std::string &prefix = "");

	std::string getIdentifier(std::vector<std::string>::iterator &iter, std::vector<std::string>::iterator end);

	bool isWhitespace(std::vector<std::string>::iterator iter);

	bool skipWhitespace(std::vector<std::string>::iterator &iter, std::vector<std::string>::iterator end);
}

#endif //SIMPLETHON_COMPILER_H