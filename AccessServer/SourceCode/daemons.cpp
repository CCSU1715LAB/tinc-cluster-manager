#include "daemons.h"

Daemons::Daemons(QObject *parent) : QObject(parent)
{
    // 基础变量初始化
    Dir_Path = "/etc/tinc/";            // Tinc工作路径
    content = nullptr;
    content_length = 0;
    Net_count = 0;
    m_tcpServer = new QTcpServer(this);
    socket_result = 0;          // 默认处理结果为0

    Extranet_IP = Daemons::GetIp();     // 获取外网IP
//    Extranet_IP = "8.140.28.140";         // 暂时写死
    qDebug() << "当前服务器外网IP为:"<< Extranet_IP;

    if(Extranet_IP.isEmpty())
    {
        exit(1);
    }

    // 定时器初始化
    Timer = new QTimer(this);
    Timer -> setInterval(10000);        // 十秒触发一次

    // 基础环境初始化
    QDir tinc(Dir_Path);
    if(!tinc.exists())
    {
        qDebug() << "Tinc目录未安装！";
        qDebug() << "开始安装Tinc";
        this -> Invoke_command("sudo apt-get install tinc\n");
        if(!tinc.exists())
        {
            exit(1);
        }
    }

    // 尝试监听端口，若监听成功，输出成功信息，若输出失败，则输出失败信息！
    if(!m_tcpServer -> listen(QHostAddress::Any, 55555))
    {
        qDebug() << "Listen the port failed!Server was not started!";
        exit(1);
    }
    else
        qDebug() << "Listen the port 55555!Server started!";
    // 每当有新的连接被建立，执行newConnectionSlot函数
    connect(m_tcpServer,&QTcpServer::newConnection,this,&Daemons::newConnectionSlot);
    // 当连接的接收发生错误时，执行errorStringSlot函数
    connect(m_tcpServer,&QTcpServer::acceptError,this,&Daemons::errorStringSlot);
    // 当数据传输成功接收，发送响应消息，执行Send_respond_Info函数
    connect(this,&Daemons::Receive_Success,this,&Daemons::Send_respond_Info);
    // 当数据成功被解析，处理数据，执行Data_Processing函数
    connect(this,&Daemons::Analysis_Success,this,&Daemons::Data_Processing);
    // 当需要创建内网的时候，调用Net_Create函数
    connect(this,&Daemons::Net_Create_Signal,this,&Daemons::Net_Create);
    // 当需要删除内网的时候，调用Net_Delete函数
    connect(this,&Daemons::Net_Delete_Signal,this,&Daemons::Net_Delete);
    // 当需要创建新的节点的时候，调用Node_Create函数
    connect(this,&Daemons::Node_Create_Signal,this,&Daemons::Node_Create);
    // 当需要删除节点的时候，调用Node_Delete函数
    connect(this,&Daemons::Node_Delete_Signal,this,&Daemons::Node_Delete);
    // 每十秒检测一次内网状态
    connect(Timer,&QTimer::timeout,this,&Daemons::Check_Net);


    Timer -> start();
}

// 当有新的连接产生时，调用的槽函数
void Daemons::newConnectionSlot()
{
    // 打印新的连接信息
    qDebug() << "There have a new Connection!";
    // 创建套接字对象
    m_tcpSocket = m_tcpServer -> nextPendingConnection();
    // 当数据传输完毕之后，执行Analysis_Msg函数
    connect(m_tcpSocket,&QTcpSocket::readyRead,this,&Daemons::Analysis_Msg);
    // 打印新连接的相关信息
    qDebug() << m_tcpSocket ->peerAddress().toString() << ":" << m_tcpSocket ->peerPort();
}

// 打印错误日志
void Daemons::errorStringSlot()
{
    // 打印错误信息！
    qDebug() << m_tcpServer -> errorString();
}

 // 爬取网页信息获取外网IP
//QString Daemons::GetIp()
//{
//    QNetworkAccessManager networkManager;
//    QString ip;

