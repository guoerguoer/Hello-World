//-------------------------------------------------------------------------------------------------------------
//程序：
//          常规信息类源文件
//历史：
//          2014-12-15      李彦儒     1.0
//修改描述：
//
#include "generalMessage.h"

//函数名称：generalWriteNetwork
//函数功能：向地面维护中心通过网络发送常规信息
//函数接口：（int lfd，地面维护中心的网络套接字
//                    const struct generalMessage *msg)
//返回值：发送的总字节数， -1为error
int generalWriteNetwork(int lfd, const struct generalMessage message)
{
    //发送的总字节数
    int totalSend = 0;
    struct generalMessage msg = message;
    //计算整个结构体的大小
    generalMessageLength(&msg);
    //分配一个动态缓冲区，将msg中的数据都填到其中，之后向网络发送这个缓冲区
    char *messageSend = (char *) malloc(msg.msgTotalLength + 1 + sizeof(int32_t));
    if(generalStructToArray(&msg, messageSend) == -1)
    {
        perror("ERROR: generalStructToArray");
        return -1;
    }

    //将缓冲区内容发送至网络
    if((totalSend = write(lfd, messageSend, msg.msgTotalLength + sizeof(int32_t))) != msg.msgTotalLength + sizeof(int32_t))
    {
        perror("general write network");
        return -1;
    }
    printf("totalSend = %d\n", totalSend);
    //释放分配的缓冲区
    free(messageSend);
    return totalSend;
}

//函数名称：generalReadNetwork
//函数功能：通过网口获取列车上的常规信息
//函数接口：（int lfd, 列车网络套接字
//                   struct generalMessage *msg)
//返回值：实际接收的总字节数， -1为error
int generalReadNetwork(int lfd, struct generalMessage *msg)
{
    //接收的总字节数
    int totalReceive;
    //首先从网络上接收消息数据的总长度
    int32_t totalMsgLength;
    read(lfd, (int32_t *) &totalMsgLength, sizeof(totalMsgLength));
    printf("general message length: %d\n", totalMsgLength);

    //获取常规消息的内容部分
    char *messageReceive = (char *) malloc(totalMsgLength + 1);
    read(lfd, messageReceive, totalMsgLength - sizeof(int32_t));
    if(messageReceive == NULL)
        return -1;
    if(generalArrayToStruct(messageReceive, msg, totalMsgLength - sizeof(int32_t)) == -1)
    {
        perror("generalArrayToStruct");
        return -1;
    }
    free(messageReceive);
    generalMessageLength(msg);
    return totalReceive;
}

//函数名称：generalMessageLength
//函数功能：计算整个结构体大小
//函数接口：struct &generalMessage *msg
//返回值：void
void generalMessageLength(struct generalMessage *msg)
{
    msg->coachstateLength = 0;
    msg->tractionunitStateLength = 0;
    msg->generalTrainNumLength = strlen(msg->generalTrainNum);              //机车号
    //96 + 60 + 24
    msg->coachstateLength = NUM_OF_COCHE * (NUM_OF_DOOR * sizeof(char) +
                                            sizeof(int32_t) +                       // airConditionTemp
                                            sizeof(int32_t));                        // airConditioningState
    int i = 0;
    for(i = 0; i < NUM_OF_TRACTION_UNIT; i++)
    {
        msg->tractionunitState[i].startDateLength = strlen(msg->tractionunitState[i].startDate);    //20
        msg->tractionunitStateLength = msg->tractionunitStateLength +
                                        msg->tractionunitState[i].startDateLength + sizeof(msg->tractionunitState[i].startDateLength) +     //开始日期
                                        sizeof(msg->tractionunitState[i].cumulativeDistance) +                  //累计行车距离
                                        sizeof(msg->tractionunitState[i].tractionPower) +                       //牵引电量
                                        sizeof(msg->tractionunitState[i].renewablePower) +                     //再生电量
                                        sizeof(msg->tractionunitState[i].consumptionPower);                     //消耗电量
    }
    msg->msgTotalLength = sizeof(msg->msgTotalLength) +
    						 sizeof(msg->generalTrainNumLength) + 
    						 msg->generalTrainNumLength + 
                             //sizeof(int32_t) +                            //信息类型码，1为常规信息，0为故障信息
                             sizeof(msg->coachstateLength) +            //车厢状态总长度
                             msg->coachstateLength +
                             sizeof(msg->currentSpeed) +                //当前车速
                             sizeof(msg->batteryVoltage) +               //蓄电池电压
                             sizeof(msg->axleTemperature) +             //轮轴温度
                             sizeof(msg->tractionunitStateLength) +       //能量及里程——牵引单元
                             msg->tractionunitStateLength;
}

