#include<unistd.h>
#include<dlfcn.h>
#include "ftd2xx.h"
#include "libMPSSE_i2c.h"
#include <string.h>
#include "leds.h"

/******************************************************************************/
/*								Macro and type defines							   */
/******************************************************************************/
#define GET_FUN_POINTER	dlsym
/*#define CHECK_ERROR(exp) {if(dlerror() != NULL){printf("line %d: ERROR dlsym\n",__LINE__);}}

#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
	!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};
#define APP_CHECK_STATUS_NOEXIT(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
	!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);}else{;}};
#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
	encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};*/
	
typedef FT_STATUS(*pfunc_I2C_GetNumChannels)(uint32 *numChannels);
typedef FT_STATUS(*pfunc_I2C_GetChannelInfo)(uint32 index, FT_DEVICE_LIST_INFO_NODE *chanInfo);
typedef FT_STATUS(*pfunc_I2C_OpenChannel)(uint32 index, FT_HANDLE *handle);
typedef FT_STATUS(*pfunc_I2C_CloseChannel)(FT_HANDLE handle);
typedef FT_STATUS(*pfunc_I2C_InitChannel)(FT_HANDLE handle, ChannelConfig *config);
typedef FT_STATUS(*pfunc_I2C_DeviceRead)(FT_HANDLE handle, uint32 deviceAddress, uint32 sizeToTransfer, uint8 *buffer, uint32 *sizeTransfered, uint32 options);
typedef FT_STATUS(*pfunc_I2C_DeviceWrite)(FT_HANDLE handle, uint32 deviceAddress, uint32 sizeToTransfer, uint8 *buffer, uint32 *sizeTransfered, uint32 options);
typedef FT_STATUS(*pfunc_FT_WriteGPIO)(FT_HANDLE handle, uint8 dir, uint8 value);
typedef FT_STATUS(*pfunc_FT_ReadGPIO)(FT_HANDLE handle, uint8 *value);
static pfunc_I2C_GetNumChannels p_I2C_GetNumChannels = NULL;
static pfunc_I2C_GetChannelInfo p_I2C_GetChannelInfo = NULL;
static pfunc_I2C_OpenChannel p_I2C_OpenChannel = NULL;
static pfunc_I2C_CloseChannel p_I2C_CloseChannel = NULL;
static pfunc_I2C_InitChannel p_I2C_InitChannel = NULL;
static pfunc_I2C_DeviceRead p_I2C_DeviceRead = NULL;
static pfunc_I2C_DeviceWrite p_I2C_DeviceWrite = NULL;
static pfunc_FT_WriteGPIO p_FT_WriteGPIO = NULL;
static pfunc_FT_ReadGPIO  p_FT_ReadGPIO = NULL;

static FT_HANDLE ftHandle;
static uint8 buffer[100] = {0};
static uint8 pca9685_regs[256] = {0};

T_Led_Controller Led_Controller;

uint8 Init_Led_Driver()
{
	void *h_libMPSSE;
	h_libMPSSE = dlopen("libMPSSE.so", RTLD_LAZY);
	
	if(!h_libMPSSE)
	{
		//printf("Failed loading libMPSSE.so. Please check if the file exists in the shared library folder(/usr/lib or /usr/lib64)\n");
		return 0;
	}

	p_I2C_GetNumChannels = (pfunc_I2C_GetNumChannels)GET_FUN_POINTER(h_libMPSSE, "I2C_GetNumChannels");
	p_I2C_GetChannelInfo = (pfunc_I2C_GetChannelInfo)GET_FUN_POINTER(h_libMPSSE, "I2C_GetChannelInfo");
	p_I2C_OpenChannel = (pfunc_I2C_OpenChannel)GET_FUN_POINTER(h_libMPSSE, "I2C_OpenChannel");
	p_I2C_CloseChannel = (pfunc_I2C_CloseChannel)GET_FUN_POINTER(h_libMPSSE, "I2C_CloseChannel");
	p_I2C_InitChannel = (pfunc_I2C_InitChannel)GET_FUN_POINTER(h_libMPSSE, "I2C_InitChannel");
	p_I2C_DeviceRead = (pfunc_I2C_DeviceRead)GET_FUN_POINTER(h_libMPSSE, "I2C_DeviceRead");
	p_I2C_DeviceWrite = (pfunc_I2C_DeviceWrite)GET_FUN_POINTER(h_libMPSSE, "I2C_DeviceWrite");
	p_FT_WriteGPIO = (pfunc_FT_WriteGPIO)GET_FUN_POINTER(h_libMPSSE, "FT_WriteGPIO");
	p_FT_ReadGPIO = (pfunc_FT_ReadGPIO)GET_FUN_POINTER(h_libMPSSE, "FT_ReadGPIO");

	/*CHECK_ERROR(p_I2C_GetNumChannels);
	CHECK_ERROR(p_I2C_GetChannelInfo);
	CHECK_ERROR(p_I2C_OpenChannel);
	CHECK_ERROR(p_I2C_CloseChannel);
	CHECK_ERROR(p_I2C_InitChannel);
	CHECK_ERROR(p_I2C_DeviceRead);
	CHECK_ERROR(p_I2C_DeviceWrite);
	CHECK_ERROR(p_FT_WriteGPIO);
	CHECK_ERROR(p_FT_ReadGPIO);*/

	memset(&Led_Controller, 0, sizeof(T_Led_Controller));

	Led_Controller.BaseChannel = base_ch;
	Led_Controller.Active = active;

	return 1;
}

