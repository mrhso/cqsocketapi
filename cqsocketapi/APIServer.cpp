#include "stdafx.h"
#include "string.h"
#include "cqp.h"
#include "base64.h"

#include "appmain.h"
#include "APIClient.h"
#include "APIServer.h"

extern APIClient *client;
extern int appAuthCode;


/********
 * Message Processer
 ********/
void prcsClientHello(const char *payload) {
	int port;
	sscanf_s(payload, "%d", &port);

	client->add(port);

	char* buffer = "ServerHello";
	client->send(buffer, strlen(buffer));
}

void prcsPrivateMessage(const char *payload) {
	int64_t id;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^\n]", &id, text, sizeof(char) * FRAME_PAYLOAD_SIZE);
	
	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendPrivateMsg(appAuthCode, id, decodedText);
}

void prcsGroupMessage(const char *payload) {
	int64_t id;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^\n]", &id, text, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendGroupMsg(appAuthCode, id, decodedText);
}

void prcsGroupBan(const char *payload) {
	int64_t group, qq, duration;
	sscanf_s(payload, "%I64d %I64d %I64d", &group, &qq, &duration);

	CQ_setGroupBan(appAuthCode, group, qq, duration);
}

void prcsDiscussMessage(const char *payload) {
	int64_t id;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^\n]", &id, text, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendDiscussMsg(appAuthCode, id, decodedText);
}

void prcsGroupMemberInfo(const char *payload) {
	int64_t group, qq;
	int32_t nocache;
	sscanf_s(payload, "%I64d %I64d %I32d", &group, &qq, &nocache);

	const char* result = CQ_getGroupMemberInfoV2(appAuthCode, group, qq, nocache);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMemberInfo %s", result);
	client->send(buffer, strlen(buffer));
}

void prcsStrangerInfo(const char *payload) {
	int64_t qq;
	int32_t nocache;
	sscanf_s(payload, "%I64d %I32d", &qq, &nocache);

	const char* result = CQ_getStrangerInfo(appAuthCode, qq, nocache);
	
	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "StrangerInfo %s", result);
	client->send(buffer, strlen(buffer));
}

void prcsLoginNick() {
	const char* result = CQ_getLoginNick(appAuthCode);

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, result, strlen(result));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "LoginNick %s", encoded_msg);
	client->send(buffer, strlen(buffer));
}

void prcsUnknownFramePrefix(const char *buffer) {
	char category[] = "UnknownFramePrefix";
	CQ_addLog(appAuthCode, CQLOG_WARNING, category, buffer);
}


/********
 * API Server
 ********/
APIServer::APIServer(void)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);
	
	localInfo.sin_family = AF_INET;
	localInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	localInfo.sin_port = htons(SERVER_PORT);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	bind(sock, (sockaddr *)&localInfo, sizeof(localInfo));
}


APIServer::~APIServer(void)
{
	closesocket(sock);
}


void APIServer::run()
{
	char* buffer = new char[FRAME_SIZE];
	char* prefix = new char[FRAME_PREFIX_SIZE];
	char* payload = new char[FRAME_PAYLOAD_SIZE];

	while (1) {
		memset(buffer, 0, sizeof(char) * FRAME_SIZE);
		memset(prefix, 0, sizeof(char) * FRAME_PREFIX_SIZE);
		memset(payload, 0, sizeof(char) * FRAME_PAYLOAD_SIZE);
		if (recv(sock, buffer, sizeof(char) * FRAME_SIZE, 0) != SOCKET_ERROR) {
			sscanf_s(buffer, "%s %[^\n]", prefix, sizeof(char) * FRAME_PREFIX_SIZE, payload, sizeof(char) * FRAME_PAYLOAD_SIZE);
			
			if (strcmp(prefix, "ClientHello") == 0) {
				prcsClientHello(payload);
				continue;
			}
			if (strcmp(prefix, "PrivateMessage") == 0) {
				prcsPrivateMessage(payload);
				continue;
			}
			if (strcmp(prefix, "GroupMessage") == 0) {
				prcsGroupMessage(payload);
				continue;
			}
			if (strcmp(prefix, "GroupBan") == 0) {
				prcsGroupBan(payload);
				continue;
			}
			if (strcmp(prefix, "DiscussMessage") == 0) {
				prcsDiscussMessage(payload);
				continue;
			}
			if (strcmp(prefix, "GroupMemberInfo") == 0) {
				prcsGroupMemberInfo(payload);
				continue;
			}
			if (strcmp(prefix, "StrangerInfo") == 0) {
				prcsStrangerInfo(payload);
				continue;
			}
			if (strcmp(prefix, "LoginNick") == 0) {
				prcsLoginNick();
				continue;
			}
			// Unknown prefix
			prcsUnknownFramePrefix(buffer);
		}
	}
}