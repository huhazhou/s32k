/* LPSPI.c              (c) 2016 NXP 
 * Descriptions: S32K144 FlexCAN example functions.
 * May 31 2016 S. Mihalik: Initial version.
 * Oct 31 2016 SM - adjust PRESCALE for 40 MHz SPLLDIV2_CLK
 * Nov 02 2016 SM - cleared flags in transmit, receive functions
 */
#include "S32K144.h"           /* include peripheral declarations S32K144 */
#include <stdio.h>
#include "SPI_MSD0_Driver.h"

#define printf(...)

/* Private define ------------------------------------------------------------*/
//#define PRINT_INFO  1	
/* Private variables ---------------------------------------------------------*/
MSD_CARDINFO SD0_CardInfo;

/*******************************************************************************
 * Function Name  : MSD0_spi_read_write
 * Description    : None
 * Input          : - data:
 * Output         : None
 * Return         : None
 * Attention		 : None
 *******************************************************************************/
int MSD0_spi_read_write(unsigned char data) {
	unsigned char recieve = 0;

	while ((LPSPI0->SR & LPSPI_SR_TDF_MASK) >> LPSPI_SR_TDF_SHIFT == 0)
		; /* Wait for Tx FIFO available */
	LPSPI0->TDR = data; /* Transmit data */
	LPSPI0->SR |= LPSPI_SR_TDF_MASK; /* Clear TDF flag */

	while ((LPSPI0->SR & LPSPI_SR_RDF_MASK) >> LPSPI_SR_RDF_SHIFT == 0)
		; /* Wait at least one RxFIFO entry */
	recieve = LPSPI0->RDR; /* Read received data */
	LPSPI0->SR |= LPSPI_SR_RDF_MASK; /* Clear RDF flag */
	return recieve;

}
void writeclock(uint8_t cnt)
{
	uint8_t i;
	//CS_HIGH;
	for(i=0;i<cnt;i++)
		MSD0_spi_read_write(0xff);
	//CS_HIGH;
}
/*******************************************************************************
 * Function Name  : MSD0_SPI_Configuration
 * Description    : SD Card SPI Configuration
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention		 : None
 *******************************************************************************/
//void MSD0_SPI_Configuration(void) {
//
//	//�ر�Ƭѡ
//	MSD0_card_disable();
//
//	//����SPI�ӿ�
//	LPSPI1_init_master();
//
//}

/*******************************************************************************
 * Function Name  : MSD0_SPIHighSpeed
 * Description    : SD Card Speed Set
 * Input          : - b_high: 1 = 18MHz, 0 = 281.25Hz
 * Output         : None
 * Return         : None
 * Attention		 : None
 *******************************************************************************/
void MSD0_SPIHighSpeed(unsigned char b_high) {

	/* Speed select */
	if (b_high == 0) {
		printf("\r\nspi�л�������");
		LPSPI0->TCR &= 0xc7ffffff;
		LPSPI0->TCR |= 0x38000000;
	} else {
		printf("\r\nspi�л�������");
		LPSPI0_init_master();

	}

}

/*******************************************************************************
 * Function Name  : MSD0_Init
 * Description    : SD Card initializtion
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention		 : None
 *******************************************************************************/