//函数名称：generalStructToArray
//函数功能：将generalMessage结构体组织成一个字符数组，以便发送到网络
//函数接口：const struct generalMessage *msg, char *buffer);
//返回值：int
int generalStructToArray(const struct generalMessage *msg, char *buffer)
{
    int tmpLength = 0;
    *((int32_t *)(buffer + tmpLength)) = (int32_t)170;                                               //故障类型码，1为常规信息，0为故障信息，B10101010
    tmpLength += sizeof(int32_t);
    
    *((int32_t *)(buffer + tmpLength)) = msg->msgTotalLength;                         //故障信息的总长度
    tmpLength += sizeof(msg->msgTotalLength);
	
	*((int32_t *)(buffer + tmpLength)) = msg->generalTrainNumLength;                              //机车号
    tmpLength += sizeof(int32_t);
    strncpy(buffer + tmpLength, msg->generalTrainNum, msg->generalTrainNumLength);
    tmpLength += msg->generalTrainNumLength;
    //车厢状态
    int i = 0, j = 0;
    for(i = 0; i < NUM_OF_COCHE; i++)
    {
        for(j = 0; j < NUM_OF_DOOR; j++)
        {
            *(buffer + tmpLength + j) = msg->coachstate[i].openCloseState[j];
        }
        tmpLength += NUM_OF_DOOR * sizeof(char);
        *((int32_t *)(buffer + tmpLength)) = msg->coachstate[i].airConditioningTemp;
        tmpLength += sizeof(msg->coachstate[i].airConditioningTemp);
        *((int32_t *)(buffer + tmpLength)) = msg->coachstate[i].airConditioningState;
        tmpLength += sizeof(msg->coachstate[i].airConditioningState);
    }
    *((int32_t *)(buffer + tmpLength)) = msg->coachstateLength;
    tmpLength += sizeof(msg->coachstateLength);
    //当前车速，单位km/h
    *((int32_t *)(buffer + tmpLength)) = msg->currentSpeed;
    tmpLength += sizeof(msg->currentSpeed);
    //蓄电池电压，单位V（伏）
    *((int32_t *)(buffer + tmpLength)) = msg->batteryVoltage;
    tmpLength += sizeof(msg->batteryVoltage);
    //轮轴温度，单位摄氏度
    *((int32_t *)(buffer + tmpLength)) = msg->axleTemperature;
    tmpLength += sizeof(msg->axleTemperature);
    //能量及里程——牵引单元
    for(i = 0; i < NUM_OF_TRACTION_UNIT; i++)
    {
        *((int32_t *)(buffer + tmpLength)) = msg->tractionunitState[i].startDateLength;
        tmpLength += sizeof(msg->tractionunitState[i].startDateLength);
        strncpy(buffer + tmpLength, msg->tractionunitState[i].startDate, msg->tractionunitState[i].startDateLength);
        tmpLength += msg->tractionunitState[i].startDateLength;
        *((int32_t *)(buffer + tmpLength)) = msg->tractionunitState[i].cumulativeDistance;
        tmpLength += sizeof(msg->tractionunitState[i].cumulativeDistance);
        *((int32_t *)(buffer + tmpLength)) = msg->tractionunitState[i].tractionPower;
        tmpLength += sizeof(msg->tractionunitState[i].tractionPower);
        *((int32_t *)(buffer + tmpLength)) = msg->tractionunitState[i].renewablePower;
        tmpLength += sizeof(msg->tractionunitState[i].renewablePower);
        *((int32_t *)(buffer + tmpLength)) = msg->tractionunitState[i].consumptionPower;
        tmpLength += sizeof(msg->tractionunitState[i].consumptionPower);
    }
    *((int32_t *)(buffer + tmpLength)) = msg->tractionunitStateLength;
    tmpLength += sizeof(msg->tractionunitStateLength);

    if(tmpLength == msg->msgTotalLength + sizeof(int32_t))
        return tmpLength;
    else
        return -1;
}

