#ifndef __LEDS_H__
#define __LEDS_H__

#define MAX_PWM 4096

extern unsigned char Init_Led_Driver(void);
extern void Init_Led_Controller(int active, int base_ch, int mode);

typedef struct
{
	int BaseChannel;
	
	int R;
	int G;
	int B;

	int R_Old;
	int G_Old;
	int B_Old;

	int Active;

} T_Led_Controller;

extern T_Led_Controller Led_Controller;

extern void WriteColor(int ch, int r, int g, int b, int w);

#endif
