#include "log.h"

spdlog::logger* g_log = NULL;

#ifdef WIN32 
#include <windows.h>

std::string GetExePath()
{
	char buffer[MAX_PATH] = { 0 };

	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string file = buffer;
	std::string path = ".\\";
	size_t pos = file.rfind('\\');

	if (pos < file.size()) {
		path = file.substr(0, pos + 1);
	}

	return path;
}

void CreateDir(const std::string& path)
{
	CreateDirectory(path.c_str(), NULL);
}

#else
#include <sys/stat.h>
#include <sys/types.h> 
#include <unistd.h>

#define MAX_PATH 512
std::string GetExePath()
{
	char buffer[MAX_PATH] = { 0 };
	ssize_t ret = readlink("/proc/self/exe", buffer, MAX_PATH);
	if (ret == -1) {
		printf("not found [/proc/self/exe] return [./] \n");
		return "./";
	}

	std::string file = buffer;
	std::string path = "./";
	size_t pos = file.rfind('/');

	if (pos < file.size()) {
		path = file.substr(0, pos + 1);
	}

	return path;
}

void CreateDir(const std::string& path)
{
	mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

#endif


void LogInit() {
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
	sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_mt>("./log_regular_file.txt"));
	
	g_log = new spdlog::logger("multisink", sinks.begin(), sinks.end());
	g_log->set_level(spdlog::level::warn);

	sinks[0]->set_level(spdlog::level::trace);  // console. Allow everything.  Default value
	sinks[1]->set_level(spdlog::level::trace);  //  regular file. Allow everything.  Default value

	g_log->warn("warn: will print only on console and regular file");
}
