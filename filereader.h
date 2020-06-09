//
// Created by geger on 5/12/2020.
//

#ifndef SIMPLETHON_FILEREADER_H
#define SIMPLETHON_FILEREADER_H

#include "compiler.h"
#include <fstream>

namespace compiler {
	extern const std::string CURRENT_DIR;

	class FileReader {
		std::ifstream stream;
		std::vector<std::string> lineBuffer;
	public:
		explicit FileReader(const char *file);

		bool hasNextLine();

		void nextLine();

		bool isOpen();
	};
}

#endif //SIMPLETHON_FILEREADER_H
