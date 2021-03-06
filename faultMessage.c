//-----------------------------------------------------------------------------
//程序：
//      故障信息类定义源文件
//历史：
//      2014-9-26   李彦儒     1.0
//修改描述：
//
#include "faultMessage.h"

//函数名称：faultWriteNetwork
//函数功能：向地面维护中心通过网络发送故障信息
//函数接口：（int lfd，地面维护中心的网络套接字
//			   const struct *faultMessage)
//返回值：发送的总字数，-1为error
int faultWriteNetwork(int lfd, const struct faultMessage message)
{
    //发送的总字节数
    int totalSend = 0;
    struct faultMessage msg = message;
    //计算整个结构体的大小
    faultMessageLength(&msg);
    //分配一个动态缓存区，将msg中的数据都填到其中,之后向网络发送这个缓冲区
    char *messageSend = (char *) malloc(msg.msgTotalLength + 1 + sizeof(int32_t));
    if(structToArray(&msg, messageSend) == -1)
    {
        perror("ERROR: structToArray");
        return -1;
    }

    //将缓冲区内容发送至网络
    if((totalSend = write(lfd, messageSend, msg.msgTotalLength + sizeof(int32_t))) != msg.msgTotalLength + sizeof(int32_t))
    {
        perror("write network");
        return -1;
    }
    //释放分配的缓冲区
    free(messageSend);
    return totalSend;
}

//函数名称：faultReadNetWork
//函数功能：通过网口获取列车上的故障信息
//函数接口：（int lfd，列车网络套接字
//             struct faultMessage *msg)
//返回值：实际接收到的总字节数, -1为error
int faultReadNetWork(int lfd, struct faultMessage *msg)
{
    //接收的总字节数
    int totalReceive;
    //首先从网络上接收消息数据的总长度
    int32_t totalMsgLength;
    read(lfd, (int32_t *) &totalMsgLength, sizeof(totalMsgLength));
    printf("Total Length: %d\n", totalMsgLength);
    //获取故障消息的内容部分
    char *messageReceive = (char *) malloc(totalMsgLength + 1);
    read(lfd, messageReceive, totalMsgLength - sizeof(int32_t));

    if(messageReceive == NULL)
        return -1;
    arrayToStruct(messageReceive, msg, totalMsgLength - sizeof(int32_t));
    free(messageReceive);
    faultMessageLength(msg);
    return totalReceive;
}

//函数名称：faultMessageLength
//函数功能：计算整个结构体大小
//函数接口：struct &faultMessage
//返回值：void
void faultMessageLength(struct faultMessage *msg)
{
    msg->faultNumLength = strlen(msg->faultNum);                        //故障序号
    msg->faultTrainTypeLength = strlen(msg->faultTrainNum);             //机车型号
    msg->faultTrainNumLength = strlen(msg->faultTrainNum);              //机车号
    msg->faultTrackOrBusLength = strlen(msg->faultTrackOrBus);          //客货类别
    msg->faultReportPartLength = strlen(msg->faultReportPart);           //报活部位
    msg->faultReportContentLength = strlen(msg->faultReportContent);    //报活内容
    msg->faultProcessingMethodLength = strlen(msg->faultProcessingMethod);  //处理方法
    msg->faultReportTimeLength = strlen(msg->faultReportTime);          //报活时分
    msg->faultReporterLength = strlen(msg->faultReporter);               //报活人

    msg->msgTotalLength = sizeof(msg->faultNumLength) + msg->faultNumLength
                            + sizeof(msg->faultTrainNumLength) + msg->faultTrainNumLength
                            + sizeof(msg->faultTrainNumLength) + msg->faultTrainNumLength
                            + sizeof(msg->faultTrackOrBusLength) + msg->faultTrackOrBusLength
                            + sizeof(msg->faultReportPartLength) + msg->faultReportPartLength
                            + sizeof(msg->faultReportContentLength) + msg->faultReportContentLength
                            + sizeof(msg->faultProcessingMethodLength) + msg->faultProcessingMethodLength
                            + sizeof(msg->faultReportTimeLength) + msg->faultReportTimeLength
                            + sizeof(msg->faultReporterLength) + msg->faultReporterLength
                            + sizeof(msg->msgTotalLength);
}