//    // Using OpenDNS to get the external IP
//    QHostInfo::lookupHost("myip.opendns.com", &networkManager, [&](const QHostInfo &hostInfo) {
//        if (hostInfo.error() == QHostInfo::NoError && !hostInfo.addresses().isEmpty()) {
//            const QHostAddress &address = hostInfo.addresses().first();
//            QNetworkRequest request(QUrl(QString("https://api.ipify.org?format=text")));
//            QNetworkReply *reply = networkManager.get(request);
//            QEventLoop loop;
//            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//            loop.exec();
//            if (reply->error() == QNetworkReply::NoError) {
//                ip = QString(reply->readAll()).trimmed();
//            } else {
//                qWarning() << reply->errorString();
//            }
//            reply->deleteLater();
//        } else {
//            qWarning() << hostInfo.errorString();
//        }
//    });

//    return ip;
//}

QString Daemons::GetIp()
{
    QProcess process;
    process.start("curl", QStringList() << "cip.cc");
    process.waitForFinished(-1);
    QString result = process.readAllStandardOutput();

    // 提取IP地址
    QRegularExpression regex("\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b");
    QRegularExpressionMatch match = regex.match(result);
    QString ip = match.hasMatch() ? match.captured() : "";

    return ip;
}


// 解析数据
void Daemons::Analysis_Msg()
{
    // 接收数据
    data = m_tcpSocket -> readAll();
    requestInfo = data;

    // 打印数据信息
    qDebug() << "Receive data from client:\n" << requestInfo.toUtf8();

    // 判断信息是否为空
    if (data.isEmpty()) {
        QString errorMsg = "The message is empty!";
        m_tcpSocket->write(errorMsg.toUtf8());
        m_tcpSocket->flush();
        m_tcpSocket->waitForBytesWritten();
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->deleteLater();
        qDebug() << "Error: " << errorMsg << " Close connection due to empty message";
        return;
    }

    // 接收数据完成，发送响应消息
//    emit Daemons::Receive_Success();

    qDebug() << "下面开始解析数据！";

    // 实例化josn检测错误对象，该对象用于解析json的格式是否正确
    QJsonParseError jsonError;
    // 实例化Json对象
    QJsonDocument document = QJsonDocument::fromJson(data,&jsonError);
    // 判断json是否无错
    if(jsonError.error != QJsonParseError::NoError)
    {
        qDebug() << "Json格式错误！";
        return;
    }
    // 判断传输的数据是否合法
    if(document.isArray())
    {
        // 获取json数组
        QJsonArray JsArray = document.array();

        // 循环遍历数组，批次处理
        for(const QJsonValue &value : JsArray)
        {
            if(value.isObject())
            {
                Json = value.toObject();

                Object = Json["Object"].toString();
                Operation = Json["Operation"].toString();
                Name = Json["Name"].toString();
                // 发射解析成功信号
                emit Daemons::Analysis_Success();
            }
        }

    }
    else
    {
        qDebug() << "信息解析出错！非JiuYang协议";
        emit Daemons::Receive_Success();
        return;
    }

    if(Object == nullptr || Operation == nullptr || Name == nullptr)
    {
        qDebug() << "信息解析出错！非JiuYang协议";
        emit Daemons::Receive_Success();
        return;
    }

    // 接收数据完成，发送响应消息
    emit Daemons::Receive_Success();
}

// 发送响应消息
void Daemons::Send_respond_Info()
{
    QJsonObject res;

    if(socket_result == 1)
    {
        res.insert("info",QJsonValue("Success"));
    }
    else
    {
        res.insert("info",QJsonValue("Failure"));
    }

    res.insert("information", QJsonValue("I have already receive the message!"));
    res.insert("details",respond_details);
    if(Operation == "Create" && Object == "Net")
    {
        QFile main_file(Dir_Path + Name + "/hosts/main");
        main_file.open(QFile::ReadOnly | QIODevice::Text);

        QTextStream in(&main_file);
        in.setCodec("UTF-8");

        QString main_str = in.readAll();
        res.insert("config", main_str);
    }
    QJsonDocument documentJson(res);
    QByteArray array = documentJson.toJson();
    m_tcpSocket -> write(array);
    m_tcpSocket->flush();
    m_tcpSocket->waitForBytesWritten();
    // 关闭连接
    m_tcpSocket->disconnectFromHost();
    m_tcpSocket->deleteLater();
    qDebug() << "Close connection after sending response";
    socket_result = 0;
    respond_details.clear();
}

