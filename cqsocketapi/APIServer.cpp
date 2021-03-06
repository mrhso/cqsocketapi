#include "stdafx.h"
#include "string.h"
#include "cqp.h"
#include "base64.h"

#include "appmain.h"
#include "APIClient.h"
#include "APIServer.h"

#include <string>
#include <fstream>
extern APIClient *client;
extern int appAuthCode;


/********
 * Message Processor
 ********/
void prcsClientHello(const char *payload) {
	int port;
	sscanf_s(payload, "%d", &port);

	client->add(port);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char),
		"ServerHello %I32d %I32d %I32d %I32d",
		CLIENT_TIMEOUT, FRAME_PREFIX_SIZE, FRAME_PAYLOAD_SIZE, FRAME_SIZE);

	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsSendPrivateMessage(const char *payload) {
	int64_t qq;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	int64_t key;
	sscanf_s(payload, "%I64d %[^ ] %I64d", &qq, text, sizeof(char) * FRAME_PAYLOAD_SIZE, &key);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	auto msgID = CQ_sendPrivateMsg(appAuthCode, qq, decodedText);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "PrivateMessageID %I32d %I64d", msgID, key);
	client->send(buffer, strlen(buffer));

	delete[] text;
	delete[] decodedText;
	delete[] buffer;
}

void prcsSendGroupMessage(const char *payload) {
	int64_t group;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	int64_t key;
	sscanf_s(payload, "%I64d %[^ ] %I64d", &group, text, sizeof(char) * FRAME_PAYLOAD_SIZE, &key);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	auto msgID = CQ_sendGroupMsg(appAuthCode, group, decodedText);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMessageID %I32d %I64d", msgID, key);
	client->send(buffer, strlen(buffer));

	delete[] text;
	delete[] decodedText;
	delete[] buffer;
}

void prcsSendDiscussMessage(const char *payload) {
	int64_t discuss;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	int64_t key;
	sscanf_s(payload, "%I64d %[^ ] %I64d", &discuss, text, sizeof(char) * FRAME_PAYLOAD_SIZE, &key);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	auto msgID = CQ_sendDiscussMsg(appAuthCode, discuss, decodedText);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "DiscussMessageID %I32d %I64d", msgID, key);
	client->send(buffer, strlen(buffer));

	delete[] text;
	delete[] decodedText;
	delete[] buffer;
}

void prcsSendLike(const char *payload) {
	int64_t qq;
	int32_t times;
	sscanf_s(payload, "%I64d %I32d", &qq, &times);

	CQ_sendLikeV2(appAuthCode, qq, times);
}

void prcsSetGroupKick(const char *payload) {
	CQBOOL rejectaddrequest;
	int64_t group, qq;
	sscanf_s(payload, "%I64d %I64d %I32d", &group, &qq, &rejectaddrequest);

	CQ_setGroupKick(appAuthCode, group, qq, rejectaddrequest);
}

void prcsSetGroupBan(const char *payload) {
	int64_t group, qq, duration;
	sscanf_s(payload, "%I64d %I64d %I64d", &group, &qq, &duration);

	CQ_setGroupBan(appAuthCode, group, qq, duration);
}

void prcsSetGroupAdmin(const char *payload) {
	CQBOOL setadmin;
	int64_t group, qq;
	sscanf_s(payload, "%I64d %I64d %I32d", &group, &qq, &setadmin);

	CQ_setGroupAdmin(appAuthCode, group, qq, setadmin);
}

void prcsSetGroupWholeBan(const char *payload) {
	CQBOOL enableban;
	int64_t group;
	sscanf_s(payload, "%I64d %I32d", &group, &enableban);

	CQ_setGroupWholeBan(appAuthCode, group, enableban);
}

void prcsSetGroupAnonymousBan(const char *payload) {
	int64_t group, duration;
	char* anonymous = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^ ] %I64d", &group, anonymous, sizeof(char) * FRAME_PAYLOAD_SIZE, &duration);

	CQ_setGroupAnonymousBan(appAuthCode, group, anonymous, duration);

	delete[] anonymous;
}

