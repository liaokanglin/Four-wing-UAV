/* �����򾭹�������ȫ����, ���ṩ�绰����֧��, �粻������, �����в�����ػ���.  */

/***  �ر�ע��: ����ʱѡ���ڲ�ʱ��24MHZ, �����û�EEPROM��СΪ2K������.  ****/

/*********************************************
  ����ɿ�-V10.C

ʹ��ң�ؽ������ͺ�: MC6B��ͨ��2.4G 100mW.

�����ϵ�������ϵ�󣬺��Ʋ��������ջ�LED��˸����ʱ��ң������������������������С�����ջ��յ��ź�LED������
              ��ʾRFͨѶ�����ӡ���ʱ������"��"һ����������˸����ʾ����ģʽ��

������������ң�������Ҳ��ݸ��������ڰˣ��������ᣬ����"��"һ����4����������ʼ������ת�����Ƴ�����
          �˺��������ţ��Ϳ��Լ�����������ֱ����ɡ�

������У���ɺ󣬿��Բ������ֵĸ�����������ݸˣ�ʵ��ǰ�����һ����ⷽ��ķ��С�
          �������Ÿ������Ǻ�����ʱ��ת�������Ǻ���˳ʱ��ת��

�����½�ֹͣ�������ţ��������½������棬Ȼ�������ݸ���������ˣ�ֹͣ���ᣬ���´��ڴ���ģʽ��

����ˮƽУ׼�������������ˮƽ���棬���ڴ���ģʽ��Ȼ�������ݸ��������ڰˣ�����"��"һ������У׼����ɺ�"����"�������У׼��

����ȡ��ˮƽУ׼�������������ˮƽ���棬���ڴ���ģʽ��Ȼ�������ݸ���������ˣ�����"��"һ��ȡ��У׼��ȡ��ˮƽУ׼��δ����ˮƽУ׼�������ᣬ���ʱ��ʹ�޷�Ҳ���ܻ�������Ư�ơ�

��ص�ѹ����������ص�ѹʱ��������"����"������ͬʱ������˸����ʱ�뾡��غ����䡣

��ң���ź��쳣���������ڿ���ͻȻ�ղ���ң���ź�ʱ���������������"������"������ͬʱ������˸�����ᱣ��ˮƽ�����Զ���С���Ž��䡣


***********************************************/

#define		Baudrate1			115200UL
#define		TX1_LENGTH	128
#define		RX1_LENGTH	128


#include "config.h"
#include "STC8xxx_PWM.H"
#include "MPU6050.H"
#include "AD.H"	
#include "EEPROM.H"
#include "PCA.h"
#include <math.H>

sbit	P_Light  = P5^4;	//����
sbit	P_BUZZER = P5^5;	//������


int		xdata g_x=0,g_y=0,g_z=0;					//�����ǽ�������
float	xdata a_x=0,a_y=0;							//�ǶȽ�������
float	data  AngleX=0,AngleY=0;					//��Ԫ���������ŷ����
float	xdata Angle_gx=0,Angle_gy=0,Angle_gz=0;		//�ɽ��ٶȼ���Ľ�����(�Ƕ���)
float	xdata Angle_ax=0,Angle_ay=0,Angle_az=0;		//�ɼ��ٶȼ���ļ��ٶ�(������)
float	xdata Ax=0,Ay=0,Az=0;						//����ң������������ĽǶ�    
float	data PID_x=0,PID_y=0,PID_z=0;				//PID���������
int		data  speed0=0,speed1=0,speed2=0,speed3=0;	//����ٶȲ���
int		data  PWM0=0,PWM1=0,PWM2=0,PWM3=0;//,PWM4=0,PWM5=0;			//������PWMģ��Ĳ���

int		int_tmp;
u8		YM=0,FRX=128,FRY=128,FRZ=128;				//4ͨ��ң���ź�.
u8		xdata	tp[16];		//��MP6050����


//****************��̬������PID*********************************************

float xdata Out_PID_X=0,Last_Angle_gx=0;					//�⻷PI�����  ��һ������������
float xdata ERRORX_Out=0,ERRORX_In=0;			//�⻷P  �⻷I  �⻷������
float xdata Out_PID_Y=0,Last_Angle_gy=0;
float xdata ERRORY_Out=0,ERRORY_In=0;            //����1:���⻷P�˻�����10.5

float xdata Last_Ax=0,Last_Ay=0,Last_Az=0;


/******************************************************************************/
#define	Out_XP	6.65f	//ADC0	�⻷P	V1 / 10
#define	Out_XI	0.0074f	//ADC4	�⻷I	V2 / 10000
#define	Out_XD	6.0f	//ADC5	�⻷D	V3 / 10

#define	In_XP	0.8275f	//ADC6	�ڻ�P	V4 / 100
#define	In_XI	0.0074f	//ADC4	�ڻ�I	V2 / 10000
#define	In_XD	6.0f	//ADC5	�ڻ�D	V3 / 10


#define	Out_YP	Out_XP
#define	Out_YI	Out_XI
#define	Out_YD	Out_XD

#define	In_YP	In_XP
#define	In_YI	In_XI
#define	In_YD	In_XD


#define	ZP	5.0f
#define	ZI	0.1f
#define	ZD	4.0f	//�������Ƶ�P D
float Z_integral=0;//Z�����

#define	ERR_MAX	500
//======================================================================


u8	data YM_LostCnt=0, Lost16S; //��һ��RxBuf[0]����(RxBuf[0]�����ڲ��ϱ䶯��)   ״̬��ʶ
u8	SW2_tmp;


//======================================================================
bit	B_8ms;	//8ms��־

bit	B_rtn_ADC0;	//���󷵻���Ϣ
bit	B_BAT_LOW;	//�͵�ѹ��־
u8	xdata cnt_ms;		//ʱ�����


