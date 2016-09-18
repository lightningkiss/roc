#ifndef _LIVE_RTSP_SERVER_H__
#define _LIVE_RTSP_SERVER_H__

#include <string>
#include <map>
#include <vector>
#include "RTSPServer.hh"
#include "Groupsock.hh"

class LiveRTSPServer : public RTSPServer {
public:
	static LiveRTSPServer* createNew(UsageEnvironment& env,
		UserAuthenticationDatabase* authDatabase = NULL, unsigned reclamationTestSeconds = 65);

	static void IncomingEventHandler(void* clientData, int mask);
	virtual void OnGetStreamOK(const char* url);
	virtual void OnGetStreamError(const char* url);

protected:
	LiveRTSPServer(UsageEnvironment& env, int ourSocket,
		UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds);
	// called only by createNew();
	virtual ~LiveRTSPServer();

protected: // redefined virtual functions
	virtual ClientConnection* createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr);

public: // should be protected, but some old compilers complain otherwise
	class LiveRTSPClientConnection : public RTSPServer::RTSPClientConnection {
	public:
		LiveRTSPClientConnection(LiveRTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr);
		virtual ~LiveRTSPClientConnection();

		void HandleSessionNotFound(char const* streamName);
		void SendDescribeRespone(ServerMediaSession* session);
		void HandleUrlError();
		void HandleStreamError();
		void HandleStreamPendding();
		bool SetRtpRule(const char* streamName);
		const std::string& ClientIpPort();

	protected:
		virtual void handleCmd_DESCRIBE(char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr);

	private:
		char fDescCSeq[256];
		std::string m_clientIpPort;
		std::string m_requestUrl;
		ServerMediaSession* fOurServerMediaSession;
		LiveRTSPServer* m_server;
	};

	virtual ServerMediaSession* lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession);
	void DeleteClientConnection(LiveRTSPClientConnection* client, const std::string& url);
	void AddClientConnection(LiveRTSPClientConnection* client, const std::string& url);

private:
	typedef struct {
		std::string url;
		std::string streamName;
		//STREAM_STATUS streamStatus;
		ServerMediaSession* session;
		std::vector<LiveRTSPClientConnection*> clients;
	}RTSPSTREAM, *PRTSPSTREAM;

	typedef std::map<std::string, PRTSPSTREAM> RTSPSTREAMS;

	LiveRTSPClientConnection* fClientConnection;
	RTSPSTREAMS m_streams;
};

#endif //_LIVE_RTSP_SERVER_H__