// 数据处理函数
void Daemons::Data_Processing()
{
    qDebug() << "Data_Processing!";
    qDebug() << "Object:" << Object;
    qDebug() << "Operation:" << Operation;
    qDebug() << "Name:" << Name;
    qDebug() << "Extranet IP :" << Extranet_IP;

    if(Operation == "Create" && Object == "Net")        // 创建内网
    {
        IP = Json["IP"].toString();                     // 解析内网IP
        Port = Json["Port"].toInt();                    // 解析内网端口
        Geteway = Json["Geteway"].toString();           // 解析内网网关
        qDebug() << "Port:" << Port;
        emit Net_Create_Signal();
    }
    else if(Operation == "Create" && Object == "Node")  // 接收节点
    {
        Internet = Json["Internet"].toString();         // 解析节点所属内网名称
        content = Json["content"].toString();           // 解析节点公钥文件
        emit Node_Create_Signal();
    }
    else if(Operation == "Delete" && Object == "Net")   // 删除内网
    {
        emit Net_Delete_Signal();                       // 发送内网删除信号
    }
    else if(Operation == "Delete" && Object == "Node")  // 删除节点
    {
        Internet = Json["Internet"].toString();
        emit Node_Delete_Signal();                      // 发送节点删除信号
    }
    else if(Operation == "Able" && Object == "Net")     // 启动内网
    {

        this -> Invoke_command("systemctl restart tinc@" + Name + ".service");
    }
    else if(Operation == "Disable" && Object == "Net")  // 关闭内网
    {
        this -> Invoke_command("systemctl stop tinc@" + Name + ".service");
    }
}

// 节点的接收函数
void Daemons::Node_Create()
{
    qDebug() << "接收节点文件！";
    QDir Tinc_dir(Dir_Path);
    // 判断文件夹是否存在
    if(!Tinc_dir.exists())
    {
        qDebug() << "Tinc工作:" << Dir_Path <<"路径不存在！";
        return ;
    }
    // 获取当前服务器所有内网
    Net_List = Tinc_dir.entryList(QDir::Dirs|QDir::Readable|QDir::NoDotAndDotDot,QDir::Name);
    // 检测内网名称是否已经存在
    if(!Net_List.contains(Internet,Qt::CaseInsensitive))
    {
        qDebug() << "节点所属内网不存在！要求加入的节点名称为：" << Name;
        return;
    }
    // 创建新的节点文件
    QFile node_file(Dir_Path + "/" + Internet + "/hosts/" + Name);

    if(node_file.open(QFile::WriteOnly | QIODevice::Text | (node_file.exists() ? QFile::Truncate : QFile::Append)))
    {
        node_file.write(content.toUtf8());
        qDebug() << "节点添加成功！";
    }
    node_file.close();

    socket_result = 1;      // 接收节点成功
}

// 节点的删除函数
void Daemons::Node_Delete()
{
    // 检测节点是否存在

    qDebug() << "正在删除内网" << Internet << "下的节点:" << Name;
    this -> Invoke_command("rm -f " + Dir_Path + Internet + "/hosts/" + Name);

    socket_result = 1;      // 删除节点成功
}