u8		xdata UART1_cmd=0;	//��������
u8		xdata TX1_Read=0;	//���Ͷ�ָ��
u8		xdata TX1_Write=0;	//����дָ��
u8		xdata TX1_cnt=0;	//���ͼ���
u8 		xdata TX1_Buffer[TX1_LENGTH];	//���ͻ���
bit		B_TX1_Busy;			//����æ��־
u8 		xdata RX1_Cnt,RX1_Timer;
u8 		xdata RX1_Buffer[RX1_LENGTH];
bit 	B_RX1_OK;


u8		xdata Cal_Setp=0;			//У׼����
u8		xdata Cal_cnt=0;			//У׼ƽ��ֵ����
int		xdata x_sum,y_sum,z_sum;	//У׼�ۼӺ�
float	xdata float_x_sum,float_y_sum;	//У׼�ۼӺ�

u8	xdata BuzzerOnTime,BuzzerOffTime,BuzzerRepeat,BuzzerOnCnt,BuzzerOffCnt;
u8	xdata cnt_100ms;


/* =================== PPM������ر��� ========================== */
u16	xdata CCAP0_RiseTime;		//��׽����������ʱ��
u8	xdata PPM1_Rise_TimeOut;	//�ߵ�ƽ��ʱ
u8	xdata PPM1_Rx_TimerOut;		//���ճ�ʱ����
u8	xdata PPM1_RxCnt;			//���մ�������
u16	xdata PPM1_Cap;				//��׽����PPM�������
bit	B_PPM1_OK;					//���յ�һ��PPM�������

u16	xdata CCAP1_RiseTime;
u8	xdata PPM2_Rise_TimeOut;	//�ߵ�ƽ��ʱ
u8	xdata PPM2_Rx_TimerOut;
u8	xdata PPM2_RxCnt;
u16	xdata PPM2_Cap;
bit	B_PPM2_OK;

u16	xdata CCAP2_RiseTime;
u8	xdata PPM3_Rise_TimeOut;	//�ߵ�ƽ��ʱ
u8	xdata PPM3_Rx_TimerOut;
u8	xdata PPM3_RxCnt;
u16	xdata PPM3_Cap;
bit	B_PPM3_OK;

u16	xdata CCAP3_RiseTime;
u8	xdata PPM4_Rise_TimeOut;	//�ߵ�ƽ��ʱ
u8	xdata PPM4_Rx_TimerOut;
u8	xdata PPM4_RxCnt;
u16	xdata PPM4_Cap;
bit	B_PPM4_OK;

u16	xdata CCAP_FallTime;

u8	PPM1,PPM2,PPM3,PPM4;
bit	B_Start;
u8	cnt_start;

/* ============================================= */


void	UART1_config(void);
void 	PrintString1(u8 *puts);	//����һ���ַ���
void	TX1_write2buff(u8 dat);	//д�뷢�ͻ��壬ָ��+1
void	TX1_int_value(int i);
void	delay_ms(u8 ms);
void	Return_Message(void);
u16 	MODBUS_CRC16(u8 *p,u8 n);	//input:	*p--->First Data Address,n----->Data Number,	return:	CRC16
void	PCA_config(void);
void 	Timer0_Config(void);
void 	Timer1_Config(void);
void	return_TTMx(u8 id,PPMx);
void 	Timer0_Config(void);
u16 	MODBUS_CRC16(u8 *p,u8 n);	//input:	*p--->First Data Address,n----->Data Number,	return:	CRC16

extern xdata u16	adc0;
extern xdata int	Battery;


//*********************************************************************
//****************�Ƕȼ���*********************************************
//*********************************************************************
#define	pi		3.14159265f                           
#define	Kp		0.8f                        
#define	Ki		0.001f                         
#define	halfT	0.004f           

float idata q0=1,q1=0,q2=0,q3=0;   
float idata exInt=0,eyInt=0,ezInt=0;  


void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az)
{
	float data norm;
	float idata vx, vy, vz;
	float idata ex, ey, ez;

	norm = sqrt(ax*ax + ay*ay + az*az);	//�Ѽ��ٶȼƵ���ά����ת�ɵ�ά����   
	ax = ax / norm;
	ay = ay / norm;
	az = az / norm;

		//	�����ǰ���Ԫ������ɡ��������Ҿ����еĵ����е�����Ԫ�ء� 
		//	�������Ҿ����ŷ���ǵĶ��壬��������ϵ������������ת����������ϵ��������������Ԫ��
		//	���������vx vy vz����ʵ���ǵ�ǰ��ŷ���ǣ�����Ԫ�����Ļ����������ϵ�ϣ����������
		//	������λ������
	vx = 2*(q1*q3 - q0*q2);
	vy = 2*(q0*q1 + q2*q3);
	vz = q0*q0 - q1*q1 - q2*q2 + q3*q3 ;

	ex = (ay*vz - az*vy) ;
	ey = (az*vx - ax*vz) ;
	ez = (ax*vy - ay*vx) ;

	exInt = exInt + ex * Ki;
	eyInt = eyInt + ey * Ki;
	ezInt = ezInt + ez * Ki;

	gx = gx + Kp*ex + exInt;
	gy = gy + Kp*ey + eyInt;
	gz = gz + Kp*ez + ezInt;

	q0 = q0 + (-q1*gx - q2*gy - q3*gz) * halfT;
	q1 = q1 + ( q0*gx + q2*gz - q3*gy) * halfT;
	q2 = q2 + ( q0*gy - q1*gz + q3*gx) * halfT;
	q3 = q3 + ( q0*gz + q1*gy - q2*gx) * halfT;

	norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
	q0 = q0 / norm;
	q1 = q1 / norm;
	q2 = q2 / norm;
	q3 = q3 / norm;

	AngleX = asin(2*(q0*q2 - q1*q3 )) * 57.2957795f; // ����   ����ɶ�
	AngleY = asin(2*(q0*q1 + q2*q3 )) * 57.2957795f; // ���
}



