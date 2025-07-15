#include "W25Q64.h"
void W25Q64_Init(void)
{
	SPI_W25Q46_Init();
}
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID)
{
	SPI_W25Q46_CS(CS_START);
	SPI_W25Q46_SwapByte(W25Q64_JEDEC_ID);
	*MID = SPI_W25Q46_SwapByte(W25Q64_DUMMY_BYTE);
	*DID = SPI_W25Q46_SwapByte(W25Q64_DUMMY_BYTE);
	*DID <<= 8;
	*DID |= SPI_W25Q46_SwapByte(W25Q64_DUMMY_BYTE);
	SPI_W25Q46_CS(CS_STOP);
}

void W25Q64_WriteEnable(void)
{
	SPI_W25Q46_CS(CS_START);
	SPI_W25Q46_SwapByte(W25Q64_WRITE_ENABLE);
	SPI_W25Q46_CS(CS_STOP);
}

void W25Q64_WaitBusy(void)
{
	uint32_t Timeout;
	SPI_W25Q46_CS(CS_START);
	SPI_W25Q46_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
	Timeout = 100000;
	while ((SPI_W25Q46_SwapByte(W25Q64_DUMMY_BYTE) & 0x01) == 0x01)
	{
		Timeout --;
		if (Timeout == 0)
		{
			break;
		}
	}
	SPI_W25Q46_CS(CS_STOP);
}

void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count)
{
	uint16_t i;
	
	W25Q64_WriteEnable();
	
	SPI_W25Q46_CS(CS_START);
	SPI_W25Q46_SwapByte(W25Q64_PAGE_PROGRAM);
	SPI_W25Q46_SwapByte(Address >> 16);
	SPI_W25Q46_SwapByte(Address >> 8);
	SPI_W25Q46_SwapByte(Address);
	for (i = 0; i < Count; i ++)
	{
		SPI_W25Q46_SwapByte(DataArray[i]);
	}
	SPI_W25Q46_CS(CS_STOP);
	
	W25Q64_WaitBusy();
}

void W25Q64_SectorErase(uint32_t Address)
{
	W25Q64_WriteEnable();
	
	SPI_W25Q46_CS(CS_START);
	SPI_W25Q46_SwapByte(W25Q64_SECTOR_ERASE_4KB);
	SPI_W25Q46_SwapByte(Address >> 16);
	SPI_W25Q46_SwapByte(Address >> 8);
	SPI_W25Q46_SwapByte(Address);
	SPI_W25Q46_CS(CS_STOP);
	
	W25Q64_WaitBusy();
}

void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count)
{
	uint32_t i;
	SPI_W25Q46_CS(CS_START);
	SPI_W25Q46_SwapByte(W25Q64_READ_DATA);
	SPI_W25Q46_SwapByte(Address >> 16);
	SPI_W25Q46_SwapByte(Address >> 8);
	SPI_W25Q46_SwapByte(Address);
	for (i = 0; i < Count; i ++)
	{
		DataArray[i] = SPI_W25Q46_SwapByte(W25Q64_DUMMY_BYTE);
	}
	SPI_W25Q46_CS(CS_STOP);
}


//从rom中获取初始参数
void W25Q64_ReadFloat(uint32_t AddressStart,float *DataArray,uint8_t Count)
{
	
	uint32_t i;
	SPI_W25Q46_CS(CS_START);
	SPI_W25Q46_SwapByte(W25Q64_READ_DATA);
	SPI_W25Q46_SwapByte(AddressStart >> 16);
	SPI_W25Q46_SwapByte(AddressStart >> 8);
	SPI_W25Q46_SwapByte(AddressStart);
	// 读取数据
  for (i = 0; i < Count; i++) {
			// 直接将读取到的字节写入到DataArray中对应的浮点数位置
			((uint8_t*)&DataArray[i])[0] = SPI_W25Q46_SwapByte(W25Q64_DUMMY_BYTE);
			((uint8_t*)&DataArray[i])[1] = SPI_W25Q46_SwapByte(W25Q64_DUMMY_BYTE);
			((uint8_t*)&DataArray[i])[2] = SPI_W25Q46_SwapByte(W25Q64_DUMMY_BYTE);
			((uint8_t*)&DataArray[i])[3] = SPI_W25Q46_SwapByte(W25Q64_DUMMY_BYTE);
   }
	SPI_W25Q46_CS(CS_STOP);
}
/*只能写入一个page的数据
因此需要Count<=64
*/
void W25Q64_WriteFloat(uint32_t PageStart,float *DataArray,uint8_t Count)
{
	W25Q64_SectorErase(PageStart);
	
	uint32_t i;
	W25Q64_WriteEnable();
	
	SPI_W25Q46_CS(CS_START);
	SPI_W25Q46_SwapByte(W25Q64_PAGE_PROGRAM);
	SPI_W25Q46_SwapByte(PageStart >> 16);
	SPI_W25Q46_SwapByte(PageStart >> 8);
	SPI_W25Q46_SwapByte(PageStart);
	// 写入数据
	for (i = 0; i < Count; i++) {
			// 将浮点数拆分为4个字节并写入
			SPI_W25Q46_SwapByte(((uint8_t*)&DataArray[i])[0]);
			SPI_W25Q46_SwapByte(((uint8_t*)&DataArray[i])[1]);
			SPI_W25Q46_SwapByte(((uint8_t*)&DataArray[i])[2]);
			SPI_W25Q46_SwapByte(((uint8_t*)&DataArray[i])[3]);
	}
	SPI_W25Q46_CS(CS_STOP);
}
void ParameterInit(float *p,uint8_t len)
{
	W25Q64_ReadFloat(P_ADDRESS,p,len);
}




