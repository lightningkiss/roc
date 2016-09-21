#ifndef _SERVER_H__
#define _SERVER_H__

#include <string>
#include <map>
#include <vector>
#include "RTSPServer.hh"
#include "Groupsock.hh"

class LiveRTSPServer : public RTSPServer {
public:
	static LiveRTSPServer* createNew(UsageEnvironment& env, UserAuthenticationDatabase* authDatabase = NULL, unsigned reclamationTestSeconds = 65);

protected:
	LiveRTSPServer(UsageEnvironment& env, int ourSocket, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds);
	// called only by createNew();
	virtual ~LiveRTSPServer();

protected: // redefined virtual functions
	virtual ClientConnection* createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr);

public: // should be protected, but some old compilers complain otherwise
	class LiveRTSPClientConnection : public RTSPServer::RTSPClientConnection {
	public:
		LiveRTSPClientConnection(LiveRTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr);
		virtual ~LiveRTSPClientConnection();

		void SendDescribeRespone(ServerMediaSession* session);
		const std::string& ClientIpPort();

	protected:
		virtual void handleCmd_DESCRIBE(char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr);

	private:
		char m_descCSeq[256];
		std::string m_clientIpPort;
		std::string m_requestUrl;
		ServerMediaSession* m_liveSession;
		LiveRTSPServer* m_liveServer;
	};

	ServerMediaSession* lookupServerMediaSession(char const* streamName);
};

#endif //_SERVER_H__