//****************��̬����*********************************************
void PWM_int (void) interrupt 	22	//PWM�жϺ���
{
	PWMCFG = 0;	//CBIF;	//����жϱ�־

	B_8ms = 1;

//======================== ��ʱ������� ==============================================
	PPM1_Rise_TimeOut++;	//�ߵ�ƽ��ʱ
	PPM2_Rise_TimeOut++;	//�ߵ�ƽ��ʱ
	PPM3_Rise_TimeOut++;	//�ߵ�ƽ��ʱ
	PPM4_Rise_TimeOut++;	//�ߵ�ƽ��ʱ

	if(--PPM1_Rx_TimerOut == 0)		//����100ms�ղ����ź�
	{
		PPM1_RxCnt = 0;			//һ���������, ��ʼ��n��������Ч
		PPM1 = 128;;			//Ĭ���е�
	}
	if(--PPM2_Rx_TimerOut == 0)		//����100ms�ղ����ź�
	{
		PPM2_RxCnt = 0;			//һ���������, ��ʼ��n��������Ч
		PPM2 = 128;;			//Ĭ���е�
	}
	if(--PPM3_Rx_TimerOut == 0)		//����200ms�ղ����ź�
	{
		PPM3_RxCnt = 0;			//һ���������, ��ʼ��n��������Ч
	}
	if(--PPM4_Rx_TimerOut == 0)		//����100ms�ղ����ź�
	{
		PPM4_RxCnt = 0;			//һ���������, ��ʼ��n��������Ч
		PPM4 = 128;				//Ĭ���е�
	}
//======================================================================

	if(++YM_LostCnt >= 250)		//ʧ��2���
	{
		YM_LostCnt = 200;		//�ظ�0.4�룬ʧ�ر���
		if(PPM3 > 80)	PPM3--;
		else if(++Lost16S >= 40)
		{
			Lost16S = 250;
			PPM3 = 0;
			B_Start = 0;
		}
	}
	if(YM_LostCnt  >= 25)	//ʧ��200ms
	{
		PPM1 = 128;
		PPM2 = 128;		//���� ��� �������0
		PPM4 = 128;
	}

	FRX = PPM1;
	FRY = PPM2;
	YM  = PPM3;	//����
	FRZ = PPM4;
	

//********************************************************************************************
	Read_MPU6050(tp);	//680us

	Angle_ax = ((float)(((int *)&tp)[0])) / 8192.0;	//���ٶȴ���	�����λ�� +- g
	Angle_ay = ((float)(((int *)&tp)[1])) / 8192.0;	//ת����ϵ	8192 LSB/g, 1g��Ӧ����8192
	Angle_az = ((float)(((int *)&tp)[2])) / 8192.0;	//���ٶ����� +-4g/S
	Last_Angle_gx = Angle_gx;		//������һ�ν��ٶ�����
	Last_Angle_gy = Angle_gy;
	Angle_gx = ((float)(((int *)&tp)[4] - g_x)) / 65.5;	//�����Ǵ���	�����λ�� +-��
	Angle_gy = ((float)(((int *)&tp)[5] - g_y)) / 65.5;	//���������� +-500��/S, 1��/�� ��Ӧ���� 65.536
	Angle_gz = ((float)(((int *)&tp)[6] - g_z)) / 65.5;	//ת����ϵ65.5 LSB/��

	IMUupdate(Angle_gx*0.0174533f, Angle_gy*0.0174533f, Angle_gz*0.0174533f, Angle_ax,Angle_ay,Angle_az);

//**********************************X��ָ��************************************************
	Ax  = AngleX - a_x - ((float)FRX - 128) / 4.0;		//�Ƕȿ������������Ƕ�

	if(YM > 35)	ERRORX_Out += Ax,	ERRORX_Out += Ax,	ERRORX_Out += Ax;	//�⻷����(����С��ĳ��ֵʱ������)
	else		ERRORX_Out = 0; //����С�ڶ�ֵʱ�������ֵ
		 if(ERRORX_Out >  1500)	ERRORX_Out =  1500;
	else if(ERRORX_Out < -1500)	ERRORX_Out = -1500;	//�����޷�

	Out_PID_X = Ax*Out_XP + ERRORX_Out*Out_XI + (Ax-Last_Ax)*Out_XD;	//�⻷PI
	Last_Ax = Ax;
	
	if(YM > 35)	ERRORX_In += (Angle_gy - Out_PID_X);	//�ڻ�����(����С��ĳ��ֵʱ������)
	else		ERRORX_In = 0;	//����С�ڶ�ֵʱ�������ֵ
		 if(ERRORX_In >  500)	ERRORX_In =  500;
	else if(ERRORX_In < -500)	ERRORX_In = -500;	//�����޷�

	PID_x = (Angle_gy + Out_PID_X) * In_XP + ERRORX_In * In_XI + (Angle_gy - Last_Angle_gy) * In_XD;	//�ڻ�PID
	if(PID_x >  500)	PID_x =  500;	//������޷�
	if(PID_x < -500)	PID_x = -500;

//**************Y��ָ��**************************************************
	Ay  = AngleY - a_y + ((float)FRY - 128) / 4.0;		//�Ƕȿ������������Ƕ�
	
	if(YM > 35)	ERRORY_Out += Ay,	ERRORY_Out += Ay,	ERRORY_Out += Ay;	//�⻷����(����С��ĳ��ֵʱ������)
	else		ERRORY_Out = 0; //����С�ڶ�ֵʱ�������ֵ
		 if(ERRORY_Out >  1500)	ERRORY_Out =  1500;
	else if(ERRORY_Out < -1500)	ERRORY_Out = -1500;	//�����޷�
	
	Out_PID_Y = Ay * Out_YP + ERRORY_Out * Out_YI + (Ay-Last_Ay)*Out_YD;	//�⻷PID
	Last_Ay = Ay;

	if(YM > 35)	ERRORY_In += (Angle_gx - Out_PID_Y);  //�ڻ�����(����С��ĳ��ֵʱ������)
	else		ERRORY_In = 0; //����С�ڶ�ֵʱ�������ֵ
		 if(ERRORY_In >  500)	ERRORY_In =  500;
	else if(ERRORY_In < -500)	ERRORY_In = -500;	//�����޷�
	
	PID_y = (Angle_gx + Out_PID_Y) * In_YP + ERRORY_In * In_YI + (Angle_gx - Last_Angle_gx) * In_YD;	//�ڻ�PID
	
	if(PID_y > 500)	PID_y =  500;	//������޷�
	if(PID_y <-500)	PID_y = -500;

//**************Z��ָ��(Z�����������������û��Ҫ�ϴ���PID)*****************************	
	Az = Angle_gz - ((float)FRZ - 128);
	
	if(YM > 35)	Z_integral += Az;	//Z�����
	else		Z_integral = 0;		//����С��40��������
		 if(Z_integral >  500.0f)	Z_integral =  500.0f;	//�����޷�
	else if(Z_integral < -500.0f)	Z_integral = -500.0f;	//�����޷�

	PID_z = Az * ZP + Z_integral * ZI + (Az - Last_Az) * ZD;
	Last_Az = Az;
	if(PID_z >  200)	PID_z =  200;	//������޷�
	if(PID_z < -200)	PID_z = -200;

	speed0 = (int)(  PID_x + PID_y + PID_z);	//M1��Ϊ��ʱ��
	speed1 = (int)(  PID_x - PID_y - PID_z);
	speed2 = (int)( -PID_x - PID_y + PID_z);
	speed3 = (int)( -PID_x + PID_y - PID_z);

//**************���ٶȲ���������PWMģ��*************************************************	
	
	if(YM < 10)	PWM0 = 1000, PWM1 = 1000, PWM2 = 1000, PWM3 = 1000;
	else if(YM < 35)	PWM0 = 860, PWM1 = 860, PWM2 = 860, PWM3 = 860;
	else
	{
		int_tmp = 1000 - (int)YM * 4;

		PWM0 = int_tmp - speed0;

			 if(PWM0 > 1000)	PWM0 = 1000;    //�ٶȲ������ƣ���ֹ����PWM������Χ0-1000
		else if(PWM0 < 10)		PWM0 = 10;

		PWM1 = int_tmp - speed1;

			 if(PWM1 > 1000)	PWM1 = 1000;
		else if(PWM1 < 10)		PWM1 = 10;

		PWM2 = int_tmp - speed2;

			 if(PWM2 > 1000)	PWM2 = 1000;
		else if(PWM2 < 10)		PWM2 = 10;

		PWM3 = int_tmp - speed3;

			 if(PWM3 > 1000)	PWM3 = 1000;
		else if(PWM3 < 10)		PWM3 = 10;
	}

	SW2_tmp = P_SW2;	//����SW2����
	EAXSFR();	//����XFR
	PWM0T2 = (u16)(PWM0 * 2);
	PWM1T2 = (u16)(PWM1 * 2);
	PWM2T2 = (u16)(PWM2 * 2);
	PWM3T2 = (u16)(PWM3 * 2);	
	P_SW2  = SW2_tmp;	//�ָ�SW2����

}