int MSD0_Init(void) {
	unsigned char r1;
	unsigned char buff[6] = { 0 };
	unsigned short retry;

	/* Check , if no card insert */
	/*
	 if( MSD0_card_insert() )
	 {
	 #ifdef PRINT_INFO
	 printf("There is no card detected! \r\n");
	 #endif
	 / * FATFS error flag * /
	 return -1;
	 }
	 */
	/* Power on and delay some times */
	/*
	 for(retry=0; retry<0x100; retry++)
	 {
	 MSD0_card_power_on();
	 }
	 */
	//fa shi zhong gei sd ka ,jixing chu shi hua
	/* Satrt send 74 clocks at least */
	for (retry = 0; retry < 80; retry++) {
		MSD0_spi_read_write(DUMMY_BYTE);
	}
	//writeclock(10);
	MSD0_card_disable();
	/* Start send CMD0 till return 0x01 means in IDLE state */
	for (retry = 0; retry < 0xFFF; retry++) {
		r1 = MSD0_send_command(CMD0, 0, 0x95);//r1 hui fu shi 0x80bushi 0x01
		//MSD0_spi_read_write(r1);
		if (r1 == 0x01) {
			retry = 0;
			printf("\r\nCMD0 FAN HUI ZHENG QUE");
			break;
		}
	}
	printf("BIAO JI 1");
	/* Timeout return */
	if (retry == 0xFFF) {
		printf("\r\nReset card into IDLE state failed!\r\n");
		return 1;
	}
	printf("BIAO JI 2");
	/* Get the card type, version */
	r1 = MSD0_send_command_hold(CMD8, 0x1AA, 0x87);
	printf("\r\n����CMD8����");
	{
		char s[3];
		sprintf(s,"%x",r1);
		printf(s);
	}
	/* r1=0x05 -> V1.0 */
	if (r1 == 0x05) {
		SD0_CardInfo.CardType = CARDTYPE_SDV1;
		printf("CMD8 FAN HUI ZHENG QUE");
		/* End of CMD8, chip disable and dummy byte */
		MSD0_card_disable();
		MSD0_spi_read_write(DUMMY_BYTE);

		/* SD1.0/MMC start initialize */
		/* Send CMD55+ACMD41, No-response is a MMC card, otherwise is a SD1.0 card */
		printf("ZHUN BEI FA SONG CMD55");
		for (retry = 0; retry < 0xFFF; retry++) {
			r1 = MSD0_send_command(CMD55, 0, 0); /* should be return 0x01 */
			if (r1 != 0x01) {
#ifdef PRINT_INFO
				printf("Send CMD55 should return 0x01, response=0x%02x\r\n", r1);
#endif
				MSD0_spi_read_write(0xa7);
				return r1;
			}
			r1 = MSD0_send_command(ACMD41, 0, 0); /* should be return 0x00 */
			if (r1 == 0x00) {
				retry = 0;
				printf("jinruACMD41 QIE FAN HUI 0");
				break;
			}
			MSD0_spi_read_write(0x5a);
		}

		/* MMC card initialize start */
		if (retry == 0xFFF) {
			for (retry = 0; retry < 0xFFF; retry++) {
				r1 = MSD0_send_command(CMD1, 0, 0); /* should be return 0x00 */
				if (r1 == 0x00) {
					retry = 0;
					break;
				}
			}

			/* Timeout return */
			if (retry == 0xFFF) {
#ifdef PRINT_INFO
				printf("Send CMD1 should return 0x00, response=0x%02x\r\n", r1);
#endif
				return 2;
			}

			SD0_CardInfo.CardType = CARDTYPE_MMC;
#ifdef PRINT_INFO
			printf("Card Type                     : MMC\r\n");
#endif
		}
		/* SD1.0 card detected, print information */
#ifdef PRINT_INFO
		else
		{
			printf("Card Type                     : SD V1\r\n");
		}
#endif

		/* Set spi speed high */
		MSD0_SPIHighSpeed(1);

		/* CRC disable */
		r1 = MSD0_send_command(CMD59, 0, 0x01);
		if (r1 != 0x00) {
#ifdef PRINT_INFO
			printf("Send CMD59 should return 0x00, response=0x%02x\r\n", r1);
#endif
			return r1; /* response error, return r1 */
		}

		/* Set the block size */
		r1 = MSD0_send_command(CMD16, MSD_BLOCKSIZE, 0xFF);
		if (r1 != 0x00) {
#ifdef PRINT_INFO
			printf("Send CMD16 should return 0x00, response=0x%02x\r\n", r1);
#endif
			return r1; /* response error, return r1 */
		}
	}

	/* r1=0x01 -> V2.x, read OCR register, check version */
	else if (r1 == 0x01) {
		printf("\r\n����CMD8���ؽ��1");
		/* 4Bytes returned after CMD8 sent	*/
		buff[0] = MSD0_spi_read_write(DUMMY_BYTE); /* should be 0x00 */
		buff[1] = MSD0_spi_read_write(DUMMY_BYTE); /* should be 0x00 */
		buff[2] = MSD0_spi_read_write(DUMMY_BYTE); /* should be 0x01 */
		buff[3] = MSD0_spi_read_write(DUMMY_BYTE); /* should be 0xAA */

		/* End of CMD8, chip disable and dummy byte */
		MSD0_card_disable();
		MSD0_spi_read_write(DUMMY_BYTE);
		{
				char s[3];
				printf("\r\n �ж��ǵĽ������ѹ\r\n");
				sprintf(s,"%x",buff[2]);
				printf(s);
				sprintf(s,"%x",buff[3]);
				printf(s);

		}
		/* Check voltage range be 2.7-3.6V	*/
		if (buff[2] == 0x01 && buff[3] == 0xAA) {
			for (retry = 0; retry < 0xFFF; retry++) {
				r1 = MSD0_send_command(CMD55, 0, 0); /* should be return 0x01 */
				printf("\r\nSend CMD55 should return 0x01, ");
				{
					char s[3];
					sprintf(s,"%x",r1);
					printf(s);
				}
				if (r1 != 0x01) {


					printf("\r\nSend CMD55 should return 0x01, ");
					{
						char s[3];
						sprintf(s,"%x",r1);
						printf(s);
					}

					return r1;
				}

				r1 = MSD0_send_command(ACMD41, 0x40000000, 0); /* should be return 0x00 */
				printf("\r\nACMD41�����");
				{
					char s[3];
					sprintf(s,"%x",r1);
					printf(s);
				}
				if (r1 == 0x00) {
					printf("\r\nACMD41�������ȷ");
					{
						char s[3];
						sprintf(s,"%x",r1);
						printf(s);
					}
					retry = 0;
					break;
				}
			}

			/* Timeout return */
			if (retry == 0xFFF) {
#ifdef PRINT_INFO
				printf("Send ACMD41 should return 0x00, response=0x%02x\r\n", r1);
#endif
				return 3;
			}

			/* Read OCR by CMD58 */
			r1 = MSD0_send_command_hold(CMD58, 0, 0);
			printf("\r\n����CMD58");
			if (r1 != 0x00) {
#ifdef PRINT_INFO
				printf("Send CMD58 should return 0x00, response=0x%02x\r\n", r1);
#endif
				return r1; /* response error, return r1 */
			}

			buff[0] = MSD0_spi_read_write(DUMMY_BYTE);
			buff[1] = MSD0_spi_read_write(DUMMY_BYTE);
			buff[2] = MSD0_spi_read_write(DUMMY_BYTE);
			buff[3] = MSD0_spi_read_write(DUMMY_BYTE);

			/* End of CMD58, chip disable and dummy byte */
			MSD0_card_disable();
			MSD0_spi_read_write(DUMMY_BYTE);

			/* OCR -> CCS(bit30)  1: SDV2HC	 0: SDV2 */
			if (buff[0] & 0x40) {
				SD0_CardInfo.CardType = CARDTYPE_SDV2HC;

				printf("\r\nCard Type                     : SD V2HC\r\n");

			} else {
				SD0_CardInfo.CardType = CARDTYPE_SDV2;

				printf("Card Type                     : SD V2\r\n");

			}

			/* Set spi speed high */
			MSD0_SPIHighSpeed(1);
		}
	}
	printf("\n\nSD����ʼ�����");
	return 0;
}

