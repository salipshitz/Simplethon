//
// Created by geger on 5/12/2020.
//

#include "filereader.h"

namespace compiler {
	void FileReader::nextLine() {
		std::string line;
		if (lineBuffer.empty()) {
			getline(stream, line);
			lineTokens = splitTokens(line.c_str());
		} else {
			lineTokens = lineBuffer;
			lineBuffer = std::vector<std::string>();
		}
		bool lineComplete = false;
		while (!lineComplete) {
			int parenOpen = 0;
			for (auto i = lineTokens.begin(); i != lineTokens.end(); i++) {
				if (*i == "(")
					parenOpen++;
				else if (*i == ")")
					parenOpen--;
				else if (*i == "." || *i == ";") {
					lineComplete = true;

					for (auto j = i + 1; j != lineTokens.end(); j++)
						lineBuffer.push_back(*j);
					lineTokens.erase(*i == ";" ? i + 1 : i, lineTokens.end());
					break;
				}
			}
			if (!parenOpen || lineComplete)
				lineComplete = true;
			else {
				getline(stream, line);
				for (const std::string &tok : splitTokens(line.c_str()))
					lineTokens.push_back(tok);
			}
		}

	}

	FileReader::FileReader(const char *file) {
		stream.open(CURRENT_DIR + "/" + file);
	}

	bool FileReader::hasNextLine() {
		return !stream.eof() || !lineBuffer.empty();
	}

	bool FileReader::isOpen() {
		return stream.is_open();
	}
}