// 内网创建函数
void Daemons::Net_Create()
{
    qDebug() << "该操作为内网添加操作！";
    QDir Tinc_dir(Dir_Path);
    // 判断文件夹是否存在
    if(!Tinc_dir.exists())
    {
        qDebug() << "Tinc工作:" << Dir_Path <<"路径不存在！";
        return ;
    }
    // 获取当前服务器所有内网
    Net_List = Tinc_dir.entryList(QDir::Dirs|QDir::Readable|QDir::NoDotAndDotDot,QDir::Name);
    // 检测内网名称是否已经存在
    if(Net_List.contains(Name,Qt::CaseInsensitive))
    {
        qDebug() << "需要创建的内网已经存在！内网名为：" << Name;
        qDebug() << "此次通讯失效！";
        return;
    }

    // 内网不存在，创建内网！
    if(!Tinc_dir.mkdir(Name) || !Tinc_dir.mkdir(Name + "/hosts"))
    {
        qDebug() << "创建新的内网失败！";
        qDebug() << "失败原因：创建内网文件夹失败！";
        return ;
    }

    // 创建内网配置文件tinc.conf
    QFile conf_file(Dir_Path + Name + "/tinc.conf");  // 创建文件
    if (conf_file.open(QFile::WriteOnly | QFile::Append | QIODevice::Text))
    { // 打开文件, 文件为追加写入
        QString conf_str = QString("Name = main\n"
                                   "Device = /dev/net/tun\n"
                                   "Port = %1\n").arg(Port); // 根据变量Port创建QString字符串
        conf_file.write(conf_str.toUtf8());  // 将QString转换为QByteArray并写入文件中
        // 设置文件权限为755
        conf_file.setPermissions(QFileDevice::ReadOwner |
                                 QFileDevice::WriteOwner |
                                 QFileDevice::ExeOwner |
                                 QFileDevice::ReadGroup |
                                 QFileDevice::ExeGroup |
                                 QFileDevice::ReadOther |
                                 QFileDevice::ExeOther);
    }
    conf_file.close();

    // 创建内网配置文件tinc-up
    QFile tinc_up(Dir_Path + Name + "/tinc-up");    // 创建文件
    if(tinc_up.open(QFile::WriteOnly | QFile::Append | QIODevice::Text))
    {
        QString up_str = QString("ip link set $INTERFACE up \n"
                                 "ip addr add %1/32 dev $INTERFACE\n"
                                 "ip route add %2.0/24 dev $INTERFACE").arg(IP).arg(Geteway);
        tinc_up.write(up_str.toUtf8());
        // 设置文件权限为755
        tinc_up.setPermissions(QFileDevice::ReadOwner |
                               QFileDevice::WriteOwner |
                               QFileDevice::ExeOwner |
                               QFileDevice::ReadGroup |
                               QFileDevice::ExeGroup |
                               QFileDevice::ReadOther |
                               QFileDevice::ExeOther);
    }
    tinc_up.close();

    // 创建内网配置文件tinc-down
    QFile tinc_down(Dir_Path + Name + "/tinc-down");
    if(tinc_down.open(QFile::WriteOnly | QFile::Append | QIODevice::Text))
    {
        QString down_str = QString("ip route del %1.0/24 dev $INTERFACE\n"
                                   "ip addr del %2/32 dev $INTERFACE\n"
                                   "ip link set $INTERFACE down").arg(Geteway).arg(IP);
        tinc_down.write(down_str.toUtf8());
        // 设置文件权限为755
        tinc_down.setPermissions(QFileDevice::ReadOwner |
                                 QFileDevice::WriteOwner |
                                 QFileDevice::ExeOwner |
                                 QFileDevice::ReadGroup |
                                 QFileDevice::ExeGroup |
                                 QFileDevice::ReadOther |
                                 QFileDevice::ExeOther);
    }
    tinc_down.close();

    // 创建内网配置文件main
    QFile tinc_main(Dir_Path + Name + "/hosts/main");
    if(tinc_main.open(QFile::WriteOnly | QFile::Append | QIODevice::Text))
    {
        QString main_str = QString("#接入及服务集群地址\n"
                                   "Address = %1\n"
                                   "Subnet = 0.0.0.0/0\n"
                                   "Port = %2").arg(Extranet_IP).arg(Port);
        tinc_main.write(main_str.toUtf8());
        // 设置文件权限为755
        tinc_main.setPermissions(QFileDevice::ReadOwner |
                                 QFileDevice::WriteOwner |
                                 QFileDevice::ExeOwner |
                                 QFileDevice::ReadGroup |
                                 QFileDevice::ExeGroup |
                                 QFileDevice::ReadOther |
                                 QFileDevice::ExeOther);
    }
    tinc_main.close();

//    emit Dir_Create_Success();
    Invoke_command("tincd -n " + Name + " -K");       // 内网生成密钥
    Invoke_command("systemctl enable tinc@" + Name + ".service");      // 写入系统自启动
    Invoke_command("service tinc@" + Name + " start");                 // 服务启动

    socket_result = 1;      // 创建内网成功
}