void prcsSetGroupAnonymous(const char *payload) {
	CQBOOL enableanonymous;
	int64_t group;
	sscanf_s(payload, "%I64d %I32d", &group, &enableanonymous);

	CQ_setGroupAnonymous(appAuthCode, group, enableanonymous);
}

void prcsSetGroupCard(const char *payload) {
	int64_t group, qq;
	char* newcard = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %I64d %[^ ]", &group, &qq, newcard, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedNewcard = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedNewcard, newcard);

	CQ_setGroupCard(appAuthCode, group, qq, decodedNewcard);

	delete[] newcard;
	delete[] decodedNewcard;
}

void prcsSetGroupLeave(const char *payload) {
	CQBOOL isdismiss;
	int64_t group;
	sscanf_s(payload, "%I64d %I32d", &group, &isdismiss);

	CQ_setGroupLeave(appAuthCode, group, isdismiss);
}

void prcsSetGroupSpecialTitle(const char *payload) {
	int64_t group, qq, duration;
	char* newspecialtitle = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %I64d %[^ ] %I64d", &group, &qq, newspecialtitle, sizeof(char) * FRAME_PAYLOAD_SIZE, &duration);

	char* decodedNewspecialtitle = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedNewspecialtitle, newspecialtitle);

	CQ_setGroupSpecialTitle(appAuthCode, group, qq, decodedNewspecialtitle, duration);

	delete[] newspecialtitle;
	delete[] decodedNewspecialtitle;
}

void prcsSetDiscussLeave(const char *payload) {
	int64_t discuss;
	sscanf_s(payload, "%I64d", &discuss);

	CQ_setDiscussLeave(appAuthCode, discuss);
}

void prcsFriendAddRequest(const char *payload) {
	int32_t responseoperation;
	char* responseflag = new char[FRAME_PAYLOAD_SIZE];
	char* remark = new char[FRAME_PAYLOAD_SIZE];

	sscanf_s(payload, "%[^ ] %I32d %[^ ]",
		responseflag, sizeof(char) * FRAME_PAYLOAD_SIZE,
		&responseoperation,
		remark, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedResponseflag = new char[FRAME_PAYLOAD_SIZE];
	char* decodedRemark = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedResponseflag, responseflag);
	Base64decode(decodedRemark, remark);

	CQ_setFriendAddRequest(appAuthCode, decodedResponseflag, responseoperation, decodedRemark);

	delete[] responseflag;
	delete[] remark;
	delete[] decodedResponseflag;
	delete[] decodedRemark;
}

void prcsGroupAddRequest(const char *payload) {
	int32_t requesttype;
	int32_t responseoperation;
	char* responseflag = new char[FRAME_PAYLOAD_SIZE];
	char* reason = new char[FRAME_PAYLOAD_SIZE];

	sscanf_s(payload, "%[^ ] %I32d %I32d %[^ ]",
		responseflag, sizeof(char) * FRAME_PAYLOAD_SIZE,
		&requesttype,
		&responseoperation,
		reason, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedResponseflag = new char[FRAME_PAYLOAD_SIZE];
	char* decodedReason = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedResponseflag, responseflag);
	Base64decode(decodedReason, reason);

	CQ_setGroupAddRequestV2(appAuthCode, decodedResponseflag, requesttype, responseoperation, decodedReason);

	delete[] responseflag;
	delete[] reason;
	delete[] decodedResponseflag;
	delete[] decodedReason;
}