/*******************************************************************************
 * Function Name  : MSD0_GetCardInfo
 * Description    : Get SD Card Information
 * Input          : None
 * Output         : None
 * Return         : 0��NO_ERR; TRUE: Error
 * Attention		 : None
 *******************************************************************************/
int MSD0_GetCardInfo(PMSD_CARDINFO SD0_CardInfo)
{
  unsigned char r1;
  unsigned char CSD_Tab[16];
  unsigned char CID_Tab[16];

  /* Send CMD9, Read CSD */
  r1 = MSD0_send_command(CMD9, 0, 0xFF);
  if(r1 != 0x00)
  {
    return r1;
  }

  if(MSD0_read_buffer(CSD_Tab, 16, RELEASE))
  {
	return 1;
  }

  /* Send CMD10, Read CID */
  r1 = MSD0_send_command(CMD10, 0, 0xFF);
  if(r1 != 0x00)
  {
    return r1;
  }

  if(MSD0_read_buffer(CID_Tab, 16, RELEASE))
  {
	return 2;
  }

  /* Byte 0 */
  SD0_CardInfo->CSD.CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
  SD0_CardInfo->CSD.SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
  SD0_CardInfo->CSD.Reserved1 = CSD_Tab[0] & 0x03;
  /* Byte 1 */
  SD0_CardInfo->CSD.TAAC = CSD_Tab[1] ;
  /* Byte 2 */
  SD0_CardInfo->CSD.NSAC = CSD_Tab[2];
  /* Byte 3 */
  SD0_CardInfo->CSD.MaxBusClkFrec = CSD_Tab[3];
  /* Byte 4 */
  SD0_CardInfo->CSD.CardComdClasses = CSD_Tab[4] << 4;
  /* Byte 5 */
  SD0_CardInfo->CSD.CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
  SD0_CardInfo->CSD.RdBlockLen = CSD_Tab[5] & 0x0F;
  /* Byte 6 */
  SD0_CardInfo->CSD.PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
  SD0_CardInfo->CSD.WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
  SD0_CardInfo->CSD.RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
  SD0_CardInfo->CSD.DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
  SD0_CardInfo->CSD.Reserved2 = 0; /* Reserved */
  SD0_CardInfo->CSD.DeviceSize = (CSD_Tab[6] & 0x03) << 10;
  /* Byte 7 */
  SD0_CardInfo->CSD.DeviceSize |= (CSD_Tab[7]) << 2;
  /* Byte 8 */
  SD0_CardInfo->CSD.DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
  SD0_CardInfo->CSD.MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
  SD0_CardInfo->CSD.MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
  /* Byte 9 */
  SD0_CardInfo->CSD.MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
  SD0_CardInfo->CSD.MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
  SD0_CardInfo->CSD.DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
  /* Byte 10 */
  SD0_CardInfo->CSD.DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
  SD0_CardInfo->CSD.EraseGrSize = (CSD_Tab[10] & 0x7C) >> 2;
  SD0_CardInfo->CSD.EraseGrMul = (CSD_Tab[10] & 0x03) << 3;
  /* Byte 11 */
  SD0_CardInfo->CSD.EraseGrMul |= (CSD_Tab[11] & 0xE0) >> 5;
  SD0_CardInfo->CSD.WrProtectGrSize = (CSD_Tab[11] & 0x1F);
  /* Byte 12 */
  SD0_CardInfo->CSD.WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
  SD0_CardInfo->CSD.ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
  SD0_CardInfo->CSD.WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
  SD0_CardInfo->CSD.MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
  /* Byte 13 */
  SD0_CardInfo->CSD.MaxWrBlockLen |= (CSD_Tab[13] & 0xc0) >> 6;
  SD0_CardInfo->CSD.WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
  SD0_CardInfo->CSD.Reserved3 = 0;
  SD0_CardInfo->CSD.ContentProtectAppli = (CSD_Tab[13] & 0x01);
  /* Byte 14 */
  SD0_CardInfo->CSD.FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
  SD0_CardInfo->CSD.CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
  SD0_CardInfo->CSD.PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
  SD0_CardInfo->CSD.TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
  SD0_CardInfo->CSD.FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
  SD0_CardInfo->CSD.ECC = (CSD_Tab[14] & 0x03);
  /* Byte 15 */
  SD0_CardInfo->CSD.CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
  SD0_CardInfo->CSD.Reserved4 = 1;

  if(SD0_CardInfo->CardType == CARDTYPE_SDV2HC)
  {
	 /* Byte 7 */
	 SD0_CardInfo->CSD.DeviceSize = (unsigned short)(CSD_Tab[8]) *256;
	 /* Byte 8 */
	 SD0_CardInfo->CSD.DeviceSize += CSD_Tab[9] ;
  }

  SD0_CardInfo->Capacity = SD0_CardInfo->CSD.DeviceSize * MSD_BLOCKSIZE * 1024;
  SD0_CardInfo->BlockSize = MSD_BLOCKSIZE;

  /* Byte 0 */
  SD0_CardInfo->CID.ManufacturerID = CID_Tab[0];
  /* Byte 1 */
  SD0_CardInfo->CID.OEM_AppliID = CID_Tab[1] << 8;
  /* Byte 2 */
  SD0_CardInfo->CID.OEM_AppliID |= CID_Tab[2];
  /* Byte 3 */
  SD0_CardInfo->CID.ProdName1 = CID_Tab[3] << 24;
  /* Byte 4 */
  SD0_CardInfo->CID.ProdName1 |= CID_Tab[4] << 16;
  /* Byte 5 */
  SD0_CardInfo->CID.ProdName1 |= CID_Tab[5] << 8;
  /* Byte 6 */
  SD0_CardInfo->CID.ProdName1 |= CID_Tab[6];
  /* Byte 7 */
  SD0_CardInfo->CID.ProdName2 = CID_Tab[7];
  /* Byte 8 */
  SD0_CardInfo->CID.ProdRev = CID_Tab[8];
  /* Byte 9 */
  SD0_CardInfo->CID.ProdSN = CID_Tab[9] << 24;
  /* Byte 10 */
  SD0_CardInfo->CID.ProdSN |= CID_Tab[10] << 16;
  /* Byte 11 */
  SD0_CardInfo->CID.ProdSN |= CID_Tab[11] << 8;
  /* Byte 12 */
  SD0_CardInfo->CID.ProdSN |= CID_Tab[12];
  /* Byte 13 */
  SD0_CardInfo->CID.Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
  /* Byte 14 */
  SD0_CardInfo->CID.ManufactDate = (CID_Tab[13] & 0x0F) << 8;
  /* Byte 15 */
  SD0_CardInfo->CID.ManufactDate |= CID_Tab[14];
  /* Byte 16 */
  SD0_CardInfo->CID.CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
  SD0_CardInfo->CID.Reserved2 = 1;

  return 0;
}