//函数名称：generalArrayToStruct
//函数功能：将字符数组分解为结构体，得到消息数据
//函数接口：const char *buffer, struct generalMessage *msg, int msgLength
//与generalStructToArray不同的是，本函数舍去了开头的总长度
//返回值：int
int generalArrayToStruct(const char *buffer, struct generalMessage *msg, int msgLength)
{
    int i;
    int tmpLength = 0;
    
    msg->generalTrainNumLength = *(int32_t *)(buffer + tmpLength);                            //机车号
    tmpLength += sizeof(int32_t);
    strncpy(msg->generalTrainNum, buffer + tmpLength, msg->generalTrainNumLength);
    tmpLength += msg->generalTrainNumLength;
    //车厢状态
    for(i = 0; i < NUM_OF_COCHE; i++)
    {
        int j;
        for(j = 0; j < NUM_OF_DOOR; j++)
        {
            msg->coachstate[i].openCloseState[j] = *(buffer + tmpLength + j);
        }
        tmpLength += NUM_OF_DOOR * sizeof(char);
        msg->coachstate[i].airConditioningTemp = *(int32_t *)(buffer + tmpLength);
        tmpLength += sizeof(int32_t);
        msg->coachstate[i].airConditioningState = *(int32_t *)(buffer + tmpLength);
        tmpLength += sizeof(int32_t);
    }
    msg->coachstateLength = *(int32_t *)(buffer + tmpLength);
    tmpLength += sizeof(int32_t);
    //当前车速，单位km/h
    msg->currentSpeed = *(int32_t *)(buffer + tmpLength);
    tmpLength += sizeof(int32_t);
    //蓄电池电压，单位V（伏）
    msg->batteryVoltage = *(int32_t *)(buffer + tmpLength);
    tmpLength += sizeof(int32_t);
    //轮轴温度，单位摄氏度
    msg->axleTemperature = *(int32_t *)(buffer + tmpLength);
    tmpLength += sizeof(int32_t);
    //能量及里程——牵引单元
    for(i = 0; i < NUM_OF_TRACTION_UNIT; i++)
    {
        msg->tractionunitState[i].startDateLength = *(int32_t *)(buffer + tmpLength);
        tmpLength += sizeof(int32_t);
        strncpy(msg->tractionunitState[i].startDate, buffer + tmpLength, msg->tractionunitState[i].startDateLength);
        tmpLength += msg->tractionunitState[i].startDateLength;

        msg->tractionunitState[i].cumulativeDistance = *(int32_t *)(buffer + tmpLength);
        tmpLength += sizeof(int32_t);
        msg->tractionunitState[i].tractionPower = *(int32_t *)(buffer + tmpLength);
        tmpLength += sizeof(int32_t);
        msg->tractionunitState[i].renewablePower = *(int32_t *)(buffer + tmpLength);
        tmpLength += sizeof(int32_t);
        msg->tractionunitState[i].consumptionPower = *(int32_t *)(buffer + tmpLength);
        tmpLength += sizeof(int32_t);
    }
    msg->tractionunitStateLength = *(int32_t *)(buffer + tmpLength);
    tmpLength += sizeof(int32_t);
    generalMessageLength(msg);

    if(msgLength == 0)              //用于从文件中读取msg
        return tmpLength;
    if(tmpLength == msgLength)
        return tmpLength;
    else
        return -1;
}

