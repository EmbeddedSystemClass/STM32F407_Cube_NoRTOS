
#include "car.h"
#include "usart.h"
#include "contact.h"
#include "odometry.h"
#include "tim.h"

#define CommunicateSerialPort   huart3
/***********************************************  ˵��  *****************************************************************
*
*   1.���ڽ���
*    ��1�����ݣ�С���������ٶ�,��λ:mm/s���������ݶ�Ϊfloat�ͣ�float��ռ4�ֽڣ�
*    ��2����ʽ��10�ֽ� [�����ٶ�4�ֽ�][�����ٶ�4�ֽ�][������"\r\n"2�ֽ�]
*
*   2.���ڷ���
*    ��1�����ݣ���̼ƣ�x,y���ꡢ���ٶȡ����ٶȺͷ���ǣ���λ����Ϊ��mm,mm,mm/s,rad/s,rad���������ݶ�Ϊfloat�ͣ�float��ռ4�ֽڣ�
*    ��2����ʽ��21�ֽ� [x����4�ֽ�][y����4�ֽ�][�����4�ֽ�][���ٶ�4�ֽ�][���ٶ�4�ֽ�][������"\n"1�ֽ�]
*
************************************************************************************************************************/
/***********************************************  ���  *****************************************************************/

char odometry_data[21]={0};   //���͸����ڵ���̼���������

float odometry_right=0,odometry_left=0;//���ڵõ����������ٶ�

/***********************************************  ����  *****************************************************************/

extern float position_x,position_y,oriention,velocity_linear,velocity_angular;         //����õ�����̼���ֵ

extern uint8_t USART_RX_BUF[USART_REC_LEN];     //���ڽ��ջ���,���USART_REC_LEN���ֽ�.
extern uint16_t USART_RX_STA;                   //���ڽ���״̬���	

extern float Milemeter_L_Motor,Milemeter_R_Motor;     //dtʱ���ڵ��������ٶ�,������̼Ƽ���

/***********************************************  ����  *****************************************************************/

uint8_t main_sta=0; //������������������if��ȥ�������flag��1��ӡ��̼ƣ���2���ü�����̼����ݺ�������3���ڽ��ճɹ�����4���ڽ���ʧ�ܣ�

union recieveData  //���յ�������
{
	float d;    //�������ٶ�
	unsigned char data[4];
}leftdata,rightdata;       //���յ�����������

union odometry  //��̼����ݹ�����
{
	float odoemtry_float;
	unsigned char odometry_char[4];
}x_data,y_data,theta_data,vel_linear,vel_angular;     //Ҫ��������̼����ݣ��ֱ�Ϊ��X��Y�����ƶ��ľ��룬��ǰ�Ƕȣ����ٶȣ����ٶ�

/****************************************************************************************************************/	

extern int32_t hSpeed_Buffer1[],hSpeed_Buffer2[];//�������ٶȻ�������
extern uint8_t main_sta;//����������ִ�б�־λ

//extern u8 bSpeed_Buffer_Index;
uint8_t bSpeed_Buffer_Index = 0;//���������ֱ��������������

//extern float Milemeter_L_Motor,Milemeter_R_Motor;      //�ۼƵ��һ�����е���� cm		
float  Milemeter_L_Motor=0,Milemeter_R_Motor=0;//dtʱ���ڵ��������ٶ�,������̼Ƽ���

uint8_t USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
uint16_t USART_RX_STA=0;   //����״̬���	
uint8_t serial_rec=0x31;   //���մ������ݱ���

extern float pulse;//���A PID���ں��PWMֵ����
extern float pulse1;//���B PID���ں��PWMֵ����


/*Printf��ӳ��*/
int fputc(int ch, FILE *f)
{
   CommunicateSerialPort.Instance ->DR=(uint8_t)ch;
   while((CommunicateSerialPort.Instance->SR&0X40)==0);
   return ch;
}
int GetKey(void) 
{ 
   while (!(CommunicateSerialPort.Instance->SR & 0x20));
   return ((int)(CommunicateSerialPort.Instance->DR & 0x1FF));
}


/*���ڽ��ջص�����*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
{
	if( huart == &CommunicateSerialPort) 
	{ 
		if((USART_RX_STA&0x8000)==0)//����δ���
        {
            if(USART_RX_STA&0x4000)//���յ���0x0d
            {
                if(serial_rec==0x0a)
                {
                    if((USART_RX_STA&0x3f)==8)
                    {							
                        USART_RX_STA|=0x8000;	//��������� 
                        main_sta|=0x04;
                        main_sta&=0xF7;
											HAL_GPIO_TogglePin(U1_RX_LED_GPIO_Port,U1_RX_LED_Pin);
                    }
                    else
                    {
                        main_sta|=0x08;
                        main_sta&=0xFB;
                        USART_RX_STA=0;//���մ���,���¿�ʼ
                    }
                }
                else 
                {
                    main_sta|=0x08;
                    USART_RX_STA=0;//���մ���,���¿�ʼ
                }
            }
            else //��û�յ�0X0D
            {	
                if(serial_rec==0x0d)
								{
									USART_RX_STA|=0x4000;
									HAL_GPIO_TogglePin(U1_RX_LED_GPIO_Port,U1_RX_LED_Pin);
								}
                else
                {
                    USART_RX_BUF[USART_RX_STA&0X3FFF]=serial_rec ;
                    USART_RX_STA++;
                    if(USART_RX_STA>(USART_REC_LEN-1))
                    {
                        main_sta|=0x08;
                        USART_RX_STA=0;//�������ݴ���,���¿�ʼ����
                    }							
                }		 
            }
        } 
	} 
	HAL_UART_Receive_IT(huart, (uint8_t *)&serial_rec, 1) ; /*���������նˣ�������һ������*/
} 

/*��ʱ���ص�����*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		if (htim->Instance == htim6.Instance)
		{
				if (hSpeedMeas_Timebase_500us !=0)//����������ɼ�ʱ����δ��
				{
						hSpeedMeas_Timebase_500us--;//��ʼ����	
				}
				else    //����������ɼ�ʱ��������
				{
						int32_t wtemp2,wtemp1;
						
						hSpeedMeas_Timebase_500us = SPEED_SAMPLING_TIME;//�ָ�����������ɼ�ʱ����
						
						/************************ 1 ***************************/
						
						wtemp2 = ENC_Calc_Rot_Speed2(); //A ��ȡ�ı�����
						wtemp1 = ENC_Calc_Rot_Speed1(); //B ��ȡ�ı�����
						
	//            //���Ϊָֹͣ����������ٶ�Ϊ�㣬������ٶȴ洢����ֹǰ���ٶȲ�̫�����С����ת
	//            if((wtemp2 == 0) && (wtemp1 == 0))
	//            {
	//                pulse=pulse1=0;
	//            }
						 
						/************************ 2 ***************************/
						
						//���������������������������̼Ƽ���
						Milemeter_L_Motor= (float)wtemp1; //����������
						Milemeter_R_Motor= (float)wtemp2;
						
						main_sta|=0x02;//ִ�м�����̼����ݲ���

						/************************ 3 ***************************/
						
						//��ʼ���������ֱ�����������
						hSpeed_Buffer2[bSpeed_Buffer_Index] = wtemp2;
						hSpeed_Buffer1[bSpeed_Buffer_Index] = wtemp1;
						bSpeed_Buffer_Index++;//������λ
						
						//���������ֱ���������������ж�
						if(bSpeed_Buffer_Index >=SPEED_BUFFER_SIZE)
						{
								bSpeed_Buffer_Index=0;//���������ֱ������������������
						}
						
						/************************ 4 ***************************/
						
						ENC_Calc_Average_Speed();//�������ε����ƽ��������
						Gain2(); //���Aת��PID���ڿ��� ��
						Gain1(); //���Bת��PID���ڿ��� ��
				}
		}
		
		if (htim->Instance == htim7.Instance)
		{
				HAL_GPIO_TogglePin(RUN_LED_GPIO_Port,RUN_LED_Pin);
		}
}

/**/

void CarMainFunction(void)
{
	uint8_t t=0;
	uint8_t i=0,j=0,m=0;
	uint8_t SendZero=0x00;
	uint8_t Send_n='\n';
		/*ִ�з�����̼����ݲ���*/
		if(main_sta&0x01)
		{
            //��̼����ݻ�ȡ
			x_data.odoemtry_float=position_x;//��λmm
			y_data.odoemtry_float=position_y;//��λmm
			theta_data.odoemtry_float=oriention;//��λrad
			vel_linear.odoemtry_float=velocity_linear;//��λmm/s
			vel_angular.odoemtry_float=velocity_angular;//��λrad/s
            
            //��������̼����ݴ浽Ҫ���͵�����
			for(j=0;j<4;j++)
			{
				odometry_data[j]=x_data.odometry_char[j];
				odometry_data[j+4]=y_data.odometry_char[j];
				odometry_data[j+8]=theta_data.odometry_char[j];
				odometry_data[j+12]=vel_linear.odometry_char[j];
				odometry_data[j+16]=vel_angular.odometry_char[j];
			}
            
			odometry_data[20]='\n';//��ӽ�����
            
				//��������Ҫ����
				if(HAL_UART_Transmit_IT(&CommunicateSerialPort,(uint8_t*)odometry_data,21)!=HAL_OK)
					Error_Handler(); //�ڷ��͵�һ������ǰ�Ӵ˾䣬�����һ�����ݲ����������͵�����				
            
			main_sta&=0xFE;//ִ�м�����̼����ݲ���
		}
		
		
		/*ִ�м�����̼����ݲ���*/
		if(main_sta&0x02)
		{
			odometry(Milemeter_R_Motor,Milemeter_L_Motor);//������̼�
        
			main_sta&=0xFD;//ִ�з�����̼����ݲ���
		} 
		
		/*������ָ��û����ȷ����ʱ*/
		if(main_sta&0x08)
		{
					  //�ڷ��͵�һ������ǰ�Ӵ˾䣬�����һ�����ݲ����������͵�����
            for(m=0;m<3;m++)
            {
							if(HAL_UART_Transmit_IT(&CommunicateSerialPort,&SendZero,1)!=HAL_OK)
								Error_Handler(); //�ڷ��͵�һ������ǰ�Ӵ˾䣬�����һ�����ݲ����������͵�����
            }		
						
						if(HAL_UART_Transmit_IT(&CommunicateSerialPort,&Send_n,1)!=HAL_OK)
								Error_Handler();
            main_sta&=0xF7;
		}
		
		/*����3���պ���*/
		if(USART_RX_STA&0x8000)
		{			
            //�����������ٶ�
            for(t=0;t<4;t++)
            {
                rightdata.data[t]=USART_RX_BUF[t];
                leftdata.data[t]=USART_RX_BUF[t+4];
            }
            
            //�����������ٶ�
            odometry_right=rightdata.d;//��λmm/s
            odometry_left=leftdata.d;//��λmm/s
            
			USART_RX_STA=0;//������ձ�־λ
		}
       
    car_control(rightdata.d,leftdata.d);	 //�����յ����������ٶȸ���С��	
}