void Write_I2C_Data(uint8 slaveAddress, uint8 registerAddress, const uint8 *data, uint32 numBytes)
{
	uint32 bytesToTransfer = 0;
	uint32 bytesTransfered = 0;
	uint32 options = 0;

	options = I2C_TRANSFER_OPTIONS_START_BIT|I2C_TRANSFER_OPTIONS_STOP_BIT|I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES;

	buffer[bytesToTransfer++] = registerAddress;
	memcpy(buffer + bytesToTransfer, data, numBytes);
	bytesToTransfer += numBytes;

	p_I2C_DeviceWrite(ftHandle, slaveAddress, bytesToTransfer, buffer, &bytesTransfered, options);
}

void Init_Led_Controller(int active, int base_ch, int mode)
{
	ChannelConfig channelConf;
	uint32 channels = 0;
	uint32 i = 0;
	
	memset(&Led_Controller, 0, sizeof(T_Led_Controller));
	
	if ( !active ) return;
	if (!Init_Led_Driver()) return;

	memset(&channelConf, 0, sizeof(channelConf));
	channelConf.ClockRate = I2C_CLOCK_FAST_MODE;
	channelConf.LatencyTimer = 50;

	if ( FT_OK != p_I2C_GetNumChannels(&channels) ) return;
	if ( FT_OK != p_I2C_OpenChannel(0, &ftHandle) ) return; 
	if ( FT_OK != p_I2C_InitChannel(ftHandle, &channelConf) ) return;
	
	Led_Controller.BaseChannel = base_ch;
	Led_Controller.Active = active;
	
	pca9685_regs[0] = 0x21;
	pca9685_regs[1] = 0x04;
	
	Write_I2C_Data(0x40, 0x00, &pca9685_regs[0], 1);
	Write_I2C_Data(0x40, 0x01, &pca9685_regs[1], 1);
}

void WriteColor(int ch, int r, int g, int b, int w)
{
	int reg_addr;
	int base_reg;
	base_reg = 6 + ch * 16;
	
	//rosso
	pca9685_regs[base_reg + 4 * 0 + 0] = 0x00;	
	pca9685_regs[base_reg + 4 * 0 + 1] = 0x00;
	pca9685_regs[base_reg + 4 * 0 + 2] = (r & 0x00FF);
	pca9685_regs[base_reg + 4 * 0 + 3] = (r & 0x0F00) >> 8;
	
	//green
	pca9685_regs[base_reg + 4 * 1 + 0] = 0x00;	
	pca9685_regs[base_reg + 4 * 1 + 1] = 0x00;
	pca9685_regs[base_reg + 4 * 1 + 2] = (g & 0x00FF);
	pca9685_regs[base_reg + 4 * 1 + 3] = (g & 0x0F00) >> 8;
	
	//blue
	pca9685_regs[base_reg + 4 * 2 + 0] = 0x00;	
	pca9685_regs[base_reg + 4 * 2 + 1] = 0x00;
	pca9685_regs[base_reg + 4 * 2 + 2] = (b & 0x00FF);
	pca9685_regs[base_reg + 4 * 2 + 3] = (b & 0x0F00) >> 8;
	
	//white
	pca9685_regs[base_reg + 4 * 3 + 0] = 0x00;	
	pca9685_regs[base_reg + 4 * 3 + 1] = 0x00;
	pca9685_regs[base_reg + 4 * 3 + 2] = (w & 0x00FF);
	pca9685_regs[base_reg + 4 * 3 + 3] = (w & 0x0F00) >> 8;

	Write_I2C_Data(0x40, base_reg, &pca9685_regs[base_reg], 16);
}