//函数名称：generalPrintMessage
//函数功能：将结构体输出至文件或命令行上
//函数接口：int fd, const struct generalMessage *msg
//返回值：void
void generalPrintMessage(FILE *stream, const struct generalMessage *msg)
{
    int i, j;
    fprintf(stream, "机车号：%s\n", msg->generalTrainNum);
    fprintf(stream,"车厢状态:\n");
    for(i = 0; i < NUM_OF_COCHE; i++)
    {
        fprintf(stream, "\t\t");
        for(j = 0; j < NUM_OF_DOOR; j++)
        {
            switch(msg->coachstate[i].openCloseState[j])
            {
                case -1:
                    fprintf(stream, "锁 ");
                    break;
                case 0:
                    fprintf(stream, "关 ");
                    break;
                case 1:
                    fprintf(stream, "开 ");
                    break;
            }
        }
        fprintf(stream, "%d ", msg->coachstate[i].airConditioningTemp);
        fprintf(stream, "%d \n", msg->coachstate[i].airConditioningState);
    }
    fprintf(stream, "当前车速:%d\n", msg->currentSpeed);
    fprintf(stream, "蓄电池电压:%d\n", msg->batteryVoltage);
    fprintf(stream, "轮轴温度:%d\n", msg->axleTemperature);
    fprintf(stream, "能量及里程——牵引单元:\n");
    for(i = 0; i < NUM_OF_TRACTION_UNIT; i++)
    {
        fprintf(stream, "\t\t开始日期:%s\n", msg->tractionunitState[i].startDate);
        fprintf(stream, "\t\t累计行车距离:%d\n", msg->tractionunitState[i].cumulativeDistance);
        fprintf(stream, "\t\t牵引电量:%d\n", msg->tractionunitState[i].tractionPower);
        fprintf(stream, "\t\t再生电量:%d\n", msg->tractionunitState[i].renewablePower);
        fprintf(stream, "\t\t消耗电量:%d\n\n", msg->tractionunitState[i].consumptionPower);
    }
}

//函数名称：generalReadFromFile
//函数功能：从本地文件中读取常规数据，并存放到结构体generalMessage中
//函数接口：（int fd，本地文件描述符
//                  struct generalMessage *msg)
//返回值：实际读取的总字节长度, -1为error
int generalReadFromFile(int fd, struct generalMessage *msg)
{
    int totalLength = 0;
    char buffer[PATH_MAX * 3];
    // fdopen的模式必须与fd的模式一致
    FILE *sfd = fdopen(fd, O_RDONLY);
    fgets(buffer, PATH_MAX * 3, sfd);
    generalArrayToStruct(buffer, msg, 0);
}

//函数名称：generalWriteToFile
//函数功能：向本地文件中写入常规数据，写入的位置是文件的末尾
//函数接口：（int fd，本地文件描述符
//                  const struct generalMessage *msg)
//返回值：实际写入的总字节长度，-1为error
int generalWriteToFile(int fd, const struct generalMessage *msg)
{
    char seperator = '\n';
    //设置文件描述符fd的O_APPEND属性，使数据总是追加到文件末尾
    //新文件打开的时候属性应为：O_CREAT | O_EXCL | O_APPEND
    int flags;
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

    char buffer[PATH_MAX * 3];
    int num = generalStructToArray(msg, buffer);            //向文件写入的总长度
    write(fd, buffer + sizeof(int32_t), num - sizeof(int32_t));
    write(fd, "\n", 1);                                         //向文件写入换行符
    fflush(NULL);

    return num + 1;  //换行符
}

//函数名称：generalReadFromCmd
//函数功能：从命令行中读取常规信息，并保存成generalMessage
//函数接口：char *cmd, struct generalMessage *msg
//返回值：0为成功，-1为error
int generalReadFromCmd(char *cmd, struct generalMessage *msg)
{
    return 0;
}
