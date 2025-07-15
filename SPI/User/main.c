/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "W25Q64.h"
#define P_LEN 5
uint8_t MID;
uint16_t DID;
uint8_t ArrayWrite[] = {0x01, 0x02, 0x03, 0x04};
uint8_t ArrayRead[4];
//SSIConfigSetExpClk
/* Main ----------------------------------------------------------------------*/
//ram 中的调试参数(欲从ROM中获取)
float p[P_LEN]={0.1592,3.1,55.33,161.3,2.666};
float p_get[P_LEN];

int main(void)
{
	sys_init();
	W25Q64_Init();
//	ParameterInit(p,P_LEN);
	SSI0_Init();
	W25Q64_ReadID(&MID, &DID);
	//检验spi配置是否正确
//	W25Q64_SectorErase(0x000000);
//	W25Q64_PageProgram(0x000000, ArrayWrite, 4);
//	W25Q64_ReadData(0x000000, ArrayRead, 4);
	
	MAP_uDMAChannelEnable(UDMA_CH11_SSI0TX);
	MAP_SSIDMAEnable(SSI0_BASE,SSI_DMA_TX);
	W25Q64_WriteFloat(P_ADDRESS,p,P_LEN);
	W25Q64_ReadFloat(0x000000,p_get,5);
	while(1)
	{
		
	}
}


