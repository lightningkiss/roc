#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "server.h"

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = False;

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
	char const* streamName, char const* inputFileName) {
	char* url = rtspServer->rtspURL(sms);
	UsageEnvironment& env = rtspServer->envir();
	env << "\n\"" << streamName << "\" stream, from the file \""
		<< inputFileName << "\"\n";
	env << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;
}

int main(int argc, char** argv) {
	// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	char const* descriptionString = "Session streamed by roc server";

	UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
	// To implement client access control to the RTSP server, do the following:
	authDB = new UserAuthenticationDatabase;
	authDB->addUserRecord("username1", "password1"); // replace these with real strings
													 // Repeat the above with each <username>, <password> that you wish to allow
													 // access to the server.
#endif

	// Create the RTSP server:
	LiveRTSPServer* rtspServer = LiveRTSPServer::createNew(*env, authDB);
	if (rtspServer == NULL) {
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

// 	char const* streamName = "h264ESVideoTest";
// 	char const* inputFileName = "test.264";
// 	ServerMediaSession* sms
// 		= ServerMediaSession::createNew(*env, streamName, streamName,
// 			descriptionString);
// 	sms->addSubsession(H264VideoFileServerMediaSubsession
// 		::createNew(*env, inputFileName, reuseFirstSource));
// 	rtspServer->addServerMediaSession(sms);
// 
// 	announceStream(rtspServer, sms, streamName, inputFileName);

	// Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
	// Try first with the default HTTP port (80), and then with the alternative HTTP
	// port numbers (8000 and 8080).
	if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
		*env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
	} else {
		*env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
	}

	env->taskScheduler().doEventLoop(); // does not return

	return 0;
}
