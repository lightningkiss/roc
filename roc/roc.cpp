#ifndef WIN32
#include <sys/wait.h>
#endif
#include <time.h>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "server.h"

static void run()
{
	OutPacketBuffer::maxSize = 1024 * 1024; // bytes

	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	RTSPServer* rtspServer = NULL;

	while (1) {
		rtspServer = LiveRTSPServer::createNew(*env);
		if (rtspServer != NULL) {
			break;
		}

#ifdef WIN32
		Sleep(60 * 1000);
#else
		sleep(5);
#endif
	}

	env->taskScheduler().doEventLoop();
}

int main(int argc, char** argv)
{
	run();
	return 0;
}