void prcsGetGroupMemberInfo(const char *payload) {
	CQBOOL nocache;
	int64_t group, qq;
	sscanf_s(payload, "%I64d %I64d %I32d", &group, &qq, &nocache);

	auto info = CQ_getGroupMemberInfoV2(appAuthCode, group, qq, nocache);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMemberInfo %s", info);
	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsGetGroupMemberList(const char *payload) {
	int64_t group;
	sscanf_s(payload, "%I64d", &group);
	char* encoded_path = new char[FRAME_PAYLOAD_SIZE];
	std::string appPath(CQ_getAppDirectory(appAuthCode));
	std::string cachePath = appPath + "cache\\";

	std::string filename = std::string(cachePath) + std::to_string(group) + ".gml";
	std::ofstream fout(filename.c_str(), std::ofstream::out);
	auto list = CQ_getGroupMemberList(appAuthCode, group);
	if (fout.is_open()) {
		fout << std::string(list);
		fout.close();
	}
	Base64encode(encoded_path, filename.c_str(), strlen(filename.c_str()));

	char* encoded_appDir = new char[FRAME_PAYLOAD_SIZE];
	auto appDir = CQ_getAppDirectory(appAuthCode);
	Base64encode(encoded_appDir, appDir, strlen(appDir));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMemberList %s %s", encoded_path, encoded_appDir);
	client->send(buffer, strlen(buffer));

	delete[] encoded_path;
	delete[] encoded_appDir;
	delete[] buffer;
}

void prcsGetStrangerInfo(const char *payload) {
	CQBOOL nocache;
	int64_t qq;
	sscanf_s(payload, "%I64d %I32d", &qq, &nocache);

	auto info = CQ_getStrangerInfo(appAuthCode, qq, nocache);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "StrangerInfo %s", info);
	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsGetCookies(const char *payload) {

	char* domain = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%[^ ]", domain, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedDomain = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedDomain, domain);

	char* encoded_cookies = new char[FRAME_PAYLOAD_SIZE];
	auto cookies = CQ_getCookiesV2(appAuthCode, decodedDomain);
	Base64encode(encoded_cookies, cookies, strlen(cookies));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "Cookies %s %s", encoded_cookies, domain);
	client->send(buffer, strlen(buffer));

	delete[] decodedDomain;
	delete[] encoded_cookies;
	delete[] buffer;
}

void prcsGetCsrfToken() {

	int32_t token = CQ_getCsrfToken(appAuthCode);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "CsrfToken %I32d", token);
	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsGetLoginQQ() {

	int64_t qq = CQ_getLoginQQ(appAuthCode);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "LoginQQ %I64d", qq);
	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsGetLoginNickname() {

	char* encoded_nickname = new char[FRAME_PAYLOAD_SIZE];
	auto nickname = CQ_getLoginNick(appAuthCode);
	Base64encode(encoded_nickname, nickname, strlen(nickname));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "LoginNick %s", encoded_nickname);
	client->send(buffer, strlen(buffer));

	delete[] encoded_nickname;
	delete[] buffer;
}

void prcsGetAppDirectory() {

	char* encoded_appDir = new char[FRAME_PAYLOAD_SIZE];
	auto appDir = CQ_getAppDirectory(appAuthCode);
	Base64encode(encoded_appDir, appDir, strlen(appDir));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "AppDirectory %s", encoded_appDir);
	client->send(buffer, strlen(buffer));

	delete[] encoded_appDir;
	delete[] buffer;
}

void prcsDeleteMessage(const char *payload) {

	int64_t msgID;
	sscanf_s(payload, "%I64d", &msgID);

	CQ_deleteMsg(appAuthCode, msgID);
}

void prcsGetGroupList() {

	char* encoded_path = new char[FRAME_PAYLOAD_SIZE];
	std::string appPath(CQ_getAppDirectory(appAuthCode));
	std::string cachePath = appPath + "cache\\";

	std::string filename = std::string(cachePath) + "list.gl";
	std::ofstream fout(filename.c_str(), std::ofstream::out);
	auto list = CQ_getGroupList(appAuthCode);
	if (fout.is_open()) {
		fout << std::string(list);
		fout.close();
	}
	Base64encode(encoded_path, filename.c_str(), strlen(filename.c_str()));

	char* encoded_appDir = new char[FRAME_PAYLOAD_SIZE];
	auto appDir = CQ_getAppDirectory(appAuthCode);
	Base64encode(encoded_appDir, appDir, strlen(appDir));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupList %s %s", encoded_path, encoded_appDir);
	client->send(buffer, strlen(buffer));

	delete[] encoded_path;
	delete[] encoded_appDir;
	delete[] buffer;
}