//函数名称：faultReadFromFile
//函数功能：从本地文件中读取故障数据，并存放到结构体faultMessage中
//函数接口：（int fd,   //本地文件描述符
//              struct faultMesssage *msg)
//返回值：实际读取的总字节长度
int faultReadFromFile(int fd, struct faultMessage *msg)
{
    int totalLength = 0;                        //校验
    char separator;                             //分隔符,设为'\n'
    read(fd, (int32_t *)(&msg->msgTotalLength), sizeof(msg->msgTotalLength));     //故障消息的总长度
    totalLength += sizeof(msg->msgTotalLength);
    read(fd, (int32_t *)(&msg->faultNumLength), sizeof(msg->faultNumLength));      //故障序号
    read(fd, msg->faultNum, msg->faultNumLength);
    totalLength += sizeof(msg->faultNumLength) + msg->faultNumLength;
    read(fd, (int32_t *)(&msg->faultTrainTypeLength), sizeof(msg->faultTrainTypeLength));       //机车型号
    read(fd, msg->faultTrainType, msg->faultTrainTypeLength);
    totalLength += sizeof(msg->faultTrainTypeLength) + msg->faultTrainTypeLength;
    read(fd, (int32_t *)(&msg->faultTrainNumLength), sizeof(msg->faultTrainNumLength));         //机车号
    read(fd, msg->faultTrainNum, msg->faultTrainNumLength);
    totalLength += sizeof(msg->faultTrainNumLength) + msg->faultTrainNumLength;
    read(fd, (int32_t *)(&msg->faultTrackOrBusLength), sizeof(msg->faultTrackOrBusLength));     //客货类别
    read(fd, msg->faultTrackOrBus, msg->faultTrackOrBusLength);
    totalLength += sizeof(msg->faultTrackOrBusLength) + msg->faultTrackOrBusLength;
    read(fd, (int32_t *)(&msg->faultReportPartLength), sizeof(msg->faultReportPartLength));      //报活部位
    read(fd, msg->faultReportPart, msg->faultReportPartLength);
    totalLength += sizeof(msg->faultReportPartLength) + msg->faultReportPartLength;
    read(fd, (int32_t *)(&msg->faultReportContentLength), sizeof(msg->faultReportContentLength)); //报活内容
    read(fd, msg->faultReportContent, msg->faultReportContentLength);
    totalLength += sizeof(msg->faultReportContentLength) + msg->faultReportContentLength;
    read(fd, (int32_t *)(&msg->faultProcessingMethodLength), sizeof(msg->faultProcessingMethodLength)); //处理方法
    read(fd, msg->faultProcessingMethod, msg->faultProcessingMethodLength);
    totalLength += sizeof(msg->faultProcessingMethodLength) + msg->faultProcessingMethodLength;
    read(fd, (int32_t *)(&msg->faultReportTimeLength), sizeof(msg->faultReportTimeLength));         //报活时分
    read(fd, msg->faultReportTime, msg->faultReportTimeLength);
    totalLength += sizeof(msg->faultReportTimeLength) + msg->faultReportTimeLength;
    read(fd, (int32_t *)(&msg->faultReporterLength), sizeof(msg->faultReporterLength));                            //报活人
    read(fd, msg->faultReporter, msg->faultReporterLength);
    totalLength += sizeof(msg->faultReporterLength) + msg->faultReporterLength;

    read(fd, (char *) &separator, sizeof(separator));                       //读取分隔符
    if(separator != '\n')
        return -1;
    if(totalLength != msg->msgTotalLength)
        return -1;
    //函数的返回值应该是实际写入的字节数，为了简便，这里先忽略
    return msg->msgTotalLength;
}