/********************** �������� ************************/
void	beep(void)	//100ms����
{
	if(BuzzerRepeat > 0)	//����������, �ظ�������Ϊ0���������Ҫ����
	{
		if((BuzzerOnCnt == 0) && (BuzzerOffCnt == 0))	//On��OFF��Ϊ0����ʼװ��On��Off��ʱ��
		{
			P_BUZZER = 1;			//��������
			BuzzerOnCnt  = BuzzerOnTime;	//װ��on����
			BuzzerOffCnt = BuzzerOffTime;	//װ��off����
		}
		else if(BuzzerOnCnt  > 0)	{if(--BuzzerOnCnt == 0)	P_BUZZER = 0;}	//On��ʱ��
		else if(BuzzerOffCnt > 0)	//Off��ʱ��
		{
			if(--BuzzerOffCnt == 0)	BuzzerRepeat--;
		}
	}
	else	P_BUZZER = 0;
}

void	SetBuzzer(u8 on,u8 off,u8 rep)	// rep: �ظ�����, on: on��ʱ��, off: off��ʱ��
{
	BuzzerRepeat = rep;
	BuzzerOnTime  = on;
	BuzzerOffTime = off;
	if(BuzzerOnTime  == 0)	BuzzerOnTime  = 1;
	if(BuzzerOffTime == 0)	BuzzerOffTime = 1;
	if(BuzzerRepeat == 1)	BuzzerOffTime = 1;
	BuzzerOnCnt = 0,	BuzzerOffCnt = 0;
}

