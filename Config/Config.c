#include "Config.h"


float Kf = 1;
float Kxp = 1;


// zigbee--------------------------------------------------------------------
zigbee_msg_t zigbee_msg;
zigbee_state_t zigbee_state = 
{
    .huart = &huart4,
    .rx_completed = 1,
};
struct zigbee_conf_t zigbee_conf;
uint8_t zigbee_txBuffer[100];
uint8_t zigbee_rxBuffer[100];
// 遥控指令：标识、左 x、左 y、右 x、右 y
// 主机接收方zigbee_rxBuffer[4] = 0xCC
uint8_t Control_txData[5]; 
uint8_t Control_rxData[5]; 
int8_t x_L;
int8_t y_L;
int8_t x_R;
int8_t y_R;
// 调参指令：标识、目标(1~5)、基数、倍率、增减
// 主机接收方zigbee_rxBuffer[4] = 0xAA
uint8_t Adjust_txData[5]; 
uint8_t Adjust_rxData[5];
float addition = 1.00;
// DMA通道的指针地址
extern DMA_HandleTypeDef hdma_uart4_rx;

// 初始化函数-------------------------------------------------------
void Config_Init(void)
{
    // NRF初始化
    // Nrf_Init(&nrf_test);

    // zigbee
    // 开启接收中断，由于数据不定长，故使用 DMA中断
	HAL_UARTEx_ReceiveToIdle_DMA(zigbee_state.huart, zigbee_rxBuffer, 100);	
    // 关闭 DMA传输过半中断，防止数据丢失
    __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);
    // 更改地址和工作模式后必须重新上电才有效
    zigbee_init(R2_ROBOT, R2_GAMEPAD, MASTER);

}

