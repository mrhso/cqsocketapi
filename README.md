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

一個Frame內部的各資訊以空白分隔。伺服端可接收以下訊息：

* `ServerHello`
* `PrivateMessage` + QQ + EncodedText (Message) + SubType + SendTime + EncodedText (UserInfo)
* `GroupMessage` + GroupID + QQ + EncodedText (Message) + SubType + SendTime + EncodedText (UserInfo)
* `DiscussMessage` + DiscussID + QQ + EncodedText (Message) + SubType + SendTime + EncodedText (UserInfo)
* `GroupAdmin` + GroupId + SubType + QQ + SendTime + EncodedText (UserInfo)
* `GroupMemberDecrease` + GroupID + AdminQQ + OperatedQQ + SubType + SendTime + EncodedText (Admin) + EncodedText (User)
* `GroupMemberIncrease` + GroupID + AdminQQ + OperatedQQ + SubType + SendTime + EncodedText (User)
* `GroupMemberInfo` + EncodedText (UserInfo)
* `StrangerInfo` + EncodedText (UserInfo)
* `LoginNick` + EncodedText (UserName)

其中：

```
EncodedText = base64_encode( GBK_encode( text ) )
```

UserInfo的格式可參考附帶的QQBot.js。

### Client Sent Frame
```
Frame = Prefix (max 256) + Payload (max 32768)
```

* `ClientHello` + Port
* `PrivateMessage` + QQ + EncodedText
* `GroupMessage` + GroupID + EncodedText
* `DiscussMessage` + DiscussID + EncodedText
* `GroupMemberInfo` + GroupID + QQ + 0或1（0表示利用快取結果）
* `StrangerInfo` + QQ + 0或1
* `LoginNick`

其中：

```
EncodedText = base64_encode( GBK_encode( text ) )
```

### Example Frame
```
GroupMessage 123456 10000 dGVzdCCy4srU
```
