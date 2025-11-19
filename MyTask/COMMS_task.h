#ifndef _COMMS_TASK_H_
#define _COMMS_TASK_H_
#include "main.h"
#include "zigbee.h"
#include "cmsis_os.h"
// 模式
#define ADJUST 0xAA
#define CONTROL 0xCC
#define RED_ZONE 0x00
#define BLUE_ZONE 0x01
#define KEY 0x12
// 倍率
#define X1 0x01
#define X10 0x11
#define X100 0x12
#define X0_1 0xF1
#define X0_01 0xF2
// YPB -----------------------------------------------------------
#define Kxp_ 0x01
#define Kxd_ 0x02
#define Kyp_ 0x03
#define Kyd_ 0x04
#define Kop_ 0x05
#define Kod_ 0x06
#define Vs_ 0x07
#define Ds_ 0x08
#define T1_ 0x09
#define T2_ 0x0A
#define T3_ 0x0B
#define T4_ 0x0C
// ZQH -----------------------------------------------------------
#define Kf_ 0x80
#define Vmax_ 0x81
#define Amax_ 0x82
#define Kp1_ 0x83
#define Ki1_ 0x84
#define Kd1_ 0x85
#define Kp2_ 0x86
#define Ki2_ 0x87
#define Kd2_ 0x88
#define Kp3_ 0x89
#define Kd3_ 0x8A
#define Imax1_ 0x8B
#define Imax2_ 0x8C
#define Imax3_ 0x8D
#define Omax_ 0x8E
#define Wmax_ 0x8F
#define Wamax_ 0x90
// 按键值
#define PC15 0x15
#define PC14 0x14
#define PF2 0x02
#define PF0 0x00
#define PF4 0x04
#define PF3 0x03
#define PA11 0x11
#define PA10 0x10
#define PA9 0x09
#define PA8 0x08
#define PG7 0x07
#define PG8 0x08
// 地址
#define R1_GAMEPAD 0x1921
#define R2_GAMEPAD 0x1927
#define R1_ROBOT 0x1945
#define R2_ROBOT 0x1949
// 参数增量计算
float Calculate(void);
#endif