/*******************************************************************************
 * Function Name  : MSD0_read_buffer
 * Description    : None
 * Input          : - *buff:
 *				   - len:
 *				   - release:
 * Output         : None
 * Return         : 0��NO_ERR; TRUE: Error
 * Attention		 : None
 *******************************************************************************/
int MSD0_read_buffer(unsigned char *buff, unsigned short len, unsigned char release)
{
  unsigned char r1;
  register unsigned short retry;

  /* Card enable, Prepare to read	*/
  MSD0_card_enable();

  /* Wait start-token 0xFE */
  for(retry=0; retry<2000; retry++)
  {
	 r1 = MSD0_spi_read_write(DUMMY_BYTE);
	 if(r1 == 0xFE)
	 {
		 retry = 0;
		 break;
	 }
  }

  /* Timeout return	*/
  if(retry == 2000)
  {
	 MSD0_card_disable();
	 return 1;
  }

  /* Start reading */
  for(retry=0; retry<len; retry++)
  {
     *(buff+retry) = MSD0_spi_read_write(DUMMY_BYTE);
  }

  /* 2bytes dummy CRC */
  MSD0_spi_read_write(DUMMY_BYTE);
  MSD0_spi_read_write(DUMMY_BYTE);

  /* chip disable and dummy byte */
  if(release)
  {
	 MSD0_card_disable();
	 MSD0_spi_read_write(DUMMY_BYTE);
  }

  return 0;
}
/*******************************************************************************
 * Function Name  : MSD0_ReadSingleBlock
 * Description    : None
 * Input          : - sector:
 *				   - buffer:
 * Output         : None
 * Return         : None
 * Attention		 : None
 *******************************************************************************/
