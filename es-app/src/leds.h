#ifndef __LEDS_H__
#define __LEDS_H__

#define MAX_PWM 4096

extern unsigned char Init_Led_Driver(void);
extern void Init_Led_Controller(int active);

typedef struct
{
	int IsMarquee;
	
	int R; int G; int B; int W;
	int R_Old; int G_Old; int B_Old; int W_Old;
}
T_Led_Strip;

typedef struct
{
	int Active;
		
	T_Led_Strip Strips[4];
}
T_Led_Controller;

extern T_Led_Controller Led_Controller;

extern void Write_Strip (int strip, int r, int g, int b, int w);
extern void Write_Channel (int channel, int v);
extern void Turn_On_Marquee (void);
extern void Turn_Off_Marquee (void);
extern void Turn_Off_All (void);

#endif