// ===================== �Զ�У׼���� =====================
void	AutoCal(void)
{
	if(PPM3 < 40)	//ֹͣʱ������У׼
	{
		if(Cal_Setp == 1)	//����У׼����
		{
			x_sum = 0;	y_sum = 0;	z_sum = 0;
			Cal_cnt  = 0;
			Cal_Setp = 2;
		}
		else if(Cal_Setp == 2)	//���������ۼ�
		{
			x_sum += ((int *)&tp)[4];  //��ȡ����������
			y_sum += ((int *)&tp)[5];
			z_sum += ((int *)&tp)[6];
			if(++Cal_cnt >= 64)
			{
				g_x = x_sum / 64;
				g_y = y_sum / 64;
				g_z = z_sum / 64;
				float_x_sum = 0;	float_y_sum = 0;
				Cal_cnt  = 0;
				Cal_Setp = 3;
			}
		}
		else if(Cal_Setp == 3)	//��X Y�Ƕ��ۼ�
		{
			float_x_sum += AngleX;
			float_y_sum += AngleY;
			if(++Cal_cnt >= 64)
			{
				Cal_cnt  = 0;
				Cal_Setp = 0;
				a_x = float_x_sum / 64.0;
				a_y = float_y_sum / 64.0;
				IAP_Gyro();
				SetBuzzer(5,1,1);
			}
		}
	}
	else
	{
		Cal_Setp = 0;
		Cal_cnt  = 0;
	}
}

// ===================== ������ =====================
void main(void)
{

	//����I/O��ȫ��Ϊ׼˫��������ģʽ
	P0M0=0x00;	P0M1=0x00;
	P1M0=0x00;	P1M1=0x00;
	P2M0=0x00;	P2M1=0x00;
	P3M0=0x00;	P3M1=0x00;
	P4M0=0x00;	P4M1=0x00;
	P5M0=0x00;	P5M1=0x00;
	P6M0=0x00;	P6M1=0x00;
	P7M0=0x00;	P7M1=0x00;

	PPM1 = 128;
	PPM2 = 128;
	PPM3 = 0;
	PPM4 = 128;

	PWMGO();

	P_Light  = 0;
	P_BUZZER = 0;
	P5n_push_pull(0x30);

	adc_init();    //����A/D
	
	PCA_config();

	delay_ms(100);
	IAPRead();		//��ȡ�����Ǿ���
	InitMPU6050();	//��ʼ��MPU-6050
	delay_ms(100);

	PWMCR =  0xc0;//ECBI;	//����PWM�����������ж�
	EA = 1;	//�������ж�
	
	cnt_start = 0;
	while(cnt_start < 25)	//�ȴ�������С	20ms * 25 = 500ms
	{
		if(B_PPM3_OK)	//����
		{
			B_PPM3_OK = 0;
			if(PPM3_Cap <= 1200)	cnt_start++;
		}
		delay_ms(1);
	}
	P_Light  = 0;
	
	cnt_start = 0;

	SetBuzzer(5,1,1);
	

//==============================================
	UART1_config();	// ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
	PrintString1("STC15W4Kϵ�д�����ɿس���!\r\n");	//SUART1����һ���ַ���
//==============================================

	B_Start = 0;	//�ϵ��ֹ����

	while(1)
	{
		if(B_PPM1_OK)	//����(���)
		{
			B_PPM1_OK = 0;
				 if(PPM1_Cap < 1120)	PPM1_Cap = 1120;
			else if(PPM1_Cap > 1880)	PPM1_Cap = 1880;
			PPM2 = (u8)((PPM1_Cap-1116)/3);	//תΪ0~255, �м�ֵΪ128
		}
		
		if(B_PPM2_OK)	//ǰ��(����)
		{
			B_PPM2_OK = 0;
				 if(PPM2_Cap < 1120)	PPM2_Cap = 1120;
			else if(PPM2_Cap > 1880)	PPM2_Cap = 1880;
			PPM1 = (u8)((PPM2_Cap-1116)/3);	//תΪ0~255, �м�ֵΪ128
		}
		
		if(B_PPM4_OK)	//����
		{
			B_PPM4_OK = 0;
				 if(PPM4_Cap < 1056)	PPM4_Cap = 1056;
				 if(PPM4_Cap > 1940)	PPM4_Cap = 1940;
				 if(PPM4_Cap < 1440)	PPM4_Cap = PPM4_Cap + 60;
			else if(PPM4_Cap > 1560)	PPM4_Cap = PPM4_Cap - 60;
			else	PPM4_Cap = 1500;
			PPM4 = (u8)((PPM4_Cap-1116)/3);	//תΪ0~255, �м�ֵΪ128
		}
		
		if(B_PPM3_OK)	//����
		{
			B_PPM3_OK = 0;
			if(PPM3_Cap < 1000)	PPM3_Cap = 1000;
			if(PPM3_Cap > 1900)	PPM3_Cap = 1900;

			if(B_Start)		//��������ʱ,
			{
				PPM3 = (u8)((PPM3_Cap-1000)/4);	//תΪ0~255,	ʵ��8~225
				if(PPM3 < 32)	PPM3 = 32;
				
				if((PPM1 < 50) && (PPM2 < 50) && (PPM3_Cap < 1120) && (PPM4 > 200))	//�����, ��ֹ
				{
					if(++cnt_start >= 50)	//1��
					{
						cnt_start = 0;
						B_Start = 0;
						SetBuzzer(1,1,2);
					}
				}
				else	cnt_start = 0;
			}
			else	//��ֹ����ʱ, �ȴ��ڰ˿���
			{
				PPM3 = 0;
				if((PPM1 < 50) && (PPM2 > 200) && (PPM3_Cap < 1120) && (PPM4 < 50))	//���ڰ�, ����
				{
					if(++cnt_start >= 50)	//1��
					{
						cnt_start = 0;
						B_Start = 1;
						SetBuzzer(5,1,1);
					}
				}
				else if((PPM1 > 200) && (PPM2 > 200) && (PPM3_Cap > 1850) && (PPM4 < 50))	//���ڰ�, ˮƽУ׼
				{
					if(++cnt_start >= 50)	//1��
					{
						cnt_start = 0;
						SetBuzzer(2,1,1);
						Cal_Setp = 1;
					}
				}
				else if((PPM1 > 200) && (PPM2 < 50) && (PPM3_Cap > 1850) && (PPM4 > 200))	//�����, ȡ��ˮƽУ׼
				{
					if(++cnt_start >= 50)	//1��
					{
						cnt_start = 0;
						g_x = 0;
						g_y = 0;
						g_z = 0;
						a_x = 0;
						a_y = 0;
						IAP_Gyro();
						SetBuzzer(1,1,2);
					}
				}
				else	cnt_start = 0;
			}
		}


		if(B_8ms)		//8ms��
		{
			B_8ms = 0;
			
			if(Cal_Setp != 0)	AutoCal();	//�Ƿ�ִ���Զ�У׼����
			AD();		// ��ADC�����ѹ

			if(++cnt_100ms >= 12)	cnt_100ms = 0,	beep();	//100ms����һ�η�����

			B = cnt_ms;
			++cnt_ms;
			B = (B ^ cnt_ms) & cnt_ms;

			if(B2)		//64ms
			{
				if(!B_BAT_LOW && (YM_LostCnt < 120))	//��ѹ��, �ź�����
				{
					if(!B_Start)	P_Light = 0;	// ����ʱ, ������(ÿ2048ms��64ms)
					else 			P_Light = 1;	// ������, �Ƴ���
				}
			}
			else if(B4)		//256ms
			{
				if(B_BAT_LOW || (YM_LostCnt >= 120))	P_Light = ~P_Light;		//��ѹ��, �����ź�, ������˸ 2HZ
			}
			else if(B6)		//1024ms
			{
				if(Battery < 1090)	B_BAT_LOW = 1;	else if(Battery > 1110)	B_BAT_LOW = 0;	//<10.90V��ѹ��, >11.10V��ѹ��
				
				if(B_BAT_LOW)	SetBuzzer(1,1,2);	//��ѹ��
				
				if(B_rtn_ADC0)	Return_Message();	//���󷵻�ADC0����

				if(!B_BAT_LOW && (YM_LostCnt < 120))	P_Light = 1;	//ң���ź�����,	��ѹ����ʱ
			}
			else if(B7)		//2048ms
			{
				if(!B_BAT_LOW && (YM_LostCnt >= 120))	SetBuzzer(1,1,3);	//��ѹ����ʱ ң���źŶ�ʧ, ÿ�������3��,
			}
		}


		if(UART1_cmd != 0)
		{
			if(UART1_cmd == 'a')		//PC����a���ɿط���һЩ����
			{
				B_rtn_ADC0 = ~B_rtn_ADC0;
			}
			UART1_cmd = 0;
		}
		
		
		if((TX1_Read != TX1_Write) && (!B_TX1_Busy))	//������Ҫ����, ���ҷ��Ϳ���
		{
			SBUF = TX1_Buffer[TX1_Read];
			B_TX1_Busy = 1;
			if(++TX1_Read >= TX1_LENGTH)	TX1_Read = 0;
		}

	}
}

