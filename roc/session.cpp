#include "session.h"

////////// ProxyServerMediaSession implementation //////////

ProxyRTSPClient* CreateNewLiveRTSPClientFunc(ProxyServerMediaSession& ourServerMediaSession,
	char const* rtspURL, char const* username, char const* password,
	portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer) {
	LiveRTSPClient* rc = new LiveRTSPClient(ourServerMediaSession, rtspURL, username, password,
		tunnelOverHTTPPortNum, verbosityLevel, socketNumToServer);
	return (ProxyRTSPClient*)rc;
}

LiveServerMediaSession* LiveServerMediaSession
::createNew(UsageEnvironment& env, GenericMediaServer* ourMediaServer,
	char const* inputStreamURL, char const* streamName,
	char const* username, char const* password,
	portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer,
	MediaTranscodingTable* transcodingTable) {
	return new LiveServerMediaSession(env, ourMediaServer, inputStreamURL, streamName, username, password,
		tunnelOverHTTPPortNum, verbosityLevel, socketNumToServer,
		transcodingTable);
}

LiveServerMediaSession::LiveServerMediaSession(UsageEnvironment& env, GenericMediaServer* ourMediaServer,
	char const* inputStreamURL, char const* streamName,	char const* username, char const* password,
	portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer,
	MediaTranscodingTable* transcodingTable, portNumBits initialPortNum, Boolean multiplexRTCPWithRTP)
	: ProxyServerMediaSession(env, ourMediaServer, inputStreamURL, streamName, username, password, 
		tunnelOverHTTPPortNum, verbosityLevel, socketNumToServer, transcodingTable, CreateNewLiveRTSPClientFunc) {
}

LiveServerMediaSession::~LiveServerMediaSession() {

}

////////// "ProxyRTSPClient" implementation /////////

LiveRTSPClient::LiveRTSPClient(ProxyServerMediaSession& ourServerMediaSession, char const* rtspURL,
	char const* username, char const* password, portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer)
	: ProxyRTSPClient(ourServerMediaSession, rtspURL, username, password, tunnelOverHTTPPortNum, verbosityLevel, socketNumToServer) {

}

LiveRTSPClient::~LiveRTSPClient() {

}

void LiveRTSPClient::continueAfterDESCRIBE(char const* sdpDescription) {
	
}

void LiveRTSPClient::continueAfterLivenessCommand(int resultCode, Boolean serverSupportsGetParameter) {
	
}

void LiveRTSPClient::continueAfterSETUP(int resultCode) {

}

void LiveRTSPClient::continueAfterPLAY(int resultCode) {

}