//函数名称：faultWriteToFile
//函数功能：向本地文件中写入故障数据，写入的位置是文件的末尾
//函数接口：（int fd，  //本地文件描述符
//                const struct faultMessage *msg)
//返回值：实际写入的总字节长度，-1为失败
int faultWriteToFile(int fd, const struct faultMessage *msg)
{
    char separator = '\n';
    //设置文件描述符fd的O_APPEND属性，使新数据总是追加到文件末尾
    //新文件打开的时候属性应为：O_CREAT | O_EXCL | O_APPEND
    int flags;              //fd的属性字段
    flags = fcntl(fd, F_GETFL);
    if(flags == -1)
    {
        perror("fcntl-F_GETFL");
        return -1;
    }
    flags |= O_APPEND;
    if(fcntl(fd, F_SETFL, flags) == -1)
    {
        perror("fcntl-F_SETFL");
        return -1;
    }

    //同falutReadFromFile，校验先不加了
    write(fd, (int32_t *)(&msg->msgTotalLength), sizeof(msg->msgTotalLength));     //故障消息的总长度
    write(fd, (int32_t *)(&msg->faultNumLength), sizeof(msg->faultNumLength));      //故障序号
    write(fd, msg->faultNum, msg->faultNumLength);
    write(fd, (int32_t *)(&msg->faultTrainTypeLength), sizeof(msg->faultTrainTypeLength));       //机车型号
    write(fd, msg->faultTrainType, msg->faultTrainTypeLength);
    write(fd, (int32_t *)(&msg->faultTrainNumLength), sizeof(msg->faultTrainNumLength));         //机车号
    write(fd, msg->faultTrainNum, msg->faultTrainNumLength);
    write(fd, (int32_t *)(&msg->faultTrackOrBusLength), sizeof(msg->faultTrackOrBusLength));     //客货类别
    write(fd, msg->faultTrackOrBus, msg->faultTrackOrBusLength);
    write(fd, (int32_t *)(&msg->faultReportPartLength), sizeof(msg->faultReportPartLength));      //报活部位
    write(fd, msg->faultReportPart, msg->faultReportPartLength);
    write(fd, (int32_t *)(&msg->faultReportContentLength), sizeof(msg->faultReportContentLength)); //报活内容
    write(fd, msg->faultReportContent, msg->faultReportContentLength);
    write(fd, (int32_t *)(&msg->faultProcessingMethodLength), sizeof(msg->faultProcessingMethodLength)); //处理方法
    write(fd, msg->faultProcessingMethod, msg->faultProcessingMethodLength);
    write(fd, (int32_t *)(&msg->faultReportTimeLength), sizeof(msg->faultReportTimeLength));         //报活时分
    write(fd, msg->faultReportTime, msg->faultReportTimeLength);
    write(fd, (int32_t *)(&msg->faultReporterLength),sizeof(msg->faultReporterLength));                            //报活人
    write(fd, msg->faultReporter, msg->faultReporterLength);
    write(fd, (char *) &separator, sizeof(separator));

    fflush(NULL);
    return msg->msgTotalLength;
}

