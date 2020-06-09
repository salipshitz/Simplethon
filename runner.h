//
// Created by geger on 5/19/2020.
//

#ifndef SIMPLETHON_RUNNER_H
#define SIMPLETHON_RUNNER_H

#include "filereader.h"

namespace compiler {
	extern std::string SMPL_PATH;

	extern std::vector<std::string> files;

	bool compileFromFile(const char *fileName);

	void cleanupFiles();
}

#endif //SIMPLETHON_RUNNER_H
