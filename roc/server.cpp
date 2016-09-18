#include "server.h"
#include "RTSPCommon.hh"
#include "RTSPClient.hh"
#include "session.h"
#include "GroupsockHelper.hh"

LiveRTSPServer* LiveRTSPServer::createNew(UsageEnvironment& env,
	UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
{
	Port rtspPort = 554;
	int ourSocket = setUpOurSocket(env, rtspPort);
	if (ourSocket == -1) return NULL;

	//increaseReceiveBufferTo(env, ourSocket, config->SocketBuffer());
	//increaseSendBufferTo(env, ourSocket, config->SocketBuffer());

	return new LiveRTSPServer(env, ourSocket, authDatabase, reclamationTestSeconds);
}

LiveRTSPServer::LiveRTSPServer(UsageEnvironment& env, int ourSocket,
	UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
	: RTSPServer(env, ourSocket, 554, authDatabase, reclamationTestSeconds)
{
	//m_config = config;
	//struct in_addr dummyAddr; dummyAddr.s_addr = 0;
	//m_eventSock = new Groupsock(env, dummyAddr, eventPort, 255);
	//increaseReceiveBufferTo(env, m_eventSock->socketNum(), 50 * 1024);
	//makeSocketNonBlocking(m_eventSock->socketNum());
	//	envir().taskScheduler().setBackgroundHandling(m_eventSock->socketNum(), SOCKET_READABLE | SOCKET_EXCEPTION,
	//		(TaskScheduler::BackgroundHandlerProc*)&IncomingEventHandler, this);
}

LiveRTSPServer::~LiveRTSPServer()
{

}

void LiveRTSPServer::IncomingEventHandler(void* clientData, int mask)
{

}

ServerMediaSession* LiveRTSPServer::lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession) {
	// check whether we already have a "ServerMediaSession" for this streamName
	ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
	if (sms != NULL) {
 		LiveServerMediaSession* lsms = (LiveServerMediaSession*)sms;
 		if (lsms->Status() == STATUS_CONNECT_OK) {
 			return sms;
 		}
	} else {
		std::string url = "rtsp://";
		url += streamName;
		char* username;
		char* password;
		NetAddress destAddress;
		portNumBits urlPortNum;
		char const* urlSuffix;

		if (!RTSPClient::parseRTSPURL(envir(), url.c_str(), username, password, destAddress, urlPortNum, &urlSuffix)) {
			//client->HandleUrlError();
			return NULL;
		}

		if (username != NULL) {
			delete[] username;
		}

		if (password != NULL) {
			delete[] password;
		}

		AddressString destIp = (*(netAddressBits*)(destAddress.data()));
		std::string destIPAddr = destIp.val();
		bool rtpOverTcp = true; // m_config->RtpOverTcp(destIPAddr);

		portNumBits tunnelOverHTTPPortNum = rtpOverTcp ? (portNumBits)(~0) : 0;
		//ServerMediaSession* session = LiveServerMediaSession::createNew(envir(), this,
		//	url.c_str(), streamName, username, password, tunnelOverHTTPPortNum);

//		addServerMediaSession(session);
	}

	return NULL;
}

GenericMediaServer::ClientConnection* LiveRTSPServer::createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr)
{
	//increaseReceiveBufferTo(envir(), clientSocket, m_config->SocketBuffer());
	//increaseSendBufferTo(envir(), clientSocket, m_config->SocketBuffer());
	return new LiveRTSPClientConnection(*this, clientSocket, clientAddr);
}

LiveRTSPServer::LiveRTSPClientConnection
::LiveRTSPClientConnection(LiveRTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr)
	: RTSPClientConnection(ourServer, clientSocket, clientAddr), m_server(&ourServer)
{
	char clientIpPort[256] = { 0 };
	sprintf(clientIpPort, "%s:%d", AddressString(clientAddr).val(), ntohs(clientAddr.sin_port));

}

LiveRTSPServer::LiveRTSPClientConnection::~LiveRTSPClientConnection()
{

}

const std::string& LiveRTSPServer::LiveRTSPClientConnection::ClientIpPort()
{
	return m_clientIpPort;
}

void LiveRTSPServer::LiveRTSPClientConnection::HandleStreamPendding()
{
	strcpy(fDescCSeq, fCurrentCSeq);
	fResponseBuffer[0] = '\0';
}

void LiveRTSPServer::LiveRTSPClientConnection::HandleUrlError()
{
	setRTSPResponse("404 Stream Not Found, In Incorrect Url Format");
}

void LiveRTSPServer::LiveRTSPClientConnection::HandleStreamError()
{
	setRTSPResponse("404 Stream Not Found, Try Again");
}

bool LiveRTSPServer::LiveRTSPClientConnection::SetRtpRule(const char* streamName)
{

	return true;
}

void LiveRTSPServer::LiveRTSPClientConnection::HandleSessionNotFound(char const* streamName) {

}

void LiveRTSPServer::LiveRTSPClientConnection::handleCmd_DESCRIBE(char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr) {
	ServerMediaSession* session = NULL;
	char* sdpDescription = NULL;
	char* rtspURL = NULL;
	do {
		char urlTotalSuffix[2 * RTSP_PARAM_STRING_MAX];
		// enough space for urlPreSuffix/urlSuffix'\0'
		urlTotalSuffix[0] = '\0';
		if (urlPreSuffix[0] != '\0') {
			strcat(urlTotalSuffix, urlPreSuffix);
			strcat(urlTotalSuffix, "/");
		}
		strcat(urlTotalSuffix, urlSuffix);

		if (!authenticationOK("DESCRIBE", urlTotalSuffix, fullRequestStr)) break;

		// We should really check that the request contains an "Accept:" #####
		// for "application/sdp", because that's what we're sending back #####

		// Begin by looking up the "ServerMediaSession" object for the specified "urlTotalSuffix":
		session = fOurServer.lookupServerMediaSession(urlTotalSuffix);
		if (session == NULL) {
			HandleSessionNotFound(urlTotalSuffix);
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

}

void LiveRTSPServer::OnGetStreamError(const char* url)
{

}

void LiveRTSPServer::OnGetStreamOK(const char* url)
{


}

void LiveRTSPServer::DeleteClientConnection(LiveRTSPClientConnection* client, const std::string& url)
{

}

void LiveRTSPServer::AddClientConnection(LiveRTSPClientConnection* client, const std::string& url)
{

}
