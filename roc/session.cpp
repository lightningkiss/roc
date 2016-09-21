#include "session.h"

LiveServerMediaSession* LiveServerMediaSession::createNew(UsageEnvironment& env, LiveRTSPServer* ourRTSPServer,
		char const* inputStreamURL, char const* streamName, char const* username, char const* password,
		portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer) 
{
	return new LiveServerMediaSession(env, ourRTSPServer, inputStreamURL, streamName, username, password,
		tunnelOverHTTPPortNum, verbosityLevel, socketNumToServer);
}

LiveServerMediaSession::LiveServerMediaSession(UsageEnvironment& env, LiveRTSPServer* ourRTSPServer, char const* inputStreamURL,
		char const* streamName, char const* username, char const* password, portNumBits tunnelOverHTTPPortNum, 
		int verbosityLevel, int socketNumToServer)
		: ProxyServerMediaSession(env, ourRTSPServer, inputStreamURL, streamName, username, password, tunnelOverHTTPPortNum,
		verbosityLevel, socketNumToServer, NULL), fProxyRTSPServer(ourRTSPServer)
{

}

LiveServerMediaSession::~LiveServerMediaSession()
{

}

void LiveServerMediaSession::continueAfterDESCRIBE(char const* sdpDescription)
{
	m_status = (sdpDescription == NULL) ? RTSP_DISCONNECT : RTSP_CONNECTOK;
	ProxyServerMediaSession::continueAfterDESCRIBE(sdpDescription);

	for (size_t i = 0; i < m_clients.size(); i++) {
		LiveRTSPServer::LiveRTSPClientConnection* client = m_clients[i];
		client->SendDescribeRespone(this);
	}
}

void LiveServerMediaSession::DeleteClientConnection(LiveRTSPServer::LiveRTSPClientConnection* client) {
	std::vector<LiveRTSPServer::LiveRTSPClientConnection*>::iterator it = m_clients.begin();
	for (; it != m_clients.end(); it++) {
		if (*it == client) {
			m_clients.erase(it);
			break;
		}
	}
}

void LiveServerMediaSession::AddClientConnection(LiveRTSPServer::LiveRTSPClientConnection* client) {
	std::vector<LiveRTSPServer::LiveRTSPClientConnection*>::iterator it = m_clients.begin();
	for (; it != m_clients.end(); it++) {
		if (*it == client) {
			return;
		}
	}

	m_clients.push_back(client);
}
