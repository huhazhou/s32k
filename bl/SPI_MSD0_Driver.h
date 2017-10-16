
#ifndef __SPI_MSD0_DRIVER_H
#define __SPI_MSD0_DRIVER_H

/* Includes ------------------------------------------------------------------*/
#include "S32K144.h"  


/* Private define ------------------------------------------------------------*/
#define CARDTYPE_MMC     	     0x00
#define CARDTYPE_SDV1      	     0x01
#define CARDTYPE_SDV2      	     0x02
#define CARDTYPE_SDV2HC    	     0x04

#define DUMMY_BYTE				 0xFF 
#define MSD_BLOCKSIZE			 512

/* SD/MMC command list - SPI mode */
#define CMD0                     0       /* Reset */
#define CMD1                     1       /* Send Operator Condition - SEND_OP_COND */
#define CMD8                     8       /* Send Interface Condition - SEND_IF_COND	*/
#define CMD9                     9       /* Read CSD */
#define CMD10                    10      /* Read CID */
#define CMD12                    12      /* Stop data transmit */
#define CMD16                    16      /* Set block size, should return 0x00 */ 
#define CMD17                    17      /* Read single block */
#define CMD18                    18      /* Read multi block */
#define ACMD23                   23      /* Prepare erase N-blokcs before multi block write */
#define CMD24                    24      /* Write single block */
#define CMD25                    25      /* Write multi block */
#define ACMD41                   41      /* should return 0x00 */
#define CMD55                    55      /* should return 0x01 */
#define CMD58                    58      /* Read OCR */
#define CMD59                    59      /* CRC disable/enbale, should return 0x00 */

/* Physical level marcos */
/*
#define MSD0_card_enable()      	GPIOA->BRR = GPIO_Pin_4//GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define MSD0_card_disable()     	GPIOA->BSRR = GPIO_Pin_4//GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define MSD0_card_power_on()
#define MSD0_card_insert()       	GPIOB->IDR & GPIO_Pin_0//GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)
*/
#define MSD0_card_enable()             ;//GPIO_HAL_ClearPins(PTB,1<<17);
#define MSD0_card_disable()             ;//GPIO_HAL_SetPins(PTB,1<<17);
#define MSD0_card_power_on()            ;
#define MSD0_card_insert()              0//SD��������
//
//
/* Private typedef -----------------------------------------------------------*/
enum _CD_HOLD
{
	HOLD = 0,
	RELEASE = 1,
};

typedef struct               /* Card Specific Data */
{
  volatile unsigned char  CSDStruct;            /* CSD structure */
  volatile unsigned char  SysSpecVersion;       /* System specification version */
  volatile unsigned char  Reserved1;            /* Reserved */
  volatile unsigned char  TAAC;                 /* Data read access-time 1 */
  volatile unsigned char  NSAC;                 /* Data read access-time 2 in CLK cycles */
  volatile unsigned char  MaxBusClkFrec;        /* Max. bus clock frequency */
  volatile unsigned short CardComdClasses;      /* Card command classes */
  volatile unsigned char  RdBlockLen;           /* Max. read data block length */
  volatile unsigned char  PartBlockRead;        /* Partial blocks for read allowed */
  volatile unsigned char  WrBlockMisalign;      /* Write block misalignment */
  volatile unsigned char  RdBlockMisalign;      /* Read block misalignment */
  volatile unsigned char  DSRImpl;              /* DSR implemented */
  volatile unsigned char  Reserved2;            /* Reserved */
  volatile unsigned int DeviceSize;           /* Device Size */
  volatile unsigned char  MaxRdCurrentVDDMin;   /* Max. read current @ VDD min */
  volatile unsigned char  MaxRdCurrentVDDMax;   /* Max. read current @ VDD max */
  volatile unsigned char  MaxWrCurrentVDDMin;   /* Max. write current @ VDD min */
  volatile unsigned char  MaxWrCurrentVDDMax;   /* Max. write current @ VDD max */
  volatile unsigned char  DeviceSizeMul;        /* Device size multiplier */
  volatile unsigned char  EraseGrSize;          /* Erase group size */
  volatile unsigned char  EraseGrMul;           /* Erase group size multiplier */
  volatile unsigned char  WrProtectGrSize;      /* Write protect group size */
  volatile unsigned char  WrProtectGrEnable;    /* Write protect group enable */
  volatile unsigned char  ManDeflECC;           /* Manufacturer default ECC */
  volatile unsigned char  WrSpeedFact;          /* Write speed factor */
  volatile unsigned char  MaxWrBlockLen;        /* Max. write data block length */
  volatile unsigned char  WriteBlockPaPartial;  /* Partial blocks for write allowed */
  volatile unsigned char  Reserved3;            /* Reserded */
  volatile unsigned char  ContentProtectAppli;  /* Content protection application */
  volatile unsigned char  FileFormatGrouop;     /* File format group */
  volatile unsigned char  CopyFlag;             /* Copy flag (OTP) */
  volatile unsigned char  PermWrProtect;        /* Permanent write protection */
  volatile unsigned char  TempWrProtect;        /* Temporary write protection */
  volatile unsigned char  FileFormat;           /* File Format */
  volatile unsigned char  ECC;                  /* ECC code */
  volatile unsigned char  CSD_CRC;              /* CSD CRC */
  volatile unsigned char  Reserved4;            /* always 1*/
}
MSD_CSD;