void prcsGetRecord(const char *payload) {

	char* file = new char[FRAME_PAYLOAD_SIZE];
	char* outFormat = new char[FRAME_PAYLOAD_SIZE];

	sscanf_s(payload, "%[^ ] %[^ ]", file, sizeof(char) * FRAME_PAYLOAD_SIZE, outFormat, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedFile = new char[FRAME_PAYLOAD_SIZE];
	char* decodedOutFormat = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedFile, file);
	Base64decode(decodedOutFormat, outFormat);

	char* encodedRecord = new char[FRAME_PAYLOAD_SIZE];
	auto record = CQ_getRecordV2(appAuthCode, decodedFile, decodedOutFormat);
	Base64encode(encodedRecord, record, strlen(record));

	char* encoded_appDir = new char[FRAME_PAYLOAD_SIZE];
	auto appDir = CQ_getAppDirectory(appAuthCode);
	Base64encode(encoded_appDir, appDir, strlen(appDir));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "Record %s %s %s %s", encodedRecord, file, outFormat, encoded_appDir);
	client->send(buffer, strlen(buffer));

	delete[] file;
	delete[] outFormat;
	delete[] decodedFile;
	delete[] decodedOutFormat;
	delete[] encodedRecord;
	delete[] encoded_appDir;
	delete[] buffer;
}

void prcsGetImage(const char *payload) {

	char* file = new char[FRAME_PAYLOAD_SIZE];

	sscanf_s(payload, "%[^ ]", file, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedFile = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedFile, file);

	char* encodedImage = new char[FRAME_PAYLOAD_SIZE];
	auto image = CQ_getImage(appAuthCode, decodedFile);
	Base64encode(encodedImage, image, strlen(image));

	char* encoded_appDir = new char[FRAME_PAYLOAD_SIZE];
	auto appDir = CQ_getAppDirectory(appAuthCode);
	Base64encode(encoded_appDir, appDir, strlen(appDir));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "Image %s %s %s", encodedImage, file, encoded_appDir);
	client->send(buffer, strlen(buffer));

	delete[] file;
	delete[] decodedFile;
	delete[] encodedImage;
	delete[] encoded_appDir;
	delete[] buffer;
}

void prcsCanSendImage() {

	auto boolean = CQ_canSendImage(appAuthCode);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "CanSendImage %I32d", boolean);
	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsCanSendRecord() {

	auto boolean = CQ_canSendRecord(appAuthCode);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "CanSendRecord %I32d", boolean);
	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsGetGroupInfo(const char *payload) {
	CQBOOL nocache;
	int64_t group;
	sscanf_s(payload, "%I64d %I32d", &group, &nocache);

	auto info = CQ_getGroupInfo(appAuthCode, group, nocache);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupInfo %s", info);
	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsGetFriendList(const char *payload) {

	CQBOOL reserved;
	sscanf_s(payload, "%I32d", &reserved);

	char* encoded_path = new char[FRAME_PAYLOAD_SIZE];
	std::string appPath(CQ_getAppDirectory(appAuthCode));
	std::string cachePath = appPath + "cache\\";

	std::string filename = std::string(cachePath) + "list.fl";
	std::ofstream fout(filename.c_str(), std::ofstream::out);
	auto list = CQ_getFriendList(appAuthCode, reserved);
	if (fout.is_open()) {
		fout << std::string(list);
		fout.close();
	}
	Base64encode(encoded_path, filename.c_str(), strlen(filename.c_str()));

	char* encoded_appDir = new char[FRAME_PAYLOAD_SIZE];
	auto appDir = CQ_getAppDirectory(appAuthCode);
	Base64encode(encoded_appDir, appDir, strlen(appDir));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "FriendList %s %s", encoded_path, encoded_appDir);
	client->send(buffer, strlen(buffer));

	delete[] encoded_path;
	delete[] encoded_appDir;
	delete[] buffer;
}

//////////////////////////////////////////////////////////////////////////
//	Not Implemented
//////////////////////////////////////////////////////////////////////////
//	CQ_setFatal
//	CQ_addLog

void prcsUnknownFramePrefix(const char *buffer) {
	char category[] = "UnknownFramePrefix";
	CQ_addLog(appAuthCode, CQLOG_WARNING, category, buffer);
}


/********
 * API Server
 ********/
APIServer::APIServer(void) {
	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);

	localInfo.sin_family = AF_INET;
	localInfo.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
	localInfo.sin_port = htons(SERVER_PORT);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	bind(sock, (sockaddr *)&localInfo, sizeof(localInfo));
}


