//-----------------------------------------------------------------------------
//程序：
//      故障信息类定义头文件
//历史：
//      2014-10-22   李彦儒     1.0
//修改描述：
//      新增faultReadNetWork、faultReadFromFile、faultWriteToFile
#ifndef FAULTMESSAGE_H
#define FAULTMESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>                      // open
#include <fcntl.h>                          // open
#include <string.h>                         // strncpy

//各种字符串数据的长度
#define PATH_MAX 4096
//故障信息类
struct faultMessage {
    //故障信息的总长度
    int32_t msgTotalLength;
    //故障序号
    int32_t faultNumLength;
    char faultNum[PATH_MAX];
    //机车型号
    int32_t faultTrainTypeLength;
    char faultTrainType[PATH_MAX];
    //机车号
    int32_t faultTrainNumLength;
    char faultTrainNum[PATH_MAX];
    //客货类别
    int32_t faultTrackOrBusLength;
    char faultTrackOrBus[PATH_MAX];
    //报活部位
    int32_t faultReportPartLength;
    char faultReportPart[PATH_MAX];
    //报活内容
    int32_t faultReportContentLength;
    char faultReportContent[PATH_MAX];
    //处理方法
    int32_t faultProcessingMethodLength;
    char faultProcessingMethod[PATH_MAX];
    //报活时分
    int32_t faultReportTimeLength;
    char faultReportTime[PATH_MAX];
    //报活人
    int32_t faultReporterLength;
    char faultReporter[PATH_MAX];

};

//函数名称：faultWriteNetwork
//函数功能：向地面维护中心通过网络发送故障信息
//函数接口：（int lfd，地面维护中心的网络套接字
//			   const struct *faultMessage)
//返回值：发送的总字数，-1为error
int faultWriteNetwork(int lfd, const struct faultMessage message);

//函数名称：faultReadNetWork
//函数功能：通过网口获取列车上的故障信息
//函数接口：（int lfd，列车网络套接字
//             struct faultMessage *msg)
//返回值：实际接收到的总字节数，-1为error
int faultReadNetWork(int lfd, struct faultMessage *msg);

//函数名称：faultReadFromFile
//函数功能：从本地文件中读取故障数据，并存放到结构体faultMessage中
//函数接口：（int fd,   //本地文件描述符
//              struct faultMesssage *msg)
//返回值：实际读取的总字节长度
int faultReadFromFile(int fd, struct faultMessage *msg);

//函数名称：faultWriteToFile
//函数功能：向本地文件中写入故障数据，写入的位置是文件的末尾
//函数接口：（int fd，  //本地文件描述符
//                const struct faultMessage *msg)
//返回值：实际写入的总字节长度，-1为失败
int faultWriteToFile(int fd, const struct faultMessage *msg);

//函数名称：faultReadFromCmd
//函数功能：从命令行中读取故障信息，并保存成faultMessage
//           格式：
//          故障序号  机车型号  机车号  客货类别  报活部位  报活内容  处理方法  报活时分  报活人
//          各项之间用空格隔开
//函数接口：char *cmd, struct faultMessage *msg
//返回值：0为成功，-1为error
int faultReadFromCmd(char *cmd, struct faultMessage *msg);

//函数名称：faultMessageLength
//函数功能：计算整个结构体大小
//函数接口：struct &faultMessage
//返回值：void
void faultMessageLength(struct faultMessage *msg);

//函数名称：structToArray
//函数功能：将结构体组织成一个字符数组，以便发送到网络
//函数接口：const struct faultMessage *msg, char *buffer
//返回值：int
int structToArray(const struct faultMessage *msg, char *buffer);

//函数名称：arrayToStruct
//函数功能：将字符数组分解为结构体，得到消息数据
//函数接口：const char *buffer, struct faultMessage *msg, int msgLength
//返回值：int
int arrayToStruct(const char *buffer, struct faultMessage *msg, int msgLength);

//函数名称：printMessage
//函数功能：将结构体输出至文件或命令行上
//函数接口：int fd， const struct faultMessage *msg
//返回值：void
void printMessage(FILE *stream, const struct faultMessage *msg);
#endif // FAULTMESSAGE_H