int MSD0_ReadSingleBlock(unsigned int sector, unsigned char *buffer)
{
  unsigned char r1;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(SD0_CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }

  /* Send CMD17 : Read single block command */
  r1 = MSD0_send_command(CMD17, sector, 0);

  if(r1 != 0x00)
  {
	 return 1;
  }

  /* Start read and return the result */
  r1 = MSD0_read_buffer(buffer, MSD_BLOCKSIZE, RELEASE);

  /* Send stop data transmit command - CMD12 */
  MSD0_send_command(CMD12, 0, 0);

  return r1;
}


/*******************************************************************************
 * Function Name  : MSD0_ReadMultiBlock
 * Description    : None
 * Input          : - sector:
 *				   - buffer:
 *                  - NbrOfSector:
 * Output         : None
 * Return         : None
 * Attention		 : None
 *******************************************************************************/
int MSD0_ReadMultiBlock(unsigned int sector, unsigned char *buffer, unsigned int NbrOfSector)
{
  unsigned char r1;
  register unsigned int i;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(SD0_CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }

  /* Send CMD18 : Read multi block command */
  r1 = MSD0_send_command(CMD18, sector, 0);
  if(r1 != 0x00)
  {
     return 1;
  }

  /* Start read	*/
  for(i=0; i<NbrOfSector; i++)
  {
     if(MSD0_read_buffer(buffer+i*MSD_BLOCKSIZE, MSD_BLOCKSIZE, HOLD))
     {
		 /* Send stop data transmit command - CMD12	*/
		 MSD0_send_command(CMD12, 0, 0);
		 /* chip disable and dummy byte */
		 MSD0_card_disable();
		 return 2;
     }
  }

  /* Send stop data transmit command - CMD12 */
  MSD0_send_command(CMD12, 0, 0);

  /* chip disable and dummy byte */
  MSD0_card_disable();
  MSD0_spi_read_write(DUMMY_BYTE);

  return 0;
}

/*******************************************************************************
 * Function Name  : MSD0_WriteSingleBlock
 * Description    : None
 * Input          : - sector:
 *				   - buffer:
 * Output         : None
 * Return         : None
 * Attention		 : None
 *******************************************************************************/