//=========================================================

void	Return_Message(void)
{
	TX1_write2buff('V');
	TX1_write2buff('=');
	TX1_write2buff(Battery/1000 + '0');
	TX1_write2buff((Battery%1000)/100 + '0');
	TX1_write2buff('.');
	TX1_write2buff((Battery%100)/10 + '0');
	TX1_write2buff(Battery%10 + '0');
	TX1_write2buff(' ');
	TX1_write2buff(' ');

	PrintString1("AngleX=");
	TX1_int_value((int)(AngleX * 10));

	PrintString1("AngleY=");
	TX1_int_value((int)(AngleY * 10));

	PrintString1("AngleZ=");
	TX1_int_value((int)(Angle_gz * 10));

	PrintString1("a_x=");
	TX1_int_value(a_x * 10);
	PrintString1("a_y=");
	TX1_int_value(a_y * 10);
	PrintString1("g_z=");
	TX1_int_value(g_z * 10);

	TX1_cnt = 0;
	TX1_write2buff(0x0d);
	TX1_write2buff(0x0a);
}


void  delay_ms(u8 ms)
{
     u16 i;
	 do
	 {
	 	i = MAIN_Fosc / 13000;
		while(--i)	;   //13T per loop
     }while(--ms);
}


void	TX1_int_value(int i)
{
	if(i < 0)	TX1_write2buff('-'),	i = 0 - i;
	else		TX1_write2buff(' ');
	TX1_write2buff(i / 1000 + '0');
	TX1_write2buff((i % 1000) / 100 + '0');
	TX1_write2buff((i % 100) / 10 + '0');
	TX1_write2buff('.');
	TX1_write2buff(i % 10 + '0');
	TX1_write2buff(' ');
	TX1_write2buff(' ');
}

/*************** װ�ش���1���ͻ��� *******************************/
void TX1_write2buff(u8 dat)	//д�뷢�ͻ��壬ָ��+1
{
	TX1_Buffer[TX1_Write] = dat;
	if(++TX1_Write >= TX1_LENGTH)	TX1_Write = 0;
}


//========================================================================
// ����: void PrintString1(u8 *puts)
// ����: ����1�����ַ���������
// ����: puts:  �ַ���ָ��.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע: 
//========================================================================
void PrintString1(u8 *puts)	//����һ���ַ���
{
	for (; *puts != 0;	puts++)   TX1_write2buff(*puts);	//����ֹͣ��0����
}

//========================================================================
// ����: SetTimer2Baudrate(u16 dat)
// ����: ����Timer2�������ʷ�������
// ����: dat: Timer2����װֵ.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע: 
//========================================================================

void	SetTimer2Baudrate(u16 dat)	// ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
{
	AUXR &= ~(1<<4);	//Timer stop
	AUXR &= ~(1<<3);	//Timer2 set As Timer
	AUXR |=  (1<<2);	//Timer2 set as 1T mode
	TH2 = dat / 256;
	TL2 = dat % 256;
	IE2  &= ~(1<<2);	//��ֹ�ж�
	AUXR |=  (1<<4);	//Timer run enable
}


