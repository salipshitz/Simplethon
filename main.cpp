#include <cstdlib>
#include "runner.h"
#include "operator.h"

#if defined(WIN32) || defined(_WIN32)

#include <direct.h>

#define GetCurrentDir _getcwd
#else

#include <unistd.h>

#define GetCurrentDir getcwd
#endif

namespace compiler {
	const char *version = "Simplethon A-0.2";

	std::string SMPL_PATH;

	std::string get_current_dir() {
		char buff[FILENAME_MAX]; //create string buffer to hold path
		GetCurrentDir(buff, FILENAME_MAX);
		std::string current_working_dir(buff);
		return current_working_dir;
	}

	const std::string CURRENT_DIR = get_current_dir();

	int main(int argc, char **argv) {
		if (getenv("SMPL_PATH"))
			SMPL_PATH = getenv("SMPL_PATH");
		else {
			std::cout << "Fatal: Environment variable 'SMPL_PATH' not set.";
			exit(1);
		}

		if (argc == 2) {
			if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
				std::cout << "Usage:\n" << "   smpl <file>\n" << "   smpl [run] [executable] [compile] <file>\n"
				          << "smpl cleanup\n" << "   smpl -h | --help\n" << "   smpl -v | --version\n" << "\nOptions:\n"
				          << "   -h --help     Show this screen\n" << "   -v --version  Show version.\n"
				          << "   run           Default mode, creates a temporary compiled file and executable, and runs it.\n"
				          << "   executable    Creates an executable and places it in the working directory. If used with\n"
				          << "                 run, the temporary executable will be returned.\n"
				          << "   compile       Compiles the code into C++ and places it in the working directory. If used\n"
				          << "                 with run or executable, the temporary C++ file will be returned.\n"
				          << "   cleanup       Cleans up the temporary directory by removing unnecessary files.\n";
			else if (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-v"))
				std::cout << version << "\n";
			else if (!strcmp(argv[1], "cleanup"));
			else {
				setupOperators();
				compileFromFile(argv[1]);
				std::string cmd = "g++ -o " + SMPL_PATH + "/temp/exec";
				cleanupFiles();
				for (const auto &f : files)
					cmd.append(" ").append(SMPL_PATH).append("/temp/").append(f);

				std::system(cmd.c_str());
				std::system((SMPL_PATH + "/temp/exec").c_str());
			}
		} else if (argc > 2) {
			bool run;
			bool exec;
			bool comp;
			char *file = nullptr;
			for (int i = 1; i < argc; i++) {
				if (!strcmp(argv[i], "run"))
					run = true;
				else if (!strcmp(argv[i], "executable"))
					exec = true;
				else if (!strcmp(argv[i], "compiled"))
					comp = true;
				else if (file == nullptr)
					file = argv[i];
				else {
					std::cout << "Too many arguments to smpl\n" << "Use option -h for help";
					return true;
				}
			}
			if (file == nullptr) {
				std::cout << "Not enough arguments to smpl\n";
				return true;
			}
			setupOperators();
			compileFromFile(file);
			std::string cmd = "g++ -o " + SMPL_PATH + "/temp/exec";
			for (const auto &f : files)
				cmd.append(" ").append(SMPL_PATH).append("/temp/").append(f);
			std::system(cmd.c_str());
			if (comp)
				for (const auto &f : files) {
					std::ifstream inF(SMPL_PATH + "/temp/" + f);
					std::ofstream outF(CURRENT_DIR + "/" + f);
					outF << inF.rdbuf();
					outF.close();
					inF.close();
				}
			if (exec) {
#ifdef WIN32
				std::ifstream inF(SMPL_PATH + "/temp/exec.exe");
				std::ofstream outF(CURRENT_DIR + "/exec.exe");
#else
				std::ifstream inF(SMPL_PATH+"/temp/exec");
				std::ofstream outF(CURRENT_DIR + "/exec");
#endif
				outF << inF.rdbuf();
				inF.close();
				outF.close();
			}
			if (run)
				std::system((SMPL_PATH + "/temp/exec").c_str());
		} else {
			std::cout << "Not enough arguments to smpl\n" << "Use option -h for help";
			return true;
		}
		return false;
	}
}

#include "import/built_in.h"

using namespace built_in;

int main(int argc, char **argv) {
	return compiler::main(argc, argv);
}