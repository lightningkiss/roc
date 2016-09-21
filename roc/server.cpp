#include "server.h"
#include "RTSPCommon.hh"
#include "RTSPClient.hh"
#include "ProxyServerMediaSession.hh"
#include "GroupsockHelper.hh"
#include "session.h"

LiveRTSPServer* LiveRTSPServer::createNew(UsageEnvironment& env, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
{
	Port rtspPort = 554;
	int ourSocket = setUpOurSocket(env, rtspPort);
	if (ourSocket == -1) return NULL;

	return new LiveRTSPServer(env, ourSocket, authDatabase, reclamationTestSeconds);
}

LiveRTSPServer::LiveRTSPServer(UsageEnvironment& env, int ourSocket,UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
	: RTSPServer(env, ourSocket, 554, authDatabase, reclamationTestSeconds)
{

}

LiveRTSPServer::~LiveRTSPServer()
{

}

ServerMediaSession* LiveRTSPServer::lookupServerMediaSession(char const* streamName)
{
	ServerMediaSession* sess = RTSPServer::lookupServerMediaSession(streamName);
	if (sess != NULL) {
		return sess;
	}

	std::string url = "rtsp://";
	url += streamName;
	portNumBits tunnelOverHTTPPortNum = (portNumBits)(~0);
	LiveServerMediaSession* session = LiveServerMediaSession::createNew(envir(), this,
		url.c_str(), streamName, NULL, NULL, tunnelOverHTTPPortNum, 2);
	addServerMediaSession(session);

	return session;
}

GenericMediaServer::ClientConnection* LiveRTSPServer::createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr)
{
	return new LiveRTSPClientConnection(*this, clientSocket, clientAddr);
}

LiveRTSPServer::LiveRTSPClientConnection
::LiveRTSPClientConnection(LiveRTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr)
: RTSPClientConnection(ourServer, clientSocket, clientAddr), m_liveServer(&ourServer), m_liveSession(NULL)
{
	char clientIpPort[256] = { 0 };
	sprintf(clientIpPort, "%s:%d", AddressString(clientAddr).val(), ntohs(clientAddr.sin_port));
	m_clientIpPort = clientIpPort;
}

LiveRTSPServer::LiveRTSPClientConnection::~LiveRTSPClientConnection()
{
	if (m_liveSession != NULL) {
		LiveServerMediaSession* sms = static_cast<LiveServerMediaSession*>(m_liveSession);
		sms->DeleteClientConnection(this);
	}
}

const std::string& LiveRTSPServer::LiveRTSPClientConnection::ClientIpPort()
{
	return m_clientIpPort;
}

void LiveRTSPServer::LiveRTSPClientConnection
::handleCmd_DESCRIBE(char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr) {
	ServerMediaSession* session = NULL;
	char* sdpDescription = NULL;
	char* rtspURL = NULL;
	do {
		char urlTotalSuffix[RTSP_PARAM_STRING_MAX];
		urlTotalSuffix[0] = '\0';
		if (urlPreSuffix[0] != '\0') {
			strcat(urlTotalSuffix, urlPreSuffix);
			strcat(urlTotalSuffix, "/");
		}
		strcat(urlTotalSuffix, urlSuffix);
		m_requestUrl = "rtsp://";
		m_requestUrl += urlTotalSuffix;
		
		if (!authenticationOK("DESCRIBE", urlTotalSuffix, fullRequestStr)) {
			break;
		}

		// We should really check that the request contains an "Accept:" #####
		// for "application/sdp", because that's what we're sending back #####

		// Begin by looking up the "ServerMediaSession" object for the specified "urlTotalSuffix":
		session = m_liveServer->lookupServerMediaSession(urlTotalSuffix);
		if (session == NULL) { // todo internal error
			break;
		} 

		m_liveSession = session;
		LiveServerMediaSession* sms = static_cast<LiveServerMediaSession*>(session);
		RTSP_CLIENT_STATUS status = sms->Status();
		if (status == RTSP_CONNECTING) {
			sms->AddClientConnection(this);
			strcpy(m_descCSeq, fCurrentCSeq);
			fResponseBuffer[0] = '\0';
			break;
		} else if (status == RTSP_DISCONNECT) {
			handleCmd_notFound();
			break;
		} 

		// Increment the "ServerMediaSession" object's reference count, in case someone removes it
		// while we're using it:
		session->incrementReferenceCount();

		// Then, assemble a SDP description for this session:
		sdpDescription = session->generateSDPDescription();
		if (sdpDescription == NULL) {
			// This usually means that a file name that was specified for a
			// "ServerMediaSubsession" does not exist.
			setRTSPResponse("404 File Not Found, Or In Incorrect Format");
			break;
		}
		unsigned sdpDescriptionSize = strlen(sdpDescription);

		// Also, generate our RTSP URL, for the "Content-Base:" header
		// (which is necessary to ensure that the correct URL gets used in subsequent "SETUP" requests).
		rtspURL = fOurRTSPServer.rtspURL(session, fClientInputSocket);

		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
			"RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
			"%s"
			"Content-Base: %s/\r\n"
			"Content-Type: application/sdp\r\n"
			"Content-Length: %d\r\n\r\n"
			"%s",
			fCurrentCSeq,
			dateHeader(),
			rtspURL,
			sdpDescriptionSize,
			sdpDescription);
	} while (0);

	if (session != NULL) {
		// Decrement its reference count, now that we're done using it:
		session->decrementReferenceCount();

		if (session->referenceCount() == 0 && session->deleteWhenUnreferenced()) {
			fOurServer.removeServerMediaSession(session);
		}
	}

	delete[] sdpDescription;
	delete[] rtspURL;
}

void LiveRTSPServer::LiveRTSPClientConnection::SendDescribeRespone(ServerMediaSession* session)
{
	char* sdpDescription = NULL;
	char* rtspURL = NULL;
	do {
		// Increment the "ServerMediaSession" object's reference count, in case someone removes it
		// while we're using it:
		session->incrementReferenceCount();

		// Then, assemble a SDP description for this session:
		sdpDescription = session->generateSDPDescription();
		if (sdpDescription == NULL) {
			// This usually means that a file name that was specified for a
			// "ServerMediaSubsession" does not exist.
			setRTSPResponse("404 File Not Found, Or In Incorrect Format");
			break;
		}
		unsigned sdpDescriptionSize = strlen(sdpDescription);

		// Also, generate our RTSP URL, for the "Content-Base:" header
		// (which is necessary to ensure that the correct URL gets used in subsequent "SETUP" requests).
		rtspURL = fOurRTSPServer.rtspURL(session, fClientInputSocket);

		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
			"RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
			"%s"
			"Content-Base: %s/\r\n"
			"Content-Type: application/sdp\r\n"
			"Content-Length: %d\r\n\r\n"
			"%s",
			m_descCSeq,
			dateHeader(),
			rtspURL,
			sdpDescriptionSize,
			sdpDescription);
	} while (0);

	if (session != NULL) {
		// Decrement its reference count, now that we're done using it:
		session->decrementReferenceCount();
		if (session->referenceCount() == 0 && session->deleteWhenUnreferenced()) {
			fOurServer.removeServerMediaSession(session);
		}
	}

	send(fClientOutputSocket, (char const*)fResponseBuffer, strlen((char*)fResponseBuffer), 0);

	delete[] sdpDescription;
	delete[] rtspURL;
}
