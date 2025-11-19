#pragma once
#include "main.h"
#include <stdio.h>
#include <string.h>

// 命令标识符
#define GET_STATE		0xD0 // 获取响应状态
#define MASTER_COMMU	0xD1 // 主机数据通讯
#define TEMP_RATE		0xD2 // 设置临时通道速率
#define SIG_STRENGTH	0xD3 // 查询信号强度
#define SET_IO_CONF		0xD4 // 设置 IO输入输出
#define SET_IO_STATE	0xD5 // 设置 IO状态
#define READ_IO_STATE	0xD6 // 读取 IO状态
#define READ_AD			0xD7 // 读取 AD
#define DO_SLEEP		0xD8 // 进入睡眠

// 响应状态
#define COMMAND_OK			0x00    
#define ADDRESS_FAUSE		0x01
#define LENGTH_FAUSE		0x02
#define CHECK_FAUSE			0x03
#define WRITE_FAUSE			0x04
#define OTHER_FAUSE			0x05
#define OTHER_ERR			0x06
#define CHAN_ERR			0x07
#define RATE_ERR			0x08
#define ID_ERR				0x09
#define WORKMODE_ERR		0x0A
#define PARAMETER_ERR		0x0B
#define SEND_DATA_TIMEOUT	0xA4
#define DEVICE_BUSY			0xA5

// 命令码
#define READ_LOCAL_INFO	0xD1 // 读取本地信息
#define GET_CONF_INFO	0xD2 // 获取配置信息
#define WRITE_CONF		0xD3 // 修改配置
#define SET_CHAN_RATE	0xD4 // 设置通道速率
#define SEARCH_DEVICE	0xD5 // 搜索设备
#define PING_ID			0xD6 // 指定 ID在线检测命令
#define RESET			0xD7 // 复位
#define RESTORE			0xDB // 恢复出厂设置

// 偏移地址
#define MYADDR 0x24	 // 本地地址 2byte
#define DSTADDR 0x26 // 目标地址 2byte
#define DEVMODE 0x20 // 工作模式 1byte

// 工作模式
#define MASTER 0x00		  // 主机模式
#define TERMINAL 0x01	  // 终端模式
#define MASTER_MID 0x02	  // 主机中继模式
#define TERMINAL_MID 0x03 // 终端中继模式
// zigbee信息结构体
typedef struct
{
	uint8_t RxData[100];
	uint8_t datalen;		 // 数据长度
	uint8_t signal_strength; // 信号强度
	uint8_t is_sleeping;	 // 是否休眠
	uint8_t info_got;		 // 信息接收
	uint8_t config_got;		 //
}zigbee_msg_t;
extern zigbee_msg_t zigbee_msg;



// zigbee状态结构体
typedef struct
{
	UART_HandleTypeDef *huart;
	uint8_t DevMode;	   // 设备模式
	uint8_t Serial_Rate;   // 串口波特率
	uint16_t PanID;		   // 局域网络标识符
	uint16_t MyAddr;	   // 当前网络地址
	uint16_t DstAddr;	   // 目标网络地址
	uint16_t ProtocolType; // 协议类型
	uint16_t source_addr;  // 源地址
	uint8_t Chan;
	uint8_t DataRate;
	uint8_t RetryNum; // 重发次数
	uint16_t RetryTimeout;
	uint8_t PowerLevel;
	// 接收中断标志位，避免时序混乱
	uint8_t tx_completed;
	uint8_t rx_completed;
} zigbee_state_t;

// 读取到的u16是大端在前，然而stm32储存是小端在前，所以结构体中的u16数据都是反的
// zigbee配置
struct zigbee_conf_t
{
	char DevName[16]; // 设备名称
	char DevPwd[16]; // 设备密码
	uint8_t DevMode; // 工作模式（0主机模式；1终端模式；2主机+中继模式；3终端+中继模式）
	uint8_t Chan; // 工作频率
	uint16_t _PanID_; // 网络号
	uint16_t _MyAddr_; // 本地ID
	uint16_t _DstAddr_; // 目标ID
	uint8_t DataRate; // RF通讯速率
	uint8_t PowerLevel; // 发射功率
	uint8_t RetryNum; // 重试次数
	uint16_t _RetryTimeout_; // 重试时间间隔（ ms）
	uint8_t Serial_Rate; // 串口波特率
	// 串口波特率值为 0～9，分别对应波特率： 
	// 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800
	uint8_t Serial_DataB; // 串口数据位
	uint8_t Serial_StopB; // 串口停止位
	uint8_t Serial_ParityB; // 串口校验位
	uint8_t Serial_Timeout; // 串口帧间隔
	uint8_t Serial_Byteout; // 串口分帧长度
	uint8_t SendMode; // 发送模式（0：单播模式； 1：广播模式）
}
//告诉编译器取消结构体的内存对齐，让结构体成员按照定义顺序紧密排列，不留任何填充字节
__attribute__((packed)); 

extern struct zigbee_conf_t zigbee_conf;

/* 通信操作 */
void zigbee_transmit(uint8_t* Tdata, uint8_t lenth, uint16_t target_addr);
void get_signal_strength(uint16_t target_addr);
// val: bit0->IO0  bit1->IO1  0=Input  1=Output
void set_IO_conf(uint16_t target_addr, uint8_t val);
// val: bit0->IO0  bit1->IO1
void set_IO_state(uint16_t target_addr, uint8_t val);
// 阻塞形式收发
// return: bit0->IO0  bit1->IO1
uint8_t get_IO_state(uint16_t target_addr);
void module_sleep(uint32_t sleep_ms);
uint8_t zigbee_decode(uint8_t* Rdata, uint8_t lenth);

/* 配置操作 */
void search_device(void);
void get_localInfo(void);
void get_Conf(void);
void write_Conf(void);
void write_MyAddr(uint16_t MyAddr);
// 修改目标ID
void write_DstAddr(uint16_t MyAddr);
// 修改发送模式
void write_DevMode(uint8_t sendmode);
void load_Conf(void);
// 读取并写入配置，写入后重启模块生效；并且只有在460800及以下波特率才能够写入成功
void zigbee_init(uint16_t MyAddr, uint16_t DstAddr, uint8_t sendmode);
