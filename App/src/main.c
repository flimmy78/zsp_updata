/*摄像头60*80，屏幕128*128*/
#include "common.h"
#include "include.h"


//函数声明
void PORTA_IRQHandler();
void DMA0_IRQHandler();

int _temp = 0;

Screen_Data screen_data[] = {

{ "P",&(STEER_KP),0.1,1 },
{ "I",&(STEER_KI),0.1,2 },
{ "D",&(STEER_KD),0.1,3 },

{ "enM",&(motor_go),99,0 }, //使能电机
{ "speed",&(motor_speed),10,4 },

{ "length",&(total_distance),500,0 },

{ "flash",&(flash_in),1,-1},

{ "end",&(temp_s[9]),1202,0 }
};

/*
图像不压缩直接显示
*/
void  main(void)
{
	Site_t site = { 0, 0 };             
	Size_t size;  
	Size_t imgsize = { CAMERA_W, CAMERA_H };
	     
	size.H = LCD_H;
	size.W = LCD_W;
	//size.H = LCD_H/2;
	//size.W = LCD_W/2;
	//size.H = 60;
	//size.W = 80;

	LCD_init();
	camera_init(imgbuff);
    ftm_pwm_init(FTM0, FTM_CH5, 300, 0);
	ftm_pwm_init(FTM0, FTM_CH6, 300, 43);
	UI_INIT();

	set_vector_handler(PORTA_VECTORn, PORTA_IRQHandler);   //设置 PORTA 的中断服务函数为 PORTA_IRQHandler
	set_vector_handler(DMA0_VECTORn, DMA0_IRQHandler);     //设置 DMA0 的中断服务函数为 PORTA_IRQHandler
	set_vector_handler(PORTD_VECTORn, PORTD_IRQHandler);   //ui所需中断的初始化
	Quad_Init();                                           //编码器中断
	flash_init();
	flash_Out();
	

	while (1)
	{    
		if (IMG_MODE == lcd_mode)
		{
			camera_get_img();                            //相机获取图像                               
			img_extract(img, imgbuff, CAMERA_SIZE);      //解压图像
			//temp_s[6] = Find_slope();
			Search_line();                               //找线
			Negation();
			img_compress(img, imgbuff, CAMERA_SIZE);     //图像压缩
			LCD_Img_Binary_Z(site, size, imgbuff, imgsize);//lcd显示图像
			//LCD_numf(tem_site_str[2], temp_s[0], GREEN, BLUE);  
			//LCD_numf(tem_site_data[2], temp_s[1], GREEN, BLUE);
			
			LCD_numf(tem_site_str[2], iscross_flag, GREEN, BLUE);
			LCD_numf(tem_site_str[3], isisland_flag, GREEN, BLUE);
			LCD_numf(tem_site_str[4], isisland_flag1, GREEN, BLUE);
			LCD_numf(tem_site_data[4], land_distance_count1, GREEN, BLUE);
			LCD_numf(tem_site_str[5], temp_s[5], GREEN, BLUE);
            LCD_numf(tem_site_data[5], sizeof(double), GREEN, BLUE);

			Control_core();

			if (1 == key_on) enable_irq(PORTD_IRQn);     //激活按键中断
		}
		else Open_UI();

		Update_Motor();

		if (UI_MODE == lcd_mode)
		{
			if (1 == ((int)motor_go) % 2 && total_distance < 1000) ftm_pwm_duty(FTM0, FTM_CH5, (int)motor_speed);
			else ftm_pwm_duty(FTM0, FTM_CH5, 0);
			ftm_pwm_duty(FTM0, FTM_CH6, 380); //舵机回中
		}
		else
		{
			if (total_distance < 1000)
			{
				ftm_pwm_duty(FTM0, FTM_CH5, (int)motor_speed);                    //电机
			}
			else ftm_pwm_duty(FTM0, FTM_CH5, 0);
		
			ftm_pwm_duty(FTM0, FTM_CH6, 380 + (int)steer_engine_degree);       //舵机 
		}
	}//while
}



/*!
*  @brief      PORTA中断服务函数，与摄像头有关
*  @since      v5.0
*/
void PORTA_IRQHandler()
{
	uint8  n;    //引脚号
	uint32 flag;
	_temp++;
	while (!PORTA_ISFR);
	flag = PORTA_ISFR;
	PORTA_ISFR = ~0;                                   //清中断标志位

	n = 29;                                             //场中断
	if (flag & (1 << n))                                 //PTA29触发中断
	{
		camera_vsync();
	}
#if ( CAMERA_USE_HREF == 1 )                            //使用行中断
	n = 28;
	if (flag & (1 << n))                                 //PTA28触发中断
	{
		camera_href();
	}
#endif


}

/*!
*  @brief      DMA0中断服务函数，与摄像头有关
*  @since      v5.0
*/
void DMA0_IRQHandler()
{
	camera_dma();
}






