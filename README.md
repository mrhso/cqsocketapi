# CoolQ Socket API
VC++ 写的酷 Q UDP Socket API。

[![Download](https://api.bintray.com/packages/mrhso/cqsocketapi/master/images/download.svg)](https://bintray.com/mrhso/cqsocketapi/master/_latestVersion) [![Build status](https://ci.appveyor.com/api/projects/status/dfrm0xppsokcr2ks?svg=true)](https://ci.appveyor.com/project/mrhso/cqsocketapi)

## 安装
1. 下载 [me.cqp.ishisashi.cqsocketapi.cpk](https://dl.bintray.com/mrhso/cqsocketapi/me.cqp.ishisashi.cqsocketapi.cpk)，放入酷 Q 的 app 目录。
2. 启用插件「CoolQ Socket API (Node.js)」。
3. 重启酷 Q。

## 设定
插件目录的 config.ini 是设定档案。

## 使用
已附带 Node.js 之实现 QQBot.js，可以直接导入使用。

Docker 用户请参见[附录](https://github.com/mrhso/cqsocketapi/blob/master/omake.md#docker-的正确使用方法)。

## 协议
### 通信
服务端和客户端用 UDP 包通信。

默认监听端口 11235，可以修改设定。

客户端应每隔 5 分钟发送 ClientHello 以保持连接。

数据包内部各信息以空格分隔，基本格式如下：
```
Frame = Prefix (max 256) + Payload (max 32768)
```
布尔值 True、False 用 1、0 表示。

### 服务端数据包
```
'ServerHello' + Timeout + PrefixSize + PayloadSize + FrameSize
'PrivateMessage' + QQNum + EncodedText (Message) + SubType + MsgID + EncodedText (UserInfo)
'GroupMessage' + GroupID + QQNum + EncodedText (Message) + SubType + MsgID + EncodedText (UserInfo) + EncodedText (Anonymous)
'DiscussMessage' + DiscussID + QQNum + EncodedText (Message) + SubType + MsgID + EncodedText (UserInfo)
'GroupAdmin' + GroupId + SubType + QQNum + SendTime + EncodedText (UserInfo)
'GroupMemberDecrease' + GroupID + AdminQQNum + OperatedQQNum + SubType + SendTime + EncodedText (AdminInfo) + EncodedText (UserInfo)
'GroupMemberIncrease' + GroupID + AdminQQNum + OperatedQQNum + SubType + SendTime + EncodedText (UserInfo)
'GroupMemberInfo' + EncodedText (UserInfo)
'StrangerInfo' + EncodedText (UserInfo)
'LoginNick' + EncodedText (UserName)
'FriendAdded' + QQNum + SubType + SendTime + EncodedText (UserInfo)
'RequestAddFriend' + QQNum + EncodedText (Message) + EncodedText (Flag) + SubType + SendTime + EncodedText (UserInfo)
'RequestAddGroup' + GroupID + QQNum + EncodedText (Message) + EncodedText (Flag) + SubType + SendTime + EncodedText (UserInfo)
'GroupUpload' + GroupID + QQNum + EncodedText (FileInfo) + SubType + SendTime + EncodedText (UserInfo)
'GroupMemberList' + EncodedText (File) + EncodedText (CQRoot)
'Cookies' + EncodedText (Cookies) + EncodedText (Domain)
'CsrfToken' + EncodedText (CsrfToken)
'LoginQQ' + QQNum
'AppDirectory' + EncodedText (AppDirectory) + EncodedText (CQRoot)
'PrivateMessageID' + MsgID + Key
'GroupMessageID' + MsgID + Key
'DiscussMessageID' + MsgID + Key
'GroupList' + EncodedText (File) + EncodedText (CQRoot)
'Record' + EncodedText (File) + EncodedText (Source) + EncodedText (Format) + EncodedText (CQRoot)
'Image' + EncodedText (File) + EncodedText (Source) + EncodedText (CQRoot)
'CanSendImage' + CanSendImage
'CanSendRecord' + CanSendRecord
'GroupBan' + GroupID + AdminQQNum + OperatedQQNum + SubType + SendTime + EncodedText (AdminInfo) + EncodedText (UserInfo)
'GroupInfo' + EncodedText (GroupInfo)
'FriendList' + EncodedText (File) + EncodedText (CQRoot)
```
其中：
```
EncodedText = base64_encode(GB18030_encode(text))
```
CanSendImage、CanSendRecord 为布尔值。

UserInfo 的格式可参考附带的 QQBot.js。

#### 示例
```
GroupMessage 123456 10000 dGVzdCCy4srUIG5hgTCKN3ZlW0NROmVtb2ppLGlkPTEyODE2Ml0= 1 123456 (null)
```
应理解为：
```JSON
{
    "Type": "GroupMessage",
    "GroupID": 123456,
    "QQNum": 10000,
    "EncodedMsg": "dGVzdCCy4srUIG5hgTCKN3ZlW0NROmVtb2ppLGlkPTEyODE2Ml0=",
    "SubType": 1,
    "MsgID": 123456,
    "EncodedAnonymous": null
}
```
由于输入来自服务器，GroupID 解读为「来源群」。QQ 中的很多 ID，如 QQ 号、群号，都为正整数。
编码的 EncodedText 最终应解码至「test 测试 naïve💢」，其中「💢」（U+1F4A2）以 CQ 码「[CQ:emoji,id=128162]」呈现。

### 客户端数据包
```
'ClientHello' + Port
'PrivateMessage' + QQNum + EncodedText (Message) + Key
'GroupMessage' + GroupID + EncodedText (Message) + Key
'DiscussMessage' + DiscussID + EncodedText (Message) + Key
'GroupMemberInfo' + GroupID + QQNum + IsNotCached
'StrangerInfo' + QQNum + IsNotCached
'LoginNick'
'GroupBan' + GroupID + QQNum + Duration
'Like' + QQNum + Times
'GroupKick' + GroupID + QQNum + RejectAddRequest
'GroupAdmin' + GroupID + QQNum + SetAdmin
'GroupWholeBan' + GroupID + EnableBan
'GroupAnonymousBan' + GroupID + EncodedText (Anonymous)
'GroupAnonymous' + GroupID + EnableAnonymous
'GroupCard' + GroupID + QQNum + EncodedText (NewCard)
'GroupLeave' + GroupID + IsDismiss
'GroupSpecialTitle' + GroupID + QQNum + EncodedText (NewSpecialTitle) + Duration
'DiscussLeave' + DiscussID
'FriendAddRequest' + EncodedText (ResponseFlag) + ResponseOperation + EncodedText (Remark)
'GroupAddRequest' + EncodedText (ResponseFlag) + RequestType + ResponseOperation + EncodedText (Reason)
'GroupMemberList' + GroupID
'Cookies'
'CsrfToken'
'LoginQQ'
'AppDirectory'
'DeleteMessage' + MsgID
'GroupList'
'Record' + EncodedText (File) + EncodedText (Format)
'Image' + EncodedText (File)
'CanSendImage'
'CanSendRecord'
```
其中 IsNotCached、RejectAddRequest、SetAdmin、EnableBan、EnableAnonymous、IsDismiss 为布尔值，Duration 以秒为单位。

#### 示例
```
GroupMessage 123456 ZWZmaWNpZW5jeQ== 123456
```
应理解为：
```JSON
{
    "Type": "GroupMessage",
    "GroupID": 123456,
    "EncodedMsg": "ZWZmaWNpZW5jeQ==",
    "Number": 123456
}
```
此为传送到群号 123456 的群的一条信息，内容为「efficiency」。
