#ifndef _LIVE_SERVER_SESSION_H__
#define _LIVE_SERVER_SESSION_H__

#include "liveMedia.hh"

class LiveRTSPClient : public ProxyRTSPClient {
public:
	LiveRTSPClient(class ProxyServerMediaSession& ourServerMediaSession, char const* rtspURL,
		char const* username, char const* password,
		portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer);
	virtual ~LiveRTSPClient();

	void continueAfterDESCRIBE(char const* sdpDescription);
	void continueAfterLivenessCommand(int resultCode, Boolean serverSupportsGetParameter);
	void continueAfterSETUP(int resultCode);
	void continueAfterPLAY(int resultCode);
};

ProxyRTSPClient*
CreateNewLiveRTSPClientFunc(ProxyServerMediaSession& ourServerMediaSession,
	char const* rtspURL,
	char const* username, char const* password,
	portNumBits tunnelOverHTTPPortNum, int verbosityLevel,
	int socketNumToServer);

class LiveServerMediaSession : public ProxyServerMediaSession {
public:
	static LiveServerMediaSession* createNew(UsageEnvironment& env,
		GenericMediaServer* ourMediaServer, // Note: We can be used by just one server
		char const* inputStreamURL, // the "rtsp://" URL of the stream we'll be proxying
		char const* streamName = NULL,
		char const* username = NULL, char const* password = NULL,
		portNumBits tunnelOverHTTPPortNum = 0,
		// for streaming the *proxied* (i.e., back-end) stream
		int verbosityLevel = 0,
		int socketNumToServer = -1,
		MediaTranscodingTable* transcodingTable = NULL);
	// Hack: "tunnelOverHTTPPortNum" == 0xFFFF (i.e., all-ones) means: Stream RTP/RTCP-over-TCP, but *not* using HTTP
	// "verbosityLevel" == 1 means display basic proxy setup info; "verbosityLevel" == 2 means display RTSP client protocol also.
	// If "socketNumToServer" is >= 0, then it is the socket number of an already-existing TCP connection to the server.
	//      (In this case, "inputStreamURL" must point to the socket's endpoint, so that it can be accessed via the socket.)

	virtual ~LiveServerMediaSession();

protected:
	LiveServerMediaSession(UsageEnvironment& env, GenericMediaServer* ourMediaServer,
		char const* inputStreamURL, char const* streamName,
		char const* username, char const* password,
		portNumBits tunnelOverHTTPPortNum, int verbosityLevel,
		int socketNumToServer,
		MediaTranscodingTable* transcodingTable,
		portNumBits initialPortNum = 6970,
		Boolean multiplexRTCPWithRTP = False);
};

#endif //_LIVE_SERVER_SESSION_H__