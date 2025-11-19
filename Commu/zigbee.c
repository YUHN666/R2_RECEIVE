#include "zigbee.h"


//zigbee_state_t zigbee_state = {.huart = &TX_Uart};
//struct zigbee_conf_t zigbee_conf;

extern uint8_t zigbee_txBuffer[100];
extern uint8_t zigbee_rxBuffer[100];
extern zigbee_state_t zigbee_state;

// 通信操作 ------------------------------------------------------------
// 数据发送函数
void zigbee_transmit(uint8_t* Tzigbee_txBuffer, uint8_t lenth, uint16_t target_addr)
{

	// 设置起始标志符
	zigbee_txBuffer[0] = 0xAA; 
	zigbee_txBuffer[lenth+4] = 0x55;	
	zigbee_txBuffer[1] = MASTER_COMMU;		// 数据发送帧
	// 设置发送地址
	zigbee_txBuffer[2] = target_addr >> 8; 
	zigbee_txBuffer[3] = target_addr & 0xFF;
	// 拷贝发送数据
	memcpy(&zigbee_txBuffer[4], Tzigbee_txBuffer, lenth);
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, lenth+5);
}

// 信号强度计算函数
void get_signal_strength(uint16_t target_addr)
{
	zigbee_txBuffer[0] = 0xAA; 
	zigbee_txBuffer[4] = 0x55;
	zigbee_txBuffer[1] = SIG_STRENGTH;		// 信号强度查询帧
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 5);
}

// 
// val: bit0->IO0  bit1->IO1  0=Input  1=Output
void set_IO_conf(uint16_t target_addr, uint8_t val)
{
	uint8_t zigbee_txBuffer[6];
	zigbee_txBuffer[0] = 0xAA; zigbee_txBuffer[5] = 0x55;
	zigbee_txBuffer[1] = SET_IO_CONF;	// 目标IO配置帧
	// 设置发送地址
	zigbee_txBuffer[2] = target_addr >> 8; 
	zigbee_txBuffer[3] = target_addr & 0xFF;
	zigbee_txBuffer[4] = val;
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 6);
}

// val: bit0->IO0  bit1->IO1
void set_IO_state(uint16_t target_addr, uint8_t val)
{
	zigbee_txBuffer[0] = 0xAA; 
	zigbee_txBuffer[5] = 0x55;
	zigbee_txBuffer[1] = SET_IO_STATE;	// 目标IO配置帧
	// 设置发送地址
	zigbee_txBuffer[2] = target_addr >> 8; 
	zigbee_txBuffer[3] = target_addr & 0xFF;
	zigbee_txBuffer[4] = val;
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 6);
}

// 阻塞形式收发
// return: bit0->IO0  bit1->IO1
uint8_t get_IO_state(uint16_t target_addr)
{
	zigbee_txBuffer[0] = 0xAA; 
	zigbee_txBuffer[4] = 0x55;
	zigbee_txBuffer[1] = READ_IO_STATE;	// 目标IO配置帧
	// 设置发送地址
	zigbee_txBuffer[2] = target_addr >> 8; 
	zigbee_txBuffer[3] = target_addr & 0xFF;
	HAL_UART_Transmit(zigbee_state.huart, zigbee_txBuffer, 5, 5);
	uint8_t rec_zigbee_txBuffer[6];
	HAL_UART_Receive(zigbee_state.huart, rec_zigbee_txBuffer, 6, 10);
	if(rec_zigbee_txBuffer[1] == READ_IO_STATE) return rec_zigbee_txBuffer[4];
	else return 0xFF;
}
// 休眠
void module_sleep(uint32_t sleep_ms)
{
	if(sleep_ms > 100000) sleep_ms = 0;	//过大时间直接无限休眠
	zigbee_txBuffer[0] = 0xAA; 
	zigbee_txBuffer[5] = 0x55;
	zigbee_txBuffer[1] = DO_SLEEP;
	zigbee_txBuffer[2] = 0x01;	//只支持睡眠模式1
	// 设置睡眠时间
	uint16_t sleep_10ms = sleep_ms / 10;
	zigbee_txBuffer[3] = sleep_ms >> 8; 
	zigbee_txBuffer[4] = sleep_ms & 0xFF;
	zigbee_msg.is_sleeping = 1;
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 6);
}
// 
uint8_t zigbee_decode(uint8_t* Rzigbee_txBuffer, uint8_t lenth)
{
	if(Rzigbee_txBuffer[0] == 0xAA)
	{
		if(Rzigbee_txBuffer[1] == MASTER_COMMU)	// 接收通讯数据
		{
			uint8_t zigbee_txBufferlen = lenth - 5;
			zigbee_msg.datalen = zigbee_txBufferlen;
			zigbee_state.source_addr = ((uint16_t)Rzigbee_txBuffer[2] << 8) + Rzigbee_txBuffer[3];
			memcpy(zigbee_msg.RxData, &Rzigbee_txBuffer[4], zigbee_txBufferlen);
			return MASTER_COMMU;
		}
		if(Rzigbee_txBuffer[1] == SIG_STRENGTH)	// 连接信号强度
		{
			zigbee_state.source_addr = ((uint16_t)Rzigbee_txBuffer[2] << 8) + Rzigbee_txBuffer[3];
			if(zigbee_state.source_addr == zigbee_state.MyAddr) return 0;		// 解决查询到自己的bug
			zigbee_msg.signal_strength = 0xFF - Rzigbee_txBuffer[4] + 1;	// 取负后单位为dbm
			return SIG_STRENGTH;
		}
		if(Rzigbee_txBuffer[1] == DO_SLEEP)
		{
			zigbee_msg.is_sleeping = 0;
			return DO_SLEEP;
		}
	}
	else if(Rzigbee_txBuffer[0] == 0xAB)
	{
		if(Rzigbee_txBuffer[1] == READ_LOCAL_INFO && Rzigbee_txBuffer[lenth-1] == 0xBA)
		{
			zigbee_msg.info_got = 1;
			// 获取模块ID
			zigbee_state.MyAddr = (Rzigbee_txBuffer[2]<<8) + Rzigbee_txBuffer[3];
		}
		if(Rzigbee_txBuffer[1] == GET_CONF_INFO && Rzigbee_txBuffer[lenth-1] == 0xBA)
		{
			zigbee_msg.config_got = 1;
			// 拷贝结构信息
			memcpy(&zigbee_conf, &Rzigbee_txBuffer[6+Rzigbee_txBuffer[4]], Rzigbee_txBuffer[5]);
		}
	}
	return 0;
}