// 内网删除函数
void Daemons::Net_Delete()
{
    // 获取最新的内网数据
    QDir Tinc_dir(Dir_Path);
    Net_List = Tinc_dir.entryList(QDir::Dirs|QDir::Readable|QDir::NoDotAndDotDot,QDir::Name);
    // 检测内网是否存在
    if(!Net_List.contains(Name))
    {
        qDebug() << "需要删除的内网" << Name << "不存在！";
        return;
    }

    qDebug() << "正在删除内网" << Name;
    this -> Invoke_command("systemctl stop tinc@" + Name + ".service");
    this -> Invoke_command("rm -rf " + Dir_Path + Name);
    this -> Invoke_command("rm -f /etc/systemd/system/tinc.service.wants/tinc@" + Name + ".service");
    socket_result = 1;      // 删除内网成功

}

// Linux命令调用函数
void Daemons::Invoke_command(QString command)
{
    qDebug() << command;
    QStringList args = command.split(' ', QString::SkipEmptyParts);
    QString prog = args.takeFirst();
    qDebug() << args;
    qDebug() << prog;

    QProcess process;
    process.start(prog, args);

    if (prog == "tincd" && args.contains("-K")) {
        process.write(QString("\n\n").toUtf8());
    }

    if(!process.waitForStarted())
    {
        qWarning() << "Failed to start process:" << command;
        qWarning() << process.errorString();
        return;
    }


    if(!process.waitForFinished())
    {
        qWarning() << "Timed out waiting for process:" << command;
        qWarning() << process.errorString();
        return;
    }

    QByteArray output = process.readAllStandardError();
    QString stringOutput(output);
    respond_details += stringOutput + "\r\n";         // 保存细节信息
    QStringList listOutput = stringOutput.split("\n", QString::SkipEmptyParts);
    qDebug() << listOutput;
}



// 检测内网函数
void Daemons::Check_Net()
{
    qDebug() << "开始检测当前服务器上的内网！";
    QDir Tinc_dir(Dir_Path);
    // 判断文件夹是否存在
    if(!Tinc_dir.exists())
    {
        qDebug() << "Tinc工作:" << Dir_Path <<"路径不存在！";
        return ;
    }
    // 获取当前服务器所有内网
    Net_List = Tinc_dir.entryList(QDir::Dirs|QDir::Readable|QDir::NoDotAndDotDot,QDir::Name);
    // 获取当前服务器内网数量
    Net_count = Net_List.size();
    qDebug() << "当前服务器内网数量为：" << Net_count;
    qDebug() << "当前服务器的内网有：" << Net_List;

    qDebug() << "开始检测当前服务器上的节点";
    Node_List.clear();   // 清空节点列表
    for(QString net : Net_List)
    {
        QDir netDir(Dir_Path + net + "/hosts/");
        QStringList nodes = netDir.entryList(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot, QDir::Name);
        for(QString node : nodes)
        {
            QString nodeName = net + ":" + node;
            Node_List.append(nodeName);
        }
    }
    Node_count = Node_List.size();
    qDebug() << "当前服务器上的节点数为：" << Node_count;
    qDebug() << "当前服务器上的节点有：" << Node_List;
}

Daemons::~Daemons()
{

}