//函数名称：faultReadFromCmd
//函数功能：从命令行中读取故障信息，并保存成faultMessage
//           格式：
//          故障序号  机车型号  机车号  客货类别  报活部位  报活内容  处理方法  报活时分  报活人
//          各项之间用空格隔开
//函数接口：char *cmd, struct faultMessage *msg
//返回值：0为成功，-1为error
int faultReadFromCmd(char *cmd, struct faultMessage *msg)
{
    const unsigned char split = ' ';
    char *position = NULL;                     //查找到的split位置
    char *stringNew = NULL;                   //剩余要查找的字符串的起始位置
    //清除行末的换行符
    if(cmd[strlen(cmd) -1] == '\n')
        cmd[strlen(cmd) -1] = '\0';
/*
    tmp = strchr(cmd, split);
    while(tmp != NULL)
    {
        tmp1 = tmp + 1;
        printf("%d ", (int)(tmp - cmd));
        tmp = strchr(tmp1, split);
    }
*/
    position = strchr(cmd, split);
    strncpy(msg->faultNum, cmd, (int) (position - cmd));        //故障序号
    stringNew = position + 1;
    position = strchr(stringNew, split);
    strncpy(msg->faultTrainType, stringNew, (int) (position - stringNew));    //机车型号
    stringNew = position + 1;
    position = strchr(stringNew, split);
    strncpy(msg->faultTrainNum, stringNew, (int) (position - stringNew));     // 机车号
    stringNew = position + 1;
    position = strchr(stringNew, split);
    strncpy(msg->faultTrackOrBus, stringNew, (int) (position - stringNew));    // 客货类别
    stringNew = position + 1;
    position = strchr(stringNew, split);
    strncpy(msg->faultReportPart, stringNew, (int) (position - stringNew));     // 报活部位
    stringNew = position + 1;
    position = strchr(stringNew, split);
    strncpy(msg->faultReportContent, stringNew, (int) (position - stringNew));  // 报活内容
    stringNew = position + 1;
    position = strchr(stringNew, split);
    strncpy(msg->faultProcessingMethod, stringNew, (int) (position - stringNew));   //处理方法
    stringNew = position + 1;
    position = strchr(stringNew, split);
    strncpy(msg->faultReportTime, stringNew, (int) (position - stringNew));      // 报活时分
    stringNew = position + 1;
    position = strchr(stringNew, split);      //截取字符串最后一项内容时，按理说找不到split了，找到了反而出错
    if(position != NULL) {
        perror("bad command");
        return -1;
    }
    strcpy(msg->faultReporter, stringNew);                                          // 报活人
    faultMessageLength(msg);
    return 0;
}
//函数名称：structToArray
//函数功能：将结构体组织成一个字符数组，以便发送到网络
//函数接口：const struct *faultMessage, char *buffer
//返回值：void
int structToArray(const struct faultMessage *msg, char *buffer)
{
    int tmpLength = 0;
    *((int32_t *)buffer) = (int32_t)85;                        //故障类型码，1为常规信息，0为故障信息，B01010101
    tmpLength += sizeof(int32_t);
    *((int *)(buffer + tmpLength)) = msg->msgTotalLength;                                                 //故障信息的总长度
    tmpLength += sizeof(int32_t);
    *((int *)(buffer + tmpLength)) = msg->faultNumLength;                                   //故障序号
    tmpLength += sizeof(int32_t);
    strncpy(buffer + tmpLength, msg->faultNum, msg->faultNumLength);
    tmpLength += msg->faultNumLength;
    *((int *)(buffer + tmpLength)) = msg->faultTrainTypeLength;                             //机车型号
    tmpLength += sizeof(int32_t);
    strncpy(buffer + tmpLength, msg->faultTrainType, msg->faultTrainTypeLength);
    tmpLength += msg->faultTrainTypeLength;
    *((int *)(buffer + tmpLength)) = msg->faultTrainNumLength;                              //机车号
    tmpLength += sizeof(int32_t);
    strncpy(buffer + tmpLength, msg->faultTrainNum, msg->faultTrainNumLength);
    tmpLength += msg->faultTrainNumLength;
    *((int *)(buffer + tmpLength)) = msg->faultTrackOrBusLength;                            //客货类别
    tmpLength += sizeof(int32_t);
    strncpy(buffer + tmpLength, msg->faultTrackOrBus, msg->faultTrackOrBusLength);
    tmpLength += msg->faultTrackOrBusLength;
    *((int *)(buffer + tmpLength)) = msg->faultReportPartLength;                             //报活部位
    tmpLength += sizeof(int32_t);
    strncpy(buffer + tmpLength, msg->faultReportPart, msg->faultReportPartLength);
    tmpLength += msg->faultReportPartLength;
    *((int *)(buffer + tmpLength)) = msg->faultReportContentLength;                         //报活内容
    tmpLength += sizeof(int32_t);
    strncpy(buffer + tmpLength, msg->faultReportContent, msg->faultReportContentLength);
    tmpLength += msg->faultReportContentLength;
    *((int *)(buffer + tmpLength)) = msg->faultProcessingMethodLength;                      //处理方法
    tmpLength += sizeof(int32_t);
    strncpy(buffer + tmpLength, msg->faultProcessingMethod, msg->faultProcessingMethodLength);
    tmpLength += msg->faultProcessingMethodLength;
    *((int *)(buffer + tmpLength)) = msg->faultReportTimeLength;                            //报活时分
    tmpLength += sizeof(int32_t);
    strncpy(buffer + tmpLength, msg->faultReportTime, msg->faultReportTimeLength);
    tmpLength += msg->faultReportTimeLength;
    *((int *)(buffer + tmpLength)) = msg->faultReporterLength;                              //报活人
    tmpLength += sizeof(int32_t);
    strncpy(buffer + tmpLength, msg->faultReporter, msg->faultReporterLength);
    tmpLength += msg->faultReporterLength;

    if(tmpLength == msg->msgTotalLength + sizeof(int32_t))
        return 0;
    else
        return -1;
}