// 配置操作 ------------------------------------------------------------
// 搜索设备
void search_device(void)
{
	zigbee_txBuffer[0] = 0xAB; // 获取配置协议起始码
	zigbee_txBuffer[1] = SEARCH_DEVICE; 
	zigbee_txBuffer[2] = 0xBA;
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 3);
	HAL_UART_Receive_IT(zigbee_state.huart, zigbee_rxBuffer, 100);
}
// 读取本地信息
void get_localInfo(void)
{
	while(zigbee_state.rx_completed == 0);
	zigbee_txBuffer[0] = 0xAB; // 获取配置协议起始码
	zigbee_txBuffer[1] = READ_LOCAL_INFO; 
	zigbee_txBuffer[2] = 0xBA;
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 3);
	zigbee_state.rx_completed = 0;
}
// 获取配置信息
void get_Conf(void)
{
	
	while(zigbee_state.rx_completed == 0);
	// 帧头帧尾
	zigbee_txBuffer[0] = 0xAB; 
	zigbee_txBuffer[6] = 0xBA;
	// 操作类型
	zigbee_txBuffer[1] = GET_CONF_INFO;
	// 本地设备地址
	zigbee_txBuffer[2] = zigbee_state.MyAddr >> 8; 
	zigbee_txBuffer[3] = zigbee_state.MyAddr & 0xFF;
	// 偏移地址
	zigbee_txBuffer[4] = 0x00;
	// 读取长度
	zigbee_txBuffer[5] = sizeof(zigbee_conf);
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 7);
	zigbee_state.rx_completed = 0;
	
}
// 修改本地ID
void write_MyAddr(uint16_t MyAddr)
{
	if(zigbee_conf._MyAddr_ == MyAddr) return;
	while(zigbee_state.rx_completed == 0);
	// 帧头帧尾
	zigbee_txBuffer[0] = 0xAB; 
	zigbee_txBuffer[8] = 0xBA;
	// 操作类型
	zigbee_txBuffer[1] = WRITE_CONF;
	// 操作地址（手册上写网络号是不对的）
	zigbee_txBuffer[2] = zigbee_conf._MyAddr_ >> 8; 
	zigbee_txBuffer[3] = zigbee_conf._MyAddr_ & 0xFF;
	// 偏移地址
	zigbee_txBuffer[4] = MYADDR;
	// 写入长度
	zigbee_txBuffer[5] = 0x02;
	// 写入数据
	zigbee_txBuffer[6] = MyAddr >> 8; 
	zigbee_txBuffer[7] = MyAddr & 0xFF;
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 9);
	zigbee_state.rx_completed = 0;
}
// 修改目标ID
void write_DstAddr(uint16_t DstAddr)
{
	if(zigbee_conf._DstAddr_ == DstAddr) return;
	while(zigbee_state.rx_completed == 0);
	// 帧头帧尾
	zigbee_txBuffer[0] = 0xAB; 
	zigbee_txBuffer[8] = 0xBA;
	// 操作类型
	zigbee_txBuffer[1] = WRITE_CONF;
	// 操作地址（手册上写网络号是不对的）
	zigbee_txBuffer[2] = zigbee_conf._MyAddr_ >> 8; 
	zigbee_txBuffer[3] = zigbee_conf._MyAddr_ & 0xFF;
	// 偏移地址
	zigbee_txBuffer[4] = DSTADDR;
	// 写入长度
	zigbee_txBuffer[5] = 0x02;
	// 写入数据
	zigbee_txBuffer[6] = DstAddr >> 8; 
	zigbee_txBuffer[7] = DstAddr & 0xFF;
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 9);
	zigbee_state.rx_completed = 0;
}
// 修改发送模式
void write_DevMode(uint8_t sendmode)
{
	if(zigbee_conf.SendMode == sendmode) return;
	while(zigbee_state.rx_completed == 0);
	// 帧头帧尾
	zigbee_txBuffer[0] = 0xAB; 
	zigbee_txBuffer[7] = 0xBA;
	// 操作类型
	zigbee_txBuffer[1] = WRITE_CONF;
	// 操作地址（手册上写网络号是不对的）
	zigbee_txBuffer[2] = zigbee_conf._MyAddr_ >> 8; 
	zigbee_txBuffer[3] = zigbee_conf._MyAddr_ & 0xFF;
	// 偏移地址
	zigbee_txBuffer[4] = DEVMODE;
	// 写入长度
	zigbee_txBuffer[5] = 0x01;
	// 写入数据
	zigbee_txBuffer[6] = sendmode; 
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 8);
	zigbee_state.rx_completed = 0;
}
// 复位
void ResetDevice(uint16_t ID)
{
	while(zigbee_state.rx_completed == 0);
	// 帧头帧尾
	zigbee_txBuffer[0] = 0xAB; 
	zigbee_txBuffer[6] = 0xBA;
	// 操作类型
	zigbee_txBuffer[1] = RESET;
	// 操作ID
	zigbee_txBuffer[2] = ID >> 8; 
	zigbee_txBuffer[3] = ID & 0xFF;
	// 协议类型
	zigbee_txBuffer[4] = zigbee_state.ProtocolType >> 8; 
	zigbee_txBuffer[5] = zigbee_state.ProtocolType & 0xFF;
	HAL_UART_Transmit_DMA(zigbee_state.huart, zigbee_txBuffer, 7);
	zigbee_state.rx_completed = 0;
}
// 加载配置
void load_Conf(void)
{
	// 配置数据
	zigbee_conf.DevMode = zigbee_rxBuffer[38];
	zigbee_conf.Chan = zigbee_rxBuffer[39];
	zigbee_conf._PanID_ = (zigbee_rxBuffer[40] << 8) | zigbee_rxBuffer[41];
	zigbee_conf._MyAddr_ = (zigbee_rxBuffer[42] << 8) | zigbee_rxBuffer[43];
	zigbee_conf._DstAddr_ = (zigbee_rxBuffer[44] << 8) | zigbee_rxBuffer[45];
	zigbee_conf.DataRate = zigbee_rxBuffer[46];
	zigbee_conf.PowerLevel = zigbee_rxBuffer[47];
	zigbee_conf.RetryNum= zigbee_rxBuffer[48];
	zigbee_conf._RetryTimeout_= (zigbee_rxBuffer[49] << 8) | zigbee_rxBuffer[50];
	zigbee_conf.Serial_Rate= zigbee_rxBuffer[51];
	zigbee_conf.Serial_DataB= zigbee_rxBuffer[52];
	zigbee_conf.Serial_StopB= zigbee_rxBuffer[53];
	zigbee_conf.Serial_ParityB= zigbee_rxBuffer[54];
	zigbee_conf.Serial_Timeout= zigbee_rxBuffer[55];
	zigbee_conf.Serial_Byteout= zigbee_rxBuffer[56];
	zigbee_conf.SendMode= zigbee_rxBuffer[57];
	
}
// 初始化
void zigbee_init(uint16_t MyAddr, uint16_t DstAddr, uint8_t sendmode)
{
	// 获取通信模块 ID
	get_localInfo();
	// 获取配置	
	get_Conf();
	// 设置本地ID
	write_MyAddr(MyAddr);
	// 设置目标地址
	write_DstAddr(DstAddr);
	// 设置工作模式
	write_DevMode(sendmode);
	// 获取配置	
	get_Conf();			
}




