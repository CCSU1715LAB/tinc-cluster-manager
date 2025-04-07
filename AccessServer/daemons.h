#ifndef DAEMONS_H
#define DAEMONS_H

#include <QTimer>
#include <QDataStream>
#include <QObject>
#include <QProcess>
#include <QEventLoop>
#include <QTcpSocket>
#include <QTcpServer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHostInfo>
#include <QTextCodec>
#include <unistd.h>
#include <QRegularExpression>


class Daemons : public QObject
{
    Q_OBJECT
public:
    explicit Daemons(QObject *parent = nullptr);
    ~Daemons();

    QString GetIp();            // 获取外网IP


    void Invoke_command(QString command);      // 调用Linux 命令



protected:
    int Port;                   // 用于保存内网端口
    int content_length;         // 用于保存传输信息长度
    int Net_count;              // 用于保存内网数量
    int Node_count;             // 用于保存节点数量
    int socket_result;          // 用于保存守护进程处理结果
    QJsonObject Json;           // 用于接收json数据
    QByteArray data;            // 用于保存接收到的信息
    QString requestInfo;        // 用于保存原始的请求信息
    QString respond_details;    // 用于保存返回的细节信息

    QString Object;             // 用于保存协议中的操作对象
    QString Operation;          // 用于保存协议中的操作
    QString Name;               // 用于保存协议中的对象名
    QString IP;                 // 用于保存协议中对象的IP地址
    QString Geteway;            // 用于保存协议中对象的网关
    QString Internet;           // 用于保存协议中节点的所属内网

    QString Dir_Path;           // 用于保存Tinc根目录
    QString Extranet_IP;        // 用于保存云服务器外网IP
    QStringList Net_List;       // 用于保存当前服务器下所有内网
    QStringList Node_List;      // 用于保存当前服务器下某个内网的节点
    QString content;            // 用于保存传输信息

signals:
    void Analysis_Success();    // 数据分析成功信号
    void Receive_Success();     // 数据接收成功信号
    void Net_Create_Signal();   // 创建内网信号
    void Node_Create_Signal();  // 创建节点信号
    void Node_Delete_Signal();  // 节点删除信号
    void Net_Delete_Signal();   // 内网删除信号

protected slots:
    void newConnectionSlot();   // 新连接建立时，调用的槽函数
    void errorStringSlot();     // 信息接收发生错误时，错误处理槽函数
    void Analysis_Msg();        // 用于分析数据是否合法的槽函数
    void Send_respond_Info();   // 发送响应信息槽函数
    void Data_Processing();     // 用于处理数据槽函数
    void Node_Create();         // 用于接收节点文件
    void Node_Delete();         // 用于节点的删除
    void Net_Create();          // 用于创建内网
    void Net_Delete();          // 用于内网删除
    void Check_Net();           // 检测内网

private:
    QTcpServer *m_tcpServer;    // TCP服务指针
    QTcpSocket *m_tcpSocket;    // TCP套接字指针
    QTimer *Timer;               // 定时器
};

#endif // DAEMONS_H
