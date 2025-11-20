// 主要三个功能
// 接收按键值
// 接收摇杆值在摇杆控制模式
// 调参在调参模式,修改 “自行修改部分的参数即可”
#include "COMMS_task.h"
struct parameter_t
{
  float Kxp;
  float Kxd;
  float Kyp;
  float Kyd;
  float Kop;
  float Kod;
  float Vs;
  float Ds;
  float T1;
  float T2;
  float T3;
  float T4;
} parameter_YPB;

extern zigbee_state_t zigbee_state;
extern uint8_t zigbee_rxBuffer[100];
extern DMA_HandleTypeDef hdma_uart4_rx;
extern uint8_t Control_txData[5]; 
extern uint8_t Control_rxData[5];
extern int8_t x_L;
extern int8_t y_L;
extern int8_t x_R;
extern int8_t y_R;
extern uint8_t Adjust_txData[5]; 
extern uint8_t Adjust_rxData[5];
extern float addition;
// 参数增量计算 ----------------------------------------------------------------
float Calculate(void)
{
  addition = 1.00;
  switch (Adjust_rxData[3])
  {
  case X1: addition *= 1; break;
  case X10: addition *= 10; break;
  case X100: addition *= 100; break;
  case X0_1: addition *= 0.1; break;
  case X0_01: addition *= 0.01; break;
  default: break;
  }
  addition *= Adjust_rxData[2];
  addition *= (Adjust_rxData[4] == 0x01 ? 1:-1);
  return addition;
}
// ZIGBEE的线程 --------------------------------------------------------------
void ZIGBEE(void* argument)
{
  /* USER CODE BEGIN ZIGBEE */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);   
  }
  /* USER CODE END ZIGBEE */
}
// DMA中断回调-------------------------------------------------------------
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  /* Prevent unused argument(s) compilation warning */
  if(huart == zigbee_state.huart)
  {
    // ID
    if(zigbee_rxBuffer[0] == 0xAB && zigbee_rxBuffer[1] == 0xD1 && zigbee_rxBuffer[Size-1] == 0xBA)
    {
      zigbee_state.MyAddr = (zigbee_rxBuffer[2] << 8) | zigbee_rxBuffer[3];
      zigbee_state.ProtocolType = (zigbee_rxBuffer[4] << 8) | zigbee_rxBuffer[5];
    }
    // 全部配置
    if(zigbee_rxBuffer[0] == 0xAB && zigbee_rxBuffer[1] == 0xD2 && zigbee_rxBuffer[Size-1] == 0xBA)
    {
      // 加载配置
      load_Conf();
    }
    // 接收按键值---------------------------------------------------------------------------
    if(zigbee_rxBuffer[0] == 0xAA && zigbee_rxBuffer[1] == 0xD1 
      && ((zigbee_rxBuffer[2]<<8)|zigbee_rxBuffer[3]) == R2_GAMEPAD
      && zigbee_rxBuffer[4] == KEY)
    { 
      if(zigbee_rxBuffer[5] == PC15) // 按下 PC15
      {
        // 要进行的操作;
      }


      
      // 以此类推
    }
    // 摇杆控制模式-------------------------------------------------------------------------
    if(zigbee_rxBuffer[0] == 0xAA && zigbee_rxBuffer[1] == 0xD1 
      && ((zigbee_rxBuffer[2]<<8)|zigbee_rxBuffer[3]) == R2_GAMEPAD
      && zigbee_rxBuffer[4] == CONTROL)
    { // 摇杆值都是 int8_t类型
      x_L = (int8_t)zigbee_rxBuffer[5]; // 左 x -128 ~ 127
      y_L = (int8_t)zigbee_rxBuffer[6]; // 左 y -128 ~ 127
      x_R = (int8_t)zigbee_rxBuffer[7]; // 右 x -128 ~ 127
      y_R = (int8_t)zigbee_rxBuffer[8]; // 右 y -128 ~ 127
      // 根据需要映射摇杆值 


      // 根据需要映射摇杆值 
    }
    // 调参模式-----------------------------------------------------------------------------
    if(zigbee_rxBuffer[0] == 0xAA && zigbee_rxBuffer[1] == 0xD1 
      && ((zigbee_rxBuffer[2]<<8)|zigbee_rxBuffer[3]) == R2_GAMEPAD
      && zigbee_rxBuffer[4] == ADJUST)
    {
      Adjust_rxData[1] = zigbee_rxBuffer[5];
      Adjust_rxData[2] = zigbee_rxBuffer[6];
      Adjust_rxData[3] = zigbee_rxBuffer[7];
      Adjust_rxData[4] = zigbee_rxBuffer[8];
      switch (Adjust_rxData[1])
      { // 请自行修改-----------------------
      // YPB ------------------------------
      case Kxp_: parameter_YPB.Kxp += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.Kxp, 4); break;
      case Kxd_: parameter_YPB.Kxd += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.Kxd, 4); break;
      case Kyp_: parameter_YPB.Kyp += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.Kyp, 4); break;
      case Kyd_: parameter_YPB.Kyd += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.Kyd, 4); break;
      case Kop_: parameter_YPB.Kop += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.Kop, 4); break;
      case Kod_: parameter_YPB.Kod += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.Kod, 4); break;
      case Vs_: parameter_YPB.Vs += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.Vs, 4); break;
      case Ds_: parameter_YPB.Ds += Calculate();memcpy(&Adjust_txData[1], &parameter_YPB.Ds, 4); break;
      case T1_: parameter_YPB.T1 += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.T1, 4); break;
      case T2_: parameter_YPB.T2 += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.T2, 4); break;
      case T3_: parameter_YPB.T3 += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.T3, 4); break;
      case T4_: parameter_YPB.T4 += Calculate(); memcpy(&Adjust_txData[1], &parameter_YPB.T4, 4); break;
      default: break;
      }
      Adjust_txData[0] = ADJUST;
      zigbee_transmit(Adjust_txData, 5, R2_GAMEPAD);
    }
    // 接收KFS位置信息----------------------------------------------------------------------
    // zigbee_rxBuffer[5]~zigbee_rxBuffer[16]依次对应格子 1~12
    // 红区
    if(zigbee_rxBuffer[0] == 0xAA && zigbee_rxBuffer[1] == 0xD1 
      && ((zigbee_rxBuffer[2]<<8)|zigbee_rxBuffer[3]) == R2_GAMEPAD
      && zigbee_rxBuffer[4] == RED_ZONE)
      {

      }
    // 蓝区
    if(zigbee_rxBuffer[0] == 0xAA && zigbee_rxBuffer[1] == 0xD1 
      && ((zigbee_rxBuffer[2]<<8)|zigbee_rxBuffer[3]) == R2_GAMEPAD
      && zigbee_rxBuffer[4] == BLUE_ZONE)
      {

      }
    HAL_UARTEx_ReceiveToIdle_DMA(zigbee_state.huart, zigbee_rxBuffer, 100);
    // 关闭 DMA传输过半中断，防止数据丢失
    __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);
    zigbee_state.rx_completed = 1;
  }
}

