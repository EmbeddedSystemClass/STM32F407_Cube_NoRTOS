#ifndef __CONTACT_H
#define __CONTACT_H

#include "stm32f4xx_hal.h"
#include "main.h"

#include "PID.h"
#include "encoder.h"

#include "math.h"
#include <stdio.h>
#include "cstring"


void LeftMovingSpeedW(unsigned int val);//���ַ�����ٶȿ��ƺ���
void RightMovingSpeedW(unsigned int val2);//���ַ�����ٶȿ��ƺ���

void car_control(float rightspeed,float leftspeed);//С���ٶ�ת���Ϳ��ƺ���

//void Contact_Init(void);//�����ַ�����ٶȳ�ʼ��

#endif  
