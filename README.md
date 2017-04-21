# CQSocketAPI
CoolQ Socket API written in VC++.


## Installation
1. Place `org.dazzyd.cqsocketapi.cpk` into CoolQ app folder.
2. Enable CQSocketAPI in CoolQ APP management window.
3. Restart CoolQ.


## QQBot
You can write your owned bot based on my [qqbot](https://github.com/yukixz/qqbot) in Python 3.

Node.js的實現可參考附帶的QQBot.js。需要`npm install --save-dev encoding`。

## Protocol

### Communication
The communication between server and clients is sending UDP frame to each other.

The server listens on port `11235`.

Clients should send `ClientHello` frame to inform server of client port.
`ClientHello` frame must be sent on startup and every (at most) 5 minutes.

A UDP frame contains one Prefix and multiple Fields.
Prefix and Fields are joined by one space.

### Server Sent Frame

```
Frame = Prefix (max 256) + Payload (max 32768)
```

一個Frame內部的各資訊以空白（U+0020）分隔。伺服端可接收以下訊息：

_請加入資料型態信息_

* `ServerHello`
* `PrivateMessage` + QQNum + EncodedText (Message) + SubType + SendTime + EncodedText (UserInfo)
* `GroupMessage` + GroupID + QQNum + EncodedText (Message) + SubType + SendTime + EncodedText (UserInfo)
* `DiscussMessage` + DiscussID + QQNum + EncodedText (Message) + SubType + SendTime + EncodedText (UserInfo)
* `GroupAdmin` + GroupId + SubType + QQNum + SendTime + EncodedText (UserInfo)
* `GroupMemberDecrease` + GroupID + AdminQQNum + OperatedQQNum + SubType + SendTime + EncodedText (Admin) + EncodedText (User)
* `GroupMemberIncrease` + GroupID + AdminQQNum + OperatedQQNum + SubType + SendTime + EncodedText (User)
* `GroupMemberInfo` + EncodedText (UserInfo)
* `StrangerInfo` + EncodedText (UserInfo)
* `LoginNick` + EncodedText (UserName)

其中：

```
EncodedText = base64_encode( GB18030_encode( text ) )
```

UserInfo的格式可參考附帶的QQBot.js。

#### Example Frame
```
GroupMessage 123456 10000 dGVzdCCy4srUILK7bmGBMIo3dmU=
```
應理解為：
```JSON
{
  "type": "GroupMessage",
  "GroupID": 123456,
  "QQNum": 10000,
  "EncodedText": "dGVzdCCy4srUIG5hgTCKN3ZlW0NROmVtb2ppLGlkPTEyODE2Ml0="
}
```
由於輸入來自伺服器，`GroupID`解讀為「來源群」。QQ中的很多ID，如QQ號、群號，都為正整數。
編碼的`EncodedText`最终应解码至`test 测试 naïve💢`，其中`💢`（U+1F4A2）以CQ格式`[CQ:emoji,id=128162]`呈現。

### Client Sent Frame
```
Frame = Prefix (max 256) + Payload (max 32768)
```

* `ClientHello` + Port
* `PrivateMessage` + QQNum + EncodedText
* `GroupMessage` + GroupID + EncodedText
* `DiscussMessage` + DiscussID + EncodedText
* `GroupMemberInfo` + GroupID + QQNum + IsNotCached `Boolean01`（0表示利用快取結果）
* `StrangerInfo` + QQNum + IsNotCached `Boolean01`
* `LoginNick`

其中：

```
EncodedText = base64_encode( GB18030_encode( text ) )
```

#### Example Frame

```
GroupMessage 123456 ZWZmaWNpZW5jeQ==
```
理解為：
```JSON
{
  "type": "GroupMessage",
  "GroupID": 123456,
  "base64Message": "ZWZmaWNpZW5jeQ=="
}
```
此為傳送到群號123456的群的一條信息，內容為`efficiency`。
