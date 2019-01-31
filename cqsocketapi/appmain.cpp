#include "stdafx.h"
#include "string.h"
#include "process.h"
#include "cqp.h"

#include "appmain.h"
#include "base64.h"
#include "winsock2.h"
#include "APIClient.h"
#include "APIServer.h"
#include "sstream"

using namespace std;

APIClient *client = nullptr;
APIServer *server = nullptr;
int appAuthCode = -1;

char SERVER_ADDRESS[16] = "127.0.0.1";
int SERVER_PORT = 11235;
char CLIENT_ADDRESS[16] = "127.0.0.1";
int CLIENT_SIZE = 32;
int CLIENT_TIMEOUT = 300;
int FRAME_PREFIX_SIZE = 256;
int FRAME_PAYLOAD_SIZE = 32768;
int FRAME_SIZE = 33025;

unsigned __stdcall startServer(void *args) {
	server = new APIServer();
	server->run();
	return 0;
}

CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}

CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	appAuthCode = AuthCode;
	return 0;
}

CQEVENT(int32_t, __eventStartup, 0)() {
	return 0;
}

CQEVENT(int32_t, __eventExit, 0)() {
	delete client;
	delete server;
	return 0;
}

CQEVENT(int32_t, __eventEnable, 0)() {

	string configFolder = ".\\app\\" CQAPPID;
	string configFile = configFolder + "\\config.ini";

	if (GetFileAttributes(configFile.data()) == -1) {
		if (GetFileAttributes(configFolder.data()) == -1) {
			CreateDirectory(configFolder.data(), NULL);
		}
		CloseHandle(CreateFile(configFile.data(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
		CQ_addLog(appAuthCode, CQLOG_INFO, "提示信息", "配置文件不存在，将以默认值自动生成");
	}

	stringstream ss;  string value; int valueInt = -1; char valueString[16];

	#define tryGetString(id)\
	GetPrivateProfileString("Server", #id, NULL, valueString, sizeof(valueString), configFile.data());\
	if (strcmp(valueString, "") != 0) {strcpy_s(id, valueString);} else {\
		WritePrivateProfileString("Server", #id, id, configFile.data());\
	}\
	ss.clear(); value.clear();valueString[0] = '\0';\

	#define tryGetInt(id)\
	valueInt = GetPrivateProfileInt("Server", #id, -1, configFile.data());\
	if (valueInt != -1) {id = valueInt;} else {\
		ss << id; ss >> value;\
		WritePrivateProfileString("Server", #id, value.data(), configFile.data());\
	}\
	ss.clear(); value.clear();valueInt = -1;\

	tryGetString(SERVER_ADDRESS);
	tryGetInt(SERVER_PORT);
	tryGetString(CLIENT_ADDRESS);
	tryGetInt(CLIENT_SIZE);
	tryGetInt(CLIENT_TIMEOUT);
	tryGetInt(FRAME_PREFIX_SIZE);
	tryGetInt(FRAME_PAYLOAD_SIZE);

	FRAME_SIZE = (FRAME_PREFIX_SIZE + FRAME_PAYLOAD_SIZE + 1);

	client = new APIClient();

	unsigned tid;
	HANDLE thd;
	thd = (HANDLE)_beginthreadex(NULL, 0, startServer, NULL, 0, &tid);

	return 0;
}

CQEVENT(int32_t, __eventDisable, 0)() {

	return 0;
}

/*
* Type=21 私聊消息
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, int32_t font) {

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, msg, strlen(msg));

	const char* user_info = CQ_getStrangerInfo(appAuthCode, fromQQ, TRUE);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "PrivateMessage %I64d %s %I32d %I32d %s", fromQQ, encoded_msg, subType, sendTime, user_info);
	client->send(buffer, strlen(buffer));

	delete[] encoded_msg;
	delete[] user_info;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=2 群消息
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, msg, strlen(msg));

	const char* user_info = CQ_getGroupMemberInfoV2(appAuthCode, fromGroup, fromQQ, TRUE);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMessage %I64d %I64d %s %I32d %I32d %s %s", fromGroup, fromQQ, encoded_msg, subType, sendTime, user_info, fromAnonymous);
	client->send(buffer, strlen(buffer));

	delete[] encoded_msg;
	delete[] user_info;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=4 讨论组消息
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t sendTime, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font) {

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, msg, strlen(msg));

	const char* user_info = CQ_getStrangerInfo(appAuthCode, fromQQ, TRUE);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "DiscussMessage %I64d %I64d %s %I32d %I32d %s", fromDiscuss, fromQQ, encoded_msg, subType, sendTime, user_info);
	client->send(buffer, strlen(buffer));

	delete[] encoded_msg;
	delete[] user_info;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=101 群事件-管理员变动
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {

	const char* user_info = CQ_getGroupMemberInfoV2(appAuthCode, fromGroup, beingOperateQQ, TRUE);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupAdmin %I64d %I32d %I64d %I32d %s", fromGroup, subType, beingOperateQQ, sendTime, user_info);
	client->send(buffer, strlen(buffer));

	delete[] user_info;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=102 群事件-群成员减少
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	const char* user_info1 = "None";

	if (subType == 2 || subType == 3) {
		user_info1 = CQ_getGroupMemberInfoV2(appAuthCode, fromGroup, fromQQ, TRUE);
	}
	const char* user_info2 = CQ_getStrangerInfo(appAuthCode, beingOperateQQ, TRUE);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMemberDecrease %I64d %I64d %I64d %I32d %I32d %s %s", fromGroup, fromQQ, beingOperateQQ, subType, sendTime, user_info1, user_info2);
	client->send(buffer, strlen(buffer));

	delete[] user_info1;
	delete[] user_info2;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=103 群事件-群成员增加
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	const char* user_info = CQ_getGroupMemberInfoV2(appAuthCode, fromGroup, beingOperateQQ, TRUE);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMemberIncrease %I64d %I64d %I64d %I32d %I32d %s", fromGroup, fromQQ, beingOperateQQ, subType, sendTime, user_info);
	client->send(buffer, strlen(buffer));

	delete[] user_info;
	delete[] buffer;

	return EVENT_IGNORE;
}


/*
* Type=201 好友事件-好友已添加
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {

	const char* user_info = CQ_getStrangerInfo(appAuthCode, fromQQ, TRUE);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "FriendAdded %I64d %I32d %I32d %s", fromQQ, subType, sendTime, user_info);
	client->send(buffer, strlen(buffer));

	delete[] user_info;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=301 请求-好友添加
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE / 2];
	Base64encode(encoded_msg, msg, strlen(msg));

	char* encoded_flag = new char[FRAME_PAYLOAD_SIZE / 2];
	Base64encode(encoded_flag, responseFlag, strlen(responseFlag));

	const char* user_info = CQ_getStrangerInfo(appAuthCode, fromQQ, TRUE);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "RequestAddFriend %I64d %s %s %I32d %I32d %s", fromQQ, encoded_msg, encoded_flag, subType, sendTime, user_info);
	client->send(buffer, strlen(buffer));

	delete[] encoded_msg;
	delete[] encoded_flag;
	delete[] user_info;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=302 请求-群添加
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE / 2];
	Base64encode(encoded_msg, msg, strlen(msg));

	char* encoded_flag = new char[FRAME_PAYLOAD_SIZE / 2];
	Base64encode(encoded_flag, responseFlag, strlen(responseFlag));

	const char* user_info = CQ_getStrangerInfo(appAuthCode, fromQQ, TRUE);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "RequestAddGroup %I64d %I64d %s %s %I32d %I32d %s", fromGroup, fromQQ, encoded_msg, encoded_flag, subType, sendTime, user_info);
	client->send(buffer, strlen(buffer));

	delete[] encoded_msg;
	delete[] encoded_flag;
	delete[] user_info;
	delete[] buffer;

	return EVENT_IGNORE;
}

CQEVENT(int32_t, _eventGroupUpload, 28)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *file) {

	char* encoded_file = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_file, file, strlen(file));

	const char* user_info = CQ_getGroupMemberInfoV2(appAuthCode, fromGroup, fromQQ, TRUE);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupUpload %I64d %I64d %s %I32d %I32d %s", fromGroup, fromQQ, encoded_file, subType, sendTime, user_info);
	client->send(buffer, strlen(buffer));

	delete[] encoded_file;
	delete[] user_info;
	delete[] buffer;

	return EVENT_IGNORE;
}