//函数名称：arrayToStruct
//函数功能：将字符数组分解为结构体，得到消息数据
//函数接口：const char *buffer, struct faultMessage *msg, int msgLength
//返回值：int
int arrayToStruct(const char *buffer, struct faultMessage *msg, int msgLength)
{
    int tmpLength = 0;
    //msg->msgTotalLength = *(int32_t *)(buffer + tmpLength);                                   //故障信息的总长度
    msg->faultNumLength = *(int32_t *)(buffer + tmpLength);                                 //故障序号
    tmpLength += sizeof(int32_t);
    strncpy(msg->faultNum, buffer + tmpLength, msg->faultNumLength);
    tmpLength += msg->faultNumLength;
    msg->faultTrainTypeLength = *(int32_t *)(buffer + tmpLength);                           //机车型号
    tmpLength += sizeof(int32_t);
    strncpy(msg->faultTrainType, buffer + tmpLength, msg->faultTrainTypeLength);
    tmpLength += msg->faultTrainTypeLength;
    msg->faultTrainNumLength = *(int32_t *)(buffer + tmpLength);                            //机车号
    tmpLength += sizeof(int32_t);
    strncpy(msg->faultTrainNum, buffer + tmpLength, msg->faultTrainNumLength);
    tmpLength += msg->faultTrainNumLength;
    msg->faultTrackOrBusLength = *(int32_t *)(buffer + tmpLength);                          //客货类别
    tmpLength += sizeof(int32_t);
    strncpy(msg->faultTrackOrBus, buffer + tmpLength, msg->faultTrackOrBusLength);
    tmpLength += msg->faultTrackOrBusLength;
    msg->faultReportPartLength = *(int32_t *)(buffer + tmpLength);                           //报活部位
    tmpLength += sizeof(int32_t);
    strncpy(msg->faultReportPart, buffer + tmpLength, msg->faultReportPartLength);
    tmpLength += msg->faultReportPartLength;
    msg->faultReportContentLength = *(int32_t *)(buffer + tmpLength);                       //报活内容
    tmpLength += sizeof(int32_t);
    strncpy(msg->faultReportContent, buffer + tmpLength, msg->faultReportContentLength);
    tmpLength += msg->faultReportContentLength;
    msg->faultProcessingMethodLength = *(int32_t *)(buffer + tmpLength);                    //处理方法
    tmpLength += sizeof(int32_t);
    strncpy(msg->faultProcessingMethod, buffer + tmpLength, msg->faultProcessingMethodLength);
    tmpLength += msg->faultProcessingMethodLength;
    msg->faultReportTimeLength = *(int32_t *)(buffer + tmpLength);                          //报活时分
    tmpLength += sizeof(int32_t);
    strncpy(msg->faultReportTime, buffer + tmpLength, msg->faultReportTimeLength);
    tmpLength += msg->faultReportTimeLength;
    msg->faultReporterLength = *(int32_t *)(buffer + tmpLength);                             //报活人
    tmpLength += sizeof(int32_t);
    strncpy(msg->faultReporter, buffer + tmpLength, msg->faultReporterLength);
    tmpLength += msg->faultReporterLength;

    if(tmpLength == msgLength)
        return tmpLength;
    else
        return -1;
}

//函数名称：printMessage
//函数功能：将结构体输出至文件或命令行上
//函数接口：int fd， const struct faultMessage *msg
//返回值：void
void printMessage(FILE *stream, const struct faultMessage *msg)
{
    fprintf(stream, "故障信息总长度：%d\n", msg->msgTotalLength);
    fprintf(stream, "故障序号：%s\n", msg->faultNum);
    fprintf(stream, "机车型号：%s\n", msg->faultTrainType);
    fprintf(stream, "机车号：%s\n", msg->faultTrainNum);
    fprintf(stream, "客货类别：%s\n", msg->faultTrackOrBus);
    fprintf(stream, "报活部位：%s\n", msg->faultReportPart);
    fprintf(stream, "报活内容：%s\n", msg->faultReportContent);
    fprintf(stream, "处理方法：%s\n", msg->faultProcessingMethod);
    fprintf(stream, "报活时分：%s\n", msg->faultReportTime);
    fprintf(stream, "报活人：%s\n", msg->faultReporter);
    fprintf(stream, "\n");
    fflush(NULL);
}

/*
int main(int argc, char**argv)
{
    struct faultMessage msg = {0, 1, 2, 3, 4, "liyanru", 5, "liyanru", 6, "liyanru", 7, "liyanru"};
    faultMessageLength(&msg);
    printf("The length of faultMessage msg = %d\n", msg.faultTotalLength);
    faultWriteNetwork(0, msg);
}
*/