APIServer::~APIServer(void) {
	closesocket(sock);
}


void APIServer::run() {
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
				prcsSendPrivateMessage(payload);
				continue;
			}
			if (strcmp(prefix, "GroupMessage") == 0) {
				prcsSendGroupMessage(payload);
				continue;
			}
			if (strcmp(prefix, "DiscussMessage") == 0) {
				prcsSendDiscussMessage(payload);
				continue;
			}

			if (strcmp(prefix, "Like") == 0) {
				prcsSendLike(payload);
				continue;
			}

			if (strcmp(prefix, "GroupKick") == 0) {
				prcsSetGroupKick(payload);
				continue;
			}

			if (strcmp(prefix, "GroupBan") == 0) {
				prcsSetGroupBan(payload);
				continue;
			}

			if (strcmp(prefix, "GroupAdmin") == 0) {
				prcsSetGroupAdmin(payload);
				continue;
			}

			if (strcmp(prefix, "GroupWholeBan") == 0) {
				prcsSetGroupWholeBan(payload);
				continue;
			}

			if (strcmp(prefix, "GroupAnonymousBan") == 0) {
				prcsSetGroupAnonymousBan(payload);
				continue;
			}

			if (strcmp(prefix, "GroupAnonymous") == 0) {
				prcsSetGroupAnonymous(payload);
				continue;
			}

			if (strcmp(prefix, "GroupCard") == 0) {
				prcsSetGroupCard(payload);
				continue;
			}

			if (strcmp(prefix, "GroupLeave") == 0) {
				prcsSetGroupLeave(payload);
				continue;
			}

			if (strcmp(prefix, "GroupSpecialTitle") == 0) {
				prcsSetGroupSpecialTitle(payload);
				continue;
			}

			if (strcmp(prefix, "DiscussLeave") == 0) {
				prcsSetDiscussLeave(payload);
				continue;
			}

			if (strcmp(prefix, "FriendAddRequest") == 0) {
				prcsFriendAddRequest(payload);
				continue;
			}

			if (strcmp(prefix, "GroupAddRequest") == 0) {
				prcsGroupAddRequest(payload);
				continue;
			}

			if (strcmp(prefix, "GroupMemberInfo") == 0) {
				prcsGetGroupMemberInfo(payload);
				continue;
			}

			if (strcmp(prefix, "GroupMemberList") == 0) {
				prcsGetGroupMemberList(payload);
				continue;
			}

			if (strcmp(prefix, "StrangerInfo") == 0) {
				prcsGetStrangerInfo(payload);
				continue;
			}

			if (strcmp(prefix, "Cookies") == 0) {
				prcsGetCookies(payload);
				continue;
			}

			if (strcmp(prefix, "CsrfToken") == 0) {
				prcsGetCsrfToken();
				continue;
			}

			if (strcmp(prefix, "LoginQQ") == 0) {
				prcsGetLoginQQ();
				continue;
			}

			if (strcmp(prefix, "LoginNick") == 0) {
				prcsGetLoginNickname();
				continue;
			}

			if (strcmp(prefix, "AppDirectory") == 0) {
				prcsGetAppDirectory();
				continue;
			}

			if (strcmp(prefix, "DeleteMessage") == 0) {
				prcsDeleteMessage(payload);
				continue;
			}

			if (strcmp(prefix, "GroupList") == 0) {
				prcsGetGroupList();
				continue;
			}

			if (strcmp(prefix, "Record") == 0) {
				prcsGetRecord(payload);
				continue;
			}

			if (strcmp(prefix, "Image") == 0) {
				prcsGetImage(payload);
				continue;
			}

			if (strcmp(prefix, "CanSendImage") == 0) {
				prcsCanSendImage();
				continue;
			}

			if (strcmp(prefix, "CanSendRecord") == 0) {
				prcsCanSendRecord();
				continue;
			}

			if (strcmp(prefix, "GroupInfo") == 0) {
				prcsGetGroupInfo(payload);
				continue;
			}

			if (strcmp(prefix, "FriendList") == 0) {
				prcsGetFriendList(payload);
				continue;
			}

			// Unknown prefix
			prcsUnknownFramePrefix(buffer);
		}
	}

}