int MSD0_WriteSingleBlock(unsigned int sector, unsigned char *buffer)
{
  unsigned char r1;
  register unsigned short i;
  unsigned int retry;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(SD0_CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	  {
		  char buf[3];
		  printf("\r\n SD");
		  sprintf(buf,"%x",SD0_CardInfo.CardType);
		  printf(buf);
	  }
	  sector = sector<<9;
  }

  /* Send CMD24 : Write single block command */
  r1 = MSD0_send_command(CMD24, sector, 0);
  {
	  char buf[3];
	  printf("\r\n Send CMD24 : Write single block command");
	  sprintf(buf,"%x",r1);
	  printf(buf);
  }
  if(r1 != 0x00)
  {
	 return 1;
  }

  /* Card enable, Prepare to write */
  MSD0_card_enable();
  MSD0_spi_read_write(DUMMY_BYTE);
  MSD0_spi_read_write(DUMMY_BYTE);
  MSD0_spi_read_write(DUMMY_BYTE);
  /* Start data write token: 0xFE */
  MSD0_spi_read_write(0xFE);

  /* Start single block write the data buffer */
  for(i=0; i<MSD_BLOCKSIZE; i++)
  {
    MSD0_spi_read_write(*buffer++);
  }

  /* 2Bytes dummy CRC */
  MSD0_spi_read_write(DUMMY_BYTE);
  MSD0_spi_read_write(DUMMY_BYTE);

  /* MSD card accept the data */
  r1 = MSD0_spi_read_write(DUMMY_BYTE);
  if((r1&0x1F) != 0x05)
  {
    MSD0_card_disable();
    return 2;
  }

  /* Wait all the data programm finished */
  retry = 0;
  while(MSD0_spi_read_write(DUMMY_BYTE) == 0x00)
  {
	 /* Timeout return */
	 if(retry++ == 0x40000)
	 {
	    MSD0_card_disable();
	    return 3;
	 }
  }

  /* chip disable and dummy byte */
  MSD0_card_disable();
  MSD0_spi_read_write(DUMMY_BYTE);

  return 0;
}

/*******************************************************************************
 * Function Name  : MSD0_WriteMultiBlock
 * Description    : None
 * Input          : - sector:
 *				   - buffer:
 *                  - NbrOfSector:
 * Output         : None
 * Return         : None
 * Attention		 : None
 *******************************************************************************/
int MSD0_WriteMultiBlock(unsigned int sector, unsigned char *buffer, unsigned int NbrOfSector)
{
  unsigned char r1;
  register unsigned short i;
  register unsigned int n;
  unsigned int retry;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(SD0_CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	  sector = sector<<9;
  }

  /* Send command ACMD23 berfore multi write if is not a MMC card */
  if(SD0_CardInfo.CardType != CARDTYPE_MMC)
  {
	  MSD0_send_command(ACMD23, NbrOfSector, 0x00);
  }

  /* Send CMD25 : Write nulti block command	*/
  r1 = MSD0_send_command(CMD25, sector, 0);

  if(r1 != 0x00)
  {
	  return 1;
  }

  /* Card enable, Prepare to write */
  MSD0_card_enable();
  MSD0_spi_read_write(DUMMY_BYTE);
  //MSD0_spi_read_write(DUMMY_BYTE);
  //MSD0_spi_read_write(DUMMY_BYTE);

  for(n=0; n<NbrOfSector; n++)
  {
	 /* Start multi block write token: 0xFC */
	 MSD0_spi_read_write(0xFC);

	 for(i=0; i<MSD_BLOCKSIZE; i++)
	 {
		MSD0_spi_read_write(*buffer++);
	 }

	 /* 2Bytes dummy CRC */
	 MSD0_spi_read_write(DUMMY_BYTE);
	 MSD0_spi_read_write(DUMMY_BYTE);

	 /* MSD card accept the data */
	 r1 = MSD0_spi_read_write(DUMMY_BYTE);
	 if((r1&0x1F) != 0x05)
	 {
	    MSD0_card_disable();
	    return 2;
	 }

	 /* Wait all the data programm finished	*/
	 retry = 0;
	 while(MSD0_spi_read_write(DUMMY_BYTE) != 0xFF)
	 {
		/* Timeout return */
		if(retry++ == 0x40000)
		{
		   MSD0_card_disable();
		   return 3;
		}
	 }
  }

  /* Send end of transmit token: 0xFD */
  r1 = MSD0_spi_read_write(0xFD);
  if(r1 == 0x00)
  {
	 return 4;
  }

  /* Wait all the data programm finished */
  retry = 0;
  while(MSD0_spi_read_write(DUMMY_BYTE) != 0xFF)
  {
	 /* Timeout return */
	 if(retry++ == 0x40000)
	 {
	     MSD0_card_disable();
	     return 5;
	 }
  }

  /* chip disable and dummy byte */
  MSD0_card_disable();
  MSD0_spi_read_write(DUMMY_BYTE);

  return 0;
}