//========================================================================
// ����: void	UART1_config(u8 brt)
// ����: UART1��ʼ��������
// ����: brt: ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע: 
//========================================================================
void	UART1_config(void)
{
	/*********** ������ʹ�ö�ʱ��2 *****************/
	AUXR |= 0x01;		//S1 BRT Use Timer2;
	SetTimer2Baudrate(65536UL - (MAIN_Fosc / 4) / Baudrate1);

	/*********** ������ʹ�ö�ʱ��1 *****************/
/*	TR1 = 0;
	AUXR &= ~0x01;		//S1 BRT Use Timer1;
	AUXR |=  (1<<6);	//Timer1 set as 1T mode
	TMOD &= ~(1<<6);	//Timer1 set As Timer
	TMOD &= ~0x30;		//Timer1_16bitAutoReload;
	TH1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) / 256);
	TL1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) % 256);
	ET1 = 0;	//��ֹ�ж�
	INT_CLKO &= ~0x02;	//�����ʱ��
	TR1  = 1;
*/	//========================================================================

	SCON = (SCON & 0x3f) | 0x40;	//UART1ģʽ, 0x00: ͬ����λ���, 0x40: 8λ����,�ɱ䲨����, 0x80: 9λ����,�̶�������, 0xc0: 9λ����,�ɱ䲨����
	PS  = 1;	//�����ȼ��ж�
	ES  = 1;	//�����ж�
	REN = 1;	//��������
	P_SW1 &= 0x3f;
	P_SW1 |= 0x00;		//UART1 switch to, 0x00: P3.0 P3.1, 0x40: P3.6 P3.7, 0x80: P1.6 P1.7 (����ʹ���ڲ�ʱ��)
//	PCON2 |=  (1<<4);	//�ڲ���·RXD��TXD, ���м�, ENABLE,DISABLE

	B_TX1_Busy = 0;
	TX1_Read   = 0;
	TX1_Write  = 0;
	UART1_cmd  = 0;
	TX1_cnt    = 0;
}


//========================================================================
// ����: void UART1_int (void) interrupt UART1_VECTOR
// ����: UART1�жϺ�����
// ����: nine.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע: 
//========================================================================
void UART1_int (void) interrupt 4
{
	if(RI)
	{
		RI = 0;
		UART1_cmd = SBUF;
	}

	if(TI)
	{
		TI = 0;
		B_TX1_Busy = 0;
	}
}



void	PCA_config(void)
{
	PPM1_Rise_TimeOut = 0;
	PPM2_Rise_TimeOut = 0;
	PPM3_Rise_TimeOut = 0;
	PPM4_Rise_TimeOut = 0;

	CR = 0;
	CH = 0;
	CL = 0;
	AUXR1 = (AUXR1 & ~(3<<4)) | PCA_P12_P17_P16_P15_P14;	//�л�IO��
	CMOD  = (CMOD  & ~(7<<1)) | PCA_Clock_12T;				//ѡ��ʱ��Դ  STC8F8K D�汾
//	CMOD  = (CMOD  & ~1) | 1;								//ECF
	PPCA = 1;	//�����ȼ��ж�

	CCAPM0     = PCA_Mode_Capture | PCA_Rise_Active | PCA_Fall_Active | ENABLE;	//����ģʽ, �ж�ģʽ
	PCA_PWM0   = PCA_PWM_8bit;	//PWM����
//	CCAP0L = (u8)CCAP0_tmp;			//��Ӱ��Ĵ���д�벶��Ĵ�������дCCAPnL
//	CCAP0H = (u8)(CCAP0_tmp >> 8);	//��дCCAPnH

	CCAPM1     = PCA_Mode_Capture | PCA_Rise_Active | PCA_Fall_Active | ENABLE;	//����ģʽ, �ж�ģʽ
	PCA_PWM1   = PCA_PWM_8bit;	//PWM����
//	CCAP1L = (u8)CCAP1_tmp;			//��Ӱ��Ĵ���д�벶��Ĵ�������дCCAPnL
//	CCAP1H = (u8)(CCAP1_tmp >> 8);	//��дCCAPnH

	CCAPM2     = PCA_Mode_Capture | PCA_Rise_Active | PCA_Fall_Active | ENABLE;	//����ģʽ, �ж�ģʽ
	PCA_PWM2   = PCA_PWM_8bit;	//PWM����
//	CCAP2L = (u8)CCAP2_tmp;			//��Ӱ��Ĵ���д�벶��Ĵ�������дCCAPnL
//	CCAP2H = (u8)(CCAP2_tmp >> 8);	//��дCCAPnH

	CCAPM3     = PCA_Mode_Capture | PCA_Rise_Active | PCA_Fall_Active | ENABLE;	//����ģʽ, �ж�ģʽ
	PCA_PWM3   = PCA_PWM_8bit;	//PWM����
//	CCAP3L = (u8)CCAP3_tmp;			//��Ӱ��Ĵ���д�벶��Ĵ�������дCCAPnL
//	CCAP3H = (u8)(CCAP3_tmp >> 8);	//��дCCAPnH

	CR = 1;
}