typedef struct				 /*Card Identification Data*/
{
  volatile unsigned char  ManufacturerID;       /* ManufacturerID */
  volatile unsigned short OEM_AppliID;          /* OEM/Application ID */
  volatile unsigned int ProdName1;            /* Product Name part1 */
  volatile unsigned char  ProdName2;            /* Product Name part2*/
  volatile unsigned char  ProdRev;              /* Product Revision */
  volatile unsigned int ProdSN;               /* Product Serial Number */
  volatile unsigned char  Reserved1;            /* Reserved1 */
  volatile unsigned short ManufactDate;         /* Manufacturing Date */
  volatile unsigned char  CID_CRC;              /* CID CRC */
  volatile unsigned char  Reserved2;            /* always 1 */
}
MSD_CID;

typedef struct
{
  MSD_CSD CSD;
  MSD_CID CID;
  unsigned int Capacity;              /* Card Capacity */
  unsigned int BlockSize;             /* Card Block Size */
  unsigned short RCA;
  unsigned char CardType;
  unsigned int SpaceTotal;            /* Total space size in file system */
  unsigned int SpaceFree;      	     /* Free space size in file system */
}
MSD_CARDINFO, *PMSD_CARDINFO;


/* Private function prototypes -----------------------------------------------*/

extern int MSD0_Init(void);
extern int MSD0_GetCardInfo(PMSD_CARDINFO cardinfo);
extern int MSD0_ReadSingleBlock(unsigned int sector, unsigned char *buffer);
extern int MSD0_ReadMultiBlock(unsigned int sector, unsigned char *buffer, unsigned int NbrOfSector);
extern int MSD0_WriteSingleBlock(unsigned int sector, unsigned char *buffer);
extern int MSD0_WriteMultiBlock(unsigned int sector, unsigned char *buffer, unsigned int NbrOfSector);

extern void MSD0_SPI_Configuration(void);
extern void MSD0_SPIHighSpeed(unsigned char b_high);

extern int MSD0_spi_read_write(unsigned char data);
extern int MSD0_send_command(unsigned char cmd, unsigned int arg, unsigned char crc);
extern int MSD0_send_command_hold(unsigned char cmd, unsigned int arg, unsigned char crc);
extern int MSD0_read_buffer(unsigned char *buff, unsigned short len, unsigned char release);

extern void LPSPI0_init_master(void) ;
extern void LPSPI0_transmit_8bits(uint8_t send);
extern uint8_t LPSPI0_receive_8bits(void) ;
extern void LPSPI1_transmit_16bits(uint16_t send) ;
extern uint16_t LPSPI1_receive_16bits(void) ;



#endif

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