/*******************************************************************************
 * Function Name  : MSD0_send_command
 * Description    : None
 * Input          : - cmd:
 *				   - arg:
 *                  - crc:
 * Output         : None
 * Return         : R1 value, response from card
 * Attention		 : None
 *******************************************************************************/
int MSD0_send_command(unsigned char cmd, unsigned int arg, unsigned char crc) {
	unsigned char r1;
	unsigned char retry;

	/* Dummy byte and chip enable */
	//CS_LOW;
	MSD0_spi_read_write(DUMMY_BYTE);
	//CS_HIGH;
	MSD0_card_enable();

	/* Command, argument and crc */
	//CS_LOW;
	MSD0_spi_read_write(cmd | 0x40);
	MSD0_spi_read_write(arg >> 24);
	MSD0_spi_read_write(arg >> 16);
	MSD0_spi_read_write(arg >> 8);
	MSD0_spi_read_write(arg);
	MSD0_spi_read_write(crc);
	//CS_HIGH;
	/* Wait response, quit till timeout */
	for (retry = 0; retry < 200; retry++) {
		//CS_LOW;
		r1 = MSD0_spi_read_write(DUMMY_BYTE);
		//CS_HIGH;
		if (r1 != 0xFF) {
			//CS_LOW;
			//MSD0_spi_read_write(0xa5);
			//MSD0_spi_read_write(r1);

			//CS_HIGH;
			break;
		}
	}
	//MSD0_spi_read_write(0xa6);
	/* Chip disable and dummy byte */
	MSD0_card_disable();
	MSD0_spi_read_write(DUMMY_BYTE);

	return r1;
}

/*******************************************************************************
 * Function Name  : MSD0_send_command_hold
 * Description    : None
 * Input          : - cmd:
 *				   - arg:
 *                  - crc:
 * Output         : None
 * Return         : R1 value, response from card
 * Attention		 : None
 *******************************************************************************/
int MSD0_send_command_hold(unsigned char cmd, unsigned int arg,
		unsigned char crc) {
	unsigned char r1;
	unsigned char retry;

	/* Dummy byte and chip enable */
	MSD0_spi_read_write(DUMMY_BYTE);
	MSD0_card_enable();

	/* Command, argument and crc */
	MSD0_spi_read_write(cmd | 0x40);
	MSD0_spi_read_write(arg >> 24);
	MSD0_spi_read_write(arg >> 16);
	MSD0_spi_read_write(arg >> 8);
	MSD0_spi_read_write(arg);
	MSD0_spi_read_write(crc);

	/* Wait response, quit till timeout */
	for (retry = 0; retry < 200; retry++) {
		r1 = MSD0_spi_read_write(DUMMY_BYTE);
		if (r1 != 0xFF) {
			break;
		}
	}

	return r1;
}