//========================================================================
// ����: void	PCA_Handler (void) interrupt PCA_VECTOR
// ����: PCA�жϴ�������.
// ����: None
// ����: none.
// �汾: V1.0, 2012-11-22
//========================================================================
void	PCA_Handler (void) interrupt PCA_VECTOR
{
	if(CCF0)		//PCAģ��0�ж�
	{
		CCF0 = 0;		//��PCAģ��0�жϱ�־
		if(P17)	//������
		{
			CCAP0_RiseTime = ((u16)CCAP0H << 8) + CCAP0L;	//��CCAP0
			PPM1_Rise_TimeOut = 1;	//�յ�������, �ߵ�ƽ��ʱ
		}
		else	//�½���
		{
			CCAP_FallTime = ((u16)CCAP0H << 8) + CCAP0L;	//��CCAP0
			if((PPM1_Rise_TimeOut != 0) && (PPM1_Rise_TimeOut < 3))	//�յ���������, �ߵ�ƽҲû�����
			{
				CCAP_FallTime = (CCAP_FallTime - CCAP0_RiseTime) >> 1;	//Ϊ�˺ô���, ת�ɵ�λΪus
				if((CCAP_FallTime >= 800) && (CCAP_FallTime <= 2500))
				{
					if(++PPM1_RxCnt >= 5)	PPM1_RxCnt = 5;		//�������յ�5������
					if(PPM1_RxCnt == 5)
					{
						if(!B_PPM1_OK)
						{
							PPM1_Cap = CCAP_FallTime;
							B_PPM1_OK = 1;		//��־�յ�һ������
							PPM1_Rx_TimerOut = 12;	//��ʱ�ղ�������
						}
					}
				}
			}
			PPM1_Rise_TimeOut = 0;
		}
	}

	if(CCF1)	//PCAģ��1�ж�
	{
		CCF1 = 0;		//��PCAģ��1�жϱ�־
		if(P16)	//������
		{
			CCAP1_RiseTime = ((u16)CCAP1H << 8) + CCAP1L;	//��CCAP1
			PPM2_Rise_TimeOut = 1;	//�յ�������, �ߵ�ƽ��ʱ
		}
		else	//�½���
		{
			CCAP_FallTime = ((u16)CCAP1H << 8) + CCAP1L;	//��CCAP1
			if((PPM2_Rise_TimeOut != 0) && (PPM2_Rise_TimeOut < 3))	//�յ���������, �ߵ�ƽҲû�����
			{
				CCAP_FallTime = (CCAP_FallTime - CCAP1_RiseTime) >> 1;	//Ϊ�˺ô���, ת�ɵ�λΪus
				if((CCAP_FallTime >= 800) && (CCAP_FallTime <= 2500))
				{
					if(++PPM2_RxCnt >= 5)	PPM2_RxCnt = 5;
					if(PPM2_RxCnt == 5)
					{
						if(!B_PPM2_OK)
						{
							PPM2_Cap = CCAP_FallTime;
							B_PPM2_OK = 1;		//��־�յ�һ������
							PPM2_Rx_TimerOut = 12;	//��ʱ�ղ�������
						}
					}
				}
			}
			PPM2_Rise_TimeOut = 0;
		}
	}

	if(CCF2)	//PCAģ��2�ж�
	{
		CCF2 = 0;		//��PCAģ��1�жϱ�־
		if(P15)	//������
		{
			CCAP2_RiseTime = ((u16)CCAP2H << 8) + CCAP2L;	//��CCAP2
			PPM3_Rise_TimeOut = 1;	//�յ�������, �ߵ�ƽ��ʱ
		}
		else	//�½���
		{
			CCAP_FallTime = ((u16)CCAP2H << 8) + CCAP2L;	//��CCAP2
			if((PPM3_Rise_TimeOut != 0) && (PPM3_Rise_TimeOut < 3))	//�յ���������, �ߵ�ƽҲû�����
			{
				CCAP_FallTime = (CCAP_FallTime - CCAP2_RiseTime) >> 1;	//Ϊ�˺ô���, ת�ɵ�λΪus
				if((CCAP_FallTime >= 800) && (CCAP_FallTime <= 2500))
				{
					if(++PPM3_RxCnt >= 5)	PPM3_RxCnt = 5;
					if(PPM3_RxCnt == 5)
					{
						if(!B_PPM3_OK)
						{
							PPM3_Cap = CCAP_FallTime;
							B_PPM3_OK = 1;		//��־�յ�һ������
							PPM3_Rx_TimerOut = 25;	//��ʱ�ղ�������
							YM_LostCnt = 0;
							Lost16S    = 0;
						}
					}
				}
			}
			PPM3_Rise_TimeOut = 0;
		}
	}

	if(CCF3)	//PCAģ��3�ж�
	{
		CCF3 = 0;		//��PCAģ��1�жϱ�־
		if(P14)	//������
		{
			CCAP3_RiseTime = ((u16)CCAP3H << 8) + CCAP3L;	//��CCAP3
			PPM4_Rise_TimeOut = 1;	//�յ�������, �ߵ�ƽ��ʱ
		}
		else	//�½���
		{
			CCAP_FallTime = ((u16)CCAP3H << 8) + CCAP3L;	//��CCAP3
			if((PPM4_Rise_TimeOut != 0) && (PPM4_Rise_TimeOut < 3))	//�յ���������, �ߵ�ƽҲû�����
			{
				CCAP_FallTime = (CCAP_FallTime - CCAP3_RiseTime) >> 1;	//Ϊ�˺ô���, ת�ɵ�λΪus
				if((CCAP_FallTime >= 800) && (CCAP_FallTime <= 2500))
				{
					if(++PPM4_RxCnt >= 5)	PPM4_RxCnt = 5;
					if(PPM4_RxCnt == 5)
					{
						if(!B_PPM4_OK)
						{
							PPM4_Cap = CCAP_FallTime;
							B_PPM4_OK = 1;		//��־�յ�һ������
							PPM4_Rx_TimerOut = 12;	//��ʱ�ղ�������
						}
					}
				}
			}
			PPM4_Rise_TimeOut = 0;
		}
	}

//	if(CF)	//PCA����ж�
//	{
//		CF = 0;			//��PCA����жϱ�־
//	}

}