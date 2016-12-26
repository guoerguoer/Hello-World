//-------------------------------------------------------------------------------------------------------------
//程序：
//          常规信息类头文件
//历史：
//          2014-12-15      李彦儒     1.0
//修改描述：
//
#ifndef GENERALMESSAGE_H
#define GENERALMESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>               // open
#include <fcntl.h>                  // open
#include <string.h>                 // strncpy

//各种字符串数据的长度
#define PATH_MAX 4096
#define NUM_OF_COCHE 6          //车厢总节数
#define NUM_OF_TRACTION_UNIT 2  //牵引单元数
#define NUM_OF_DOOR 8           //每节车厢门数

//车厢状态信息类
struct coachState{
    //每辆地铁含有6节车厢，每节车厢包含8个车门，一个空调
    char openCloseState[NUM_OF_DOOR];             //一节车厢八个车门的状态，-1：闭锁；0：关闭；1：打开
    //空调温度
    int32_t airConditioningTemp;        //单位为摄氏度
    //空调开关状态
    int32_t airConditioningState;        //0：关闭；1：打开
};

//能量及里程——牵引单元（Traction unit）信息类
struct tractionUnitState{
    //开始日期, eg:2014/12/25
    int32_t startDateLength;
    char startDate[PATH_MAX];
    //累计行车距离，单位km
    int32_t cumulativeDistance;
    //牵引电量，单位kWh
    int32_t tractionPower;
    //再生电量，单位kWh
    int32_t renewablePower;
    //消耗电量，单位kWh
    int32_t consumptionPower;
};

//常规信息类
struct generalMessage {
    //常规信息的总长度
    int32_t msgTotalLength;
    //机车号
    int32_t generalTrainNumLength;
    char generalTrainNum[PATH_MAX];
    //车厢状态，NUM_OF_COCHE节车厢
    struct coachState coachstate[NUM_OF_COCHE];
    int32_t coachstateLength;
    //当前车速，单位km/h
    int32_t currentSpeed;
    //蓄电池电压，单位V（伏）
    int32_t batteryVoltage;
    //轮轴温度，单位摄氏度
    int32_t axleTemperature;
    //能量及里程——牵引单元
    struct tractionUnitState tractionunitState[NUM_OF_TRACTION_UNIT];
    int32_t tractionunitStateLength;
};

//函数名称：generalWriteNetwork
//函数功能：向地面维护中心通过网络发送常规信息
//函数接口：（int lfd，地面维护中心的网络套接字
//                    const struct generalMessage *msg)
//返回值：发送的总字节数， -1为error
int generalWriteNetwork(int lfd, const struct generalMessage msg);

//函数名称：generalReadNetwork
//函数功能：通过网口获取列车上的常规信息
//函数接口：（int lfd, 列车网络套接字
//                   struct generalMessage *msg)
//返回值：实际接收的总字节数， -1为error
int generalReadNetwork(int lfd, struct generalMessage *msg);

//函数名称：generalReadFromFile
//函数功能：从本地文件中读取常规数据，并存放到结构体generalMessage中
//函数接口：（int fd，本地文件描述符
//                  struct generalMessage *msg)
//返回值：实际读取的总字节长度, -1为error
int generalReadFromFile(int fd, struct generalMessage *msg);

//函数名称：generalWriteToFile
//函数功能：向本地文件中写入常规数据，写入的位置是文件的末尾
//函数接口：（int fd，本地文件描述符
//                  const struct generalMessage *msg)
//返回值：实际写入的总字节长度，-1为error
int generalWriteToFile(int fd, const struct generalMessage *msg);

//函数名称：generalReadFromCmd
//函数功能：从命令行中读取常规信息，并保存成generalMessage
//函数接口：char *cmd, struct generalMessage *msg
//返回值：0为成功，-1为error
int generalReadFromCmd(char *cmd, struct generalMessage *msg);

//函数名称：generalMessageLength
//函数功能：计算整个结构体大小
//函数接口：struct &generalMessage *msg
//返回值：void
void generalMessageLength(struct generalMessage *msg);

//函数名称：generalStructToArray
//函数功能：将generalMessage结构体组织成一个字符数组，以便发送到网络
//函数接口：const struct generalMessage *msg, char *buffer);
//返回值：int
int generalStructToArray(const struct generalMessage *msg, char *buffer);

//函数名称：generalArrayToStruct
//函数功能：将字符数组分解为结构体，得到消息数据
//函数接口：const char *buffer, struct generalMessage *msg, int msgLength
//返回值：int
int generalArrayToStruct(const char *buffer, struct generalMessage *msg, int msgLength);

//函数名称：generalPrintMessage
//函数功能：将结构体输出至文件或命令行上
//函数接口：int fd, const struct generalMessage *msg
//返回值：void
void generalPrintMessage(FILE *stream, const struct generalMessage *msg);

#endif // GENERALMESSAGE_H

