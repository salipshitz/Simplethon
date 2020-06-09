//
// Created by geger on 5/19/2020.
//

#include <string>
#include <sys/stat.h>
#include <ctime>
#include <filesystem>
#include "runner.h"

#if defined(WIN32) || defined(_WIN32)
#define stat _stat
#else
#include<unistd.h>
#endif

namespace compiler {
	std::vector<std::string> files;

	bool importFile(const std::string &identifier) {
		struct stat ih;
		struct stat ic;

		if (!stat((SMPL_PATH + "/import/" + identifier + ".h").c_str(), &ih) &&
		    !stat((SMPL_PATH + "/import/" + identifier + ".cpp").c_str(), &ic)) {
			files.push_back(identifier + ".h");
			files.push_back(identifier + ".cpp");

			struct stat th;
			struct stat tc;

			if (!stat((SMPL_PATH + "/temp/" + identifier + ".h").c_str(), &th) &&
			    !stat((SMPL_PATH + "/temp/" + identifier + ".cpp").c_str(), &tc)) {
				time_t iht = ih.st_mtime;
				time_t ict = ic.st_mtime;
				time_t tht = th.st_mtime;
				time_t tct = tc.st_mtime;

				if (iht == ict && tht == tct)
					return true;
			}

			std::ifstream headerIn(SMPL_PATH + "/import/" + identifier + ".h", std::ios::binary);
			std::ifstream sourceIn(SMPL_PATH + "/import/" + identifier + ".cpp", std::ios::binary);

			std::ofstream headerOut(SMPL_PATH + "/temp/" + identifier + ".h", std::ios::binary);
			headerOut << headerIn.rdbuf();
			headerOut.close();

			std::ofstream sourceOut(SMPL_PATH + "/temp/" + identifier + ".cpp", std::ios::binary);
			sourceOut << sourceIn.rdbuf();
			sourceOut.close();

			struct utimbuf ht;
			ht.actime = time(nullptr);
			ht.modtime = ih.st_mtime;
			utime((SMPL_PATH + "/temp/" + identifier + ".h").c_str(), &ht);

			struct utimbuf ct;
			ct.actime = time(nullptr);
			ct.modtime = ic.st_mtime;
			utime((SMPL_PATH + "/temp/" + identifier + ".cpp").c_str(), &ct);

			return true;
		} else {
			FileReader *f = fr;
			bool compiled = compileFromFile(identifier.c_str());
			fr = f;
			return compiled;
		}
	}

	std::string fileIdent(const std::string &fileName) {
		std::string fileIdent = fileName;
		int slash = fileIdent.find_last_of('/');
		if (slash == std::string::npos)
			slash = 0;
		int dot = fileIdent.find_last_of('.');
		if (dot == std::string::npos)
			dot = 0;
		fileIdent = fileIdent.substr(slash, dot);
		return fileIdent;
	}

	bool compileFromFile(const char *fileName) {
		struct stat initialFile;
		struct stat tempFile;
		if (!stat(fileName, &initialFile) &&
		    !stat((SMPL_PATH + "/temp/" + fileIdent(fileName) + ".cpp").c_str(), &tempFile)) {
			time_t initialModTime = initialFile.st_mtime;
			time_t tempModTime = tempFile.st_mtime;
			if (initialModTime == tempModTime) {
				return true;
			}
		}

		fr = new FileReader(fileName);

		if (!fr->isOpen()) {
			std::cout << std::string("File '") + fileName + "' not found\n";
			std::cout << CURRENT_DIR;
			exit(1);
		}

		std::string output;
		std::string mainOutput;
		std::string headerOutput;

		std::string ident = "built_in";
		if (!importFile(ident))
			return false;
		std::ostringstream include;
		include << "\n#include\"" << ident << ".h\"\nusing namespace " << ident << ";";
		headerOutput += include.str();

		while (fr->hasNextLine()) {
			fr->nextLine();
			auto iter = lineTokens.begin();
			if (skipWhitespace(iter, lineTokens.end()) && *iter++ == "Import") {
				if (*iter++ != ":") {
					std::cout << "Syntax Error: Missing token ':'";
					exit(1);
				}
				std::string importIdent = getIdentifier(iter, lineTokens.end());
				if (!importFile(importIdent))
					return false;
				std::ostringstream includeText;
				includeText << "\n#include\"" << importIdent << ".h\"\nusing namespace " << importIdent << ";";
				headerOutput += includeText.str();
			} else {
				std::string result, implementation;
				std::string header = parseTokens(lineTokens, result, &implementation);
				mainOutput += result;
				output += implementation;
				headerOutput += header;
			}
		}

		std::string fident = fileIdent(fileName);

		files.emplace_back(fident + ".cpp");
		files.emplace_back(fident + ".h");

		std::ofstream hstrm(SMPL_PATH + "/temp/" + fident + ".h");
		std::ofstream strm(SMPL_PATH + "/temp/" + fident + ".cpp");

		hstrm << headerOutput;

		strm << "#include \"" + fident + ".h\"\n" << output << "\nint main(){" << mainOutput << "}";
		strm.close();

		struct utimbuf ot;
		ot.actime = time(nullptr);
		ot.modtime = initialFile.st_mtime;
		utime((SMPL_PATH + "/temp/" + ident + ".h").c_str(), &ot);

		delete fr;

		return true;
	}

	void cleanupFiles() {
		for (const auto &entry : std::filesystem::directory_iterator(SMPL_PATH + "/temp/")) {
			if (std::find(files.begin(), files.end(), entry->))
		}
			DIR *dir;
		struct dirent *ent;
		if ((dir = opendir((SMPL_PATH + "/temp/").c_str())) != nullptr)
			while ((ent = readdir(dir)) != nullptr)
				if (std::find(files.begin(), files.end(), ent->d_name) == files.end())
					remove((SMPL_PATH + "/temp/" + ent->d_name).c_str());
	}
}
