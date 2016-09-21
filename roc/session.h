#ifndef _CLIENT_H__
#define _CLIENT_H__

#include "ProxyServerMediaSession.hh"
#include "server.h"

typedef enum {
	RTSP_CONNECTING,
	RTSP_CONNECTOK,
	RTSP_DISCONNECT
} RTSP_CLIENT_STATUS;

class LiveServerMediaSession : public ProxyServerMediaSession {
public:
	static LiveServerMediaSession* createNew(UsageEnvironment& env,
		LiveRTSPServer* ourRTSPServer, // Note: We can be used by just one "RTSPServer"
		char const* inputStreamURL, // the "rtsp://" URL of the stream we'll be proxying
		char const* streamName = NULL,
		char const* username = NULL, char const* password = NULL,
		portNumBits tunnelOverHTTPPortNum = 0,
		// for streaming the *proxied* (i.e., back-end) stream
		int verbosityLevel = 0,
		int socketNumToServer = -1);

	virtual ~LiveServerMediaSession();
	virtual void continueAfterDESCRIBE(char const* sdpDescription);
	RTSP_CLIENT_STATUS Status() { return m_status; }

	void DeleteClientConnection(LiveRTSPServer::LiveRTSPClientConnection* client);
	void AddClientConnection(LiveRTSPServer::LiveRTSPClientConnection* client);

protected:
	LiveServerMediaSession(UsageEnvironment& env, LiveRTSPServer* ourRTSPServer,
		char const* inputStreamURL, char const* streamName,
		char const* username, char const* password,
		portNumBits tunnelOverHTTPPortNum, int verbosityLevel,
		int socketNumToServer);

private:
	LiveRTSPServer* fProxyRTSPServer;
	RTSP_CLIENT_STATUS m_status;
	std::vector<LiveRTSPServer::LiveRTSPClientConnection*> m_clients;
};

#endif //_CLIENT_H__