void LPSPI0_init_master(void) {
	PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTB */
	PORTE->PCR[0] |= PORT_PCR_MUX(2); /* Port B14: MUX = ALT3, LPSPI1_SCK */
	PORTE->PCR[1] |= PORT_PCR_MUX(2); /* Port B15: MUX = ALT3, LPSPI1_SIN */
	PORTE->PCR[2] |= PORT_PCR_MUX(2); /* Port B16: MUX = ALT3, LPSPI1_SOUT */
	PORTE->PCR[6] |= PORT_PCR_MUX(2); /* Port B17: MUX = ALT3, LPSPI1_PCS3 */

	//PTE->PDDR |= 1<<6; /* Port D0: Data Direction= output */
	//PORTE->PCR[6] = 0x00000100; /* Port D0: MUX = GPIO */

	PCC->PCCn[PCC_LPSPI0_INDEX] = 0; /* Disable clocks to modify PCS ( default) */
	PCC->PCCn[PCC_LPSPI0_INDEX] = 0xC6000000; /* Enable PCS=SPLL_DIV2 (40 MHz func'l clock) */
	LPSPI0->CR = 0x00000000; /* Disable module for configuration */
	LPSPI0->IER = 0x00000000; /* Interrupts not used */
	LPSPI0->DER = 0x00000000; /* DMA not used */
	LPSPI0->CFGR0 = 0x00000000; /* Defaults: */
	/* RDM0=0: rec'd data to FIFO as normal */
	/* CIRFIFO=0; Circular FIFO is disabled */
	/* HRSEL, HRPOL, HREN=0: Host request disabled */
	LPSPI0->CFGR1 = 0x00000001; /* Configurations: master mode*/
	/* PCSCFG=0: PCS[3:2] are enabled */
	/* OUTCFG=0: Output data retains last value when CS negated */
	/* PINCFG=0: SIN is input, SOUT is output */
	/* MATCFG=0: Match disabled */
	/* PCSPOL=0: PCS is active low */
	/* NOSTALL=0: Stall if Tx FIFO empty or Rx FIFO full */
	/* AUTOPCS=0: does not apply for master mode */
	/* SAMPLE=0: input data sampled on SCK edge */
	/* MASTER=1: Master mode */
	LPSPI0->TCR = 0xd2000007; /*zi ji kong zhi cs*/
	/* Transmit cmd: PCS3, 16 bits, prescale func'l clk by 4, etc*/
	/* CPOL=0: SCK inactive state is low */
	/* CPHA=1: Change data on SCK lead'g, capture on trail'g edge*/
	/* PRESCALE=128: Functional clock divided by 128**2 = 256 */
	/* PCS=3: Transfer using PCS3 */
	/* LSBF=0: Data is transfered MSB first */
	/* BYSW=0: Byte swap disabled */
	/* CONT, CONTC=0: Continuous transfer disabled */
	/* RXMSK=0: Normal transfer: rx data stored in rx FIFO */
	/* TXMSK=0: Normal transfer: data loaded from tx FIFO */
	/* WIDTH=0: Single bit transfer */
	/* FRAMESZ=15: # bits in frame = 15+1=16 */
	LPSPI0->CCR = 0x04090808; /* Clock dividers based on prescaled func'l clk of 100 nsec */
	/* SCKPCS=4: SCK to PCS delay = 4+1 = 5 (500 nsec) */
	/* PCSSCK=4: PCS to SCK delay = 9+1 = 10 (1 usec) */
	/* DBT=8: Delay between Transfers = 8+2 = 10 (1 usec) */
	/* SCKDIV=8: SCK divider =8+2 = 10 (1 usec: 1 MHz baud rate) */
	LPSPI0->FCR = 0x00000003; /* RXWATER=0: Rx flags set when Rx FIFO >0 */
	/* TXWATER=3: Tx flags set when Tx FIFO <= 3 */
	LPSPI0->CR = 0x00000009; /* Enable module for operation */
	/* DBGEN=1: module enabled in debug mode */
	/* DOZEN=0: module enabled in Doze mode */
	/* RST=0: Master logic not reset */
	/* MEN=1: Module is enabled */
}

void LPSPI0_transmit_8bits(uint8_t send) {
	while ((LPSPI0->SR & LPSPI_SR_TDF_MASK) >> LPSPI_SR_TDF_SHIFT == 0)
		;
	/* Wait for Tx FIFO available */
	LPSPI0->TDR = send; /* Transmit data */
	LPSPI0->SR |= LPSPI_SR_TDF_MASK; /* Clear TDF flag */
}
uint8_t LPSPI0_receive_8bits(void) {
	uint8_t receive = 0;
	while ((LPSPI0->SR & LPSPI_SR_RDF_MASK) >> LPSPI_SR_RDF_SHIFT == 0)
		;
	/* Wait at least one RxFIFO entry */
	receive = LPSPI0->RDR; /* Read received data */
	LPSPI0->SR |= LPSPI_SR_RDF_MASK; /* Clear RDF flag */
	return receive; /* Return received data */
}
void LPSPI1_transmit_16bits(uint16_t send) {
	while ((LPSPI0->SR & LPSPI_SR_TDF_MASK) >> LPSPI_SR_TDF_SHIFT == 0)
		;
	/* Wait for Tx FIFO available */
	LPSPI0->TDR = send; /* Transmit data */
	LPSPI0->SR |= LPSPI_SR_TDF_MASK; /* Clear TDF flag */
}
uint16_t LPSPI1_receive_16bits(void) {
	uint16_t receive = 0;
	while ((LPSPI0->SR & LPSPI_SR_RDF_MASK) >> LPSPI_SR_RDF_SHIFT == 0)
		;
	/* Wait at least one RxFIFO entry */
	receive = LPSPI0->RDR; /* Read received data */
	LPSPI0->SR |= LPSPI_SR_RDF_MASK; /* Clear RDF flag */
	return receive; /* Return received data */
}
