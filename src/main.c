#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "S32K144.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "bl.h"
#include "ff.h"
#include "convert.h"

#define OS_ASSERT(v)											\
	if(pdTRUE != (v)){											\
		os_printf("fault in %s call: %s\n",__FUNCTION__,#v);	\
		for(;;);												\
	}

static SemaphoreHandle_t  prt_mutex;
void init_printf_mutex(void)
{
	if(prt_mutex==NULL){
		prt_mutex = xSemaphoreCreateMutex();
		xSemaphoreGive(prt_mutex);
	}
}
void os_printf(const char *fmt, ...)
{
	va_list va;
	va_start(va,fmt);
	xSemaphoreTake(prt_mutex,portMAX_DELAY);
	vprintf(fmt,va);
	xSemaphoreGive(prt_mutex);
	va_end(va);
}

void os_sleep_ms(int ms)
{
	vTaskDelay(ms);
}
void os_sleep()
{
	vTaskDelay(portMAX_DELAY);
}
int test_buzzer()
{
	for(int i=0;i<3;i++){
		buzzer_ctrl(1);
		os_printf("buzzer_on\n");
		os_sleep_ms(1000);
		buzzer_ctrl(0);
		os_printf("buzzer_off\n");
		os_sleep_ms(500);
	}
	return 0;
}
static int test_sd()
{
	FATFS fatfs;
	FRESULT res;

	FIL f;
	unsigned int flen;

	os_printf("mount filesystem\n");
	res = f_mount(0, &fatfs);
	if (res != FR_OK) {
		os_printf("mount filesystem 0 failed : %d\n", res);
		return -1;
	}

	os_printf("create test.txt\n");
	res = f_open(&f, "test.txt", FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
	if (res != FR_OK){
		os_printf("write error\n");
		return -1;
	}

	os_printf("write: \"test12345\"\n");
	f_lseek(&f, 0);
	res = f_write(&f, "test12345", 9, &flen);
	if (res != FR_OK){
		os_printf("write error\n");
		return -1;
	}


	os_printf("read and check file data\n");
	char buf[128];
	memset(buf,0,128);
	UINT rd;
	f_lseek(&f, 0);
	res = f_read(&f, buf, 9, &rd);
	if(res != FR_OK){
		os_printf("read error\n");
		return -1;
	}
	if(rd!=9){
		os_printf("read len error %d\n",rd);
		return -1;
	}
	if(strcmp(buf,"test12345")){
		os_printf("read data error %s\n",buf);
		return -1;
	}

	os_printf("close file\n");
	res = f_close(&f);
	if(res != FR_OK){
		os_printf("close error\n");
		return -1;
	}
	return 0;
}

struct CanParamSt{
	SemaphoreHandle_t mtx;
	int selfPort;
	int targetPort;
	TaskHandle_t handle;
	int rxFlag;
};
static void can_task_proc(void *p)
{
	struct CanParamSt *canSt = (struct CanParamSt*)p;
	int port = canSt->selfPort;
	int targetID = canSt->targetPort + 0xA;

	for (;;) {

		xSemaphoreTake(canSt->mtx,portMAX_DELAY);
		FLEXCAN_transmit_msg(port,targetID,8,(uint8_t*)"\xAA\x55\xAA\x55\xAA\x55\xAA\x55"); /* Transmit message using MB0 */
		xSemaphoreGive(canSt->mtx);

		uint32_t RxLENGTH;
		uint8_t RxDATA[8];
		xSemaphoreTake(canSt->mtx,portMAX_DELAY);
		int res = FLEXCAN_receive_msg(port, &RxLENGTH, RxDATA);
		xSemaphoreGive(canSt->mtx);
		if(res==0){
			char buf[32];
			os_printf("CAN%d get message :RxLENGTH=%X RxDATA=%s\n",port,RxLENGTH,hx_dumphex2str(RxDATA, 8, buf));
			canSt->rxFlag = 1;
		}else{

		}

		os_sleep_ms(1000);
	}
}
static int test_can()
{
	uint32_t test_rst = 0U;
	SemaphoreHandle_t  can_mutex;
	can_mutex = xSemaphoreCreateMutex();
	xSemaphoreGive(prt_mutex);

	for(int i=0;i<3;i++){
		int j = (i+1)%3;
		os_printf("CAN%d <-> CAN%d\n",i,j);
		struct CanParamSt canA = {
			.mtx = can_mutex,
			.selfPort = i,
			.targetPort = j,
			.rxFlag = 0,
		};
		OS_ASSERT(xTaskCreate(can_task_proc,"can_task_a", 1024/4, &canA , 5, &canA.handle));
		struct CanParamSt canB = {
			.mtx = can_mutex,
			.selfPort = j,
			.targetPort = i,
			.rxFlag = 0,
		};
		OS_ASSERT(xTaskCreate(can_task_proc,"can_task_b", 1024/4, &canB , 5, &canB.handle));

		for(int i=0;i<5;i++){
			if(canA.rxFlag && canB.rxFlag)
				break;
			os_sleep_ms(1000);
		}
		if(canA.rxFlag && canB.rxFlag){
			os_printf("PASS.\n",i,j);
			test_rst |= 1<<i;
		}
		if(canA.rxFlag==0)
			os_printf("CAN%d receive timeout, Test FAIL***\n",i);
		if(canB.rxFlag==0)
			os_printf("CAN%d receive timeout, Test FAIL***\n",j);

		vTaskDelete(canA.handle);
		vTaskDelete(canB.handle);
		os_sleep_ms(500);
	}

	if(test_rst == 0b111){
		os_printf("CAN test complete\n");
		return 0;
	}else{
		os_printf("CAN test has FAIL***\n");
		return -1;
	}
}
struct UartParamSt {
	char tr;
	int port;
	TaskHandle_t handle;
	int test_rst;
};

static xQueueHandle uartQueueTbl[3];

void UartRxIRQHandler(int port)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	uint8_t c = LPUART1->DATA;
	xQueueSendToBackFromISR(uartQueueTbl[port],&c,&xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken)
}
int LPUART_receive_from_queue(int port,int timeout){
	uint8_t c;
	if(xQueueReceive(uartQueueTbl[port], &c, timeout)==pdTRUE){
		return c;
	}else{
		return -1;
	}
}
void LPUART_receive_flush(int port)
{
	while(LPUART_receive_from_queue(port,1)!=-1);
}
char *LPUART_receive_string_timeout(int port,char *buf, int bufsize, int timeout)
{
	char *p = buf;
	char *end = buf+bufsize-1;
	while(p<end){
		int c = LPUART_receive_from_queue(port,timeout);
		if(c==-1)
			return NULL;	/*timeout*/
		if(c=='\n'){
			*p = '\0';
			return buf;		/*get*/
		}
		*p++ = c;
	}
	*p = '\0';
	return buf;		/*full*/
}
char *LPUART_receive_string(int port,char *buf, int bufsize)
{
	return LPUART_receive_string_timeout(port,buf,bufsize,portMAX_DELAY);
}
void LPUART_transmit_string(int port,const char *data_string)  {  /* Function to Transmit whole string */
  uint32_t i=0;
  LPUART_Type* uartTbl[3] = {LPUART0,LPUART1,LPUART2};
  while(data_string[i] != '\0')  {           /* Send chars one at a time */
    LPUART_transmit_char(uartTbl[port],data_string[i]);
    i++;
  }
}

static void uart_proc(void *p)
{
	struct UartParamSt* param = (struct UartParamSt*)p;
	if(param->tr=='t'){
		for(int i=0;i<5;i++){
			os_printf("UART%d send message\n",param->port);
			LPUART_transmit_string(param->port, "$$TEST$$\n");
			os_sleep_ms(1000);
		}
	}else{
		char buf[64];
		LPUART_receive_string(param->port, buf, 64);
		os_printf("UART%d get message: %s",param->port,buf);
		param->test_rst = 1;
		os_sleep();
	}

}

static int test_uart()
{
	uint32_t testFlag;
	uartQueueTbl[0] = xQueueCreate(1, sizeof(uint8_t));
	uartQueueTbl[1] = xQueueCreate(128, sizeof(uint8_t));
	uartQueueTbl[2] = xQueueCreate(1, sizeof(uint8_t));
	for(int i=1;i<2;i++){
		struct UartParamSt tx = {
			.tr = 't',
			.port = i,
			.test_rst = 0,
		};
		OS_ASSERT(xTaskCreate(uart_proc,"uart_tx", 2048/4, &tx , 5, &tx.handle));
		struct UartParamSt rx = {
			.tr = 'r',
			.port = i,
			.test_rst = 0,
		};
		OS_ASSERT(xTaskCreate(uart_proc,"uart_rx", 2048/4, &rx , 5, &rx.handle));

		for(int i=0;i<5;i++){
			if(rx.test_rst)
				break;
			os_sleep_ms(1000);
		}
		if(rx.test_rst){
			testFlag |= 1<<rx.port;
		}

		vTaskDelete(tx.handle);
		vTaskDelete(rx.handle);
		os_sleep_ms(500);
	}

	if(testFlag == 0b111){
		os_printf("UART test complete\n");
		return 0;
	}else{
		os_printf("UART test has FAIL***\n");
		return -1;
	}
}


static int test_4g()
{
#define UART_4G				(1)
#define UART_4G_BUFSIZE		(128)
	char buf[UART_4G_BUFSIZE];
	uartQueueTbl[1] = xQueueCreate(128, sizeof(uint8_t));
	struct AtTalkTable {
		const char *tx;
		const char *rx;
		int retry;
		int rx_timeout;
	}attbl[] = {
			{"ATE1",		"OK",	10,	1000},
			{"AT+CIMI",		"OK",	5,	1000},
			{"AT+MIPPROFILE=1,\"CMNET\"",	"OK",	5,	1000},
	};
	int step=0;
	for(;step<sizeof(attbl)/sizeof(attbl[0]);step++){
		int r=0;
		for(;r<attbl[step].retry;r++){
			LPUART_receive_flush(UART_4G);
			LPUART_transmit_string(UART_4G, attbl[step].tx);
			LPUART_transmit_string(UART_4G,"\r\n");
			char *s = LPUART_receive_string_timeout(UART_4G, buf, UART_4G_BUFSIZE, attbl[step].rx_timeout);
			if(s || strcmp(s,attbl[step].rx)==0)
				break;
		}
		if(r==attbl[step].retry)
			break;
	}
	if(step==sizeof(attbl)/sizeof(attbl[0])){
		os_printf("AT route complete!\n");
		return 0;
	}else{
		os_printf("AT route abort, step=%d cmd=%s wand=%s!\n",step,attbl[step].tx,attbl[step].rx);
		return -1;
	}
}

static TaskHandle_t mainTaskHandle;
void mainTask(void *p)
{
	os_printf("board layout init ok. build %s %s\n",__DATE__,__TIME__);
	static const struct {
		const char *testitem;
		int (*func)(void);
	} test_tbl[] = {
			{"UART0/1/2 Loop-back Test",		test_uart},
			{"4G Module Test",					test_4g},
			{"SD Card Test",					test_sd},
			{"CAN0/1/2 Loop Test",				test_can},
			{"Buzzer Test\n",					test_buzzer},
	};

	for(int i=0;i<sizeof(test_tbl)/sizeof(test_tbl[0]);i++){
		os_printf("\n");
		os_printf("%s\n",test_tbl[i].testitem);
		test_tbl[i].func();
	}

	for(;;);
}
void PORT_init1 (void) {
  PCC->PCCn[PCC_PORTC_INDEX ]|=PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
  PORTC->PCR[6]|=PORT_PCR_MUX(2);           /* Port C6: MUX = ALT2,UART1 TX */
  PORTC->PCR[7]|=PORT_PCR_MUX(2);           /* Port C7: MUX = ALT2,UART1 RX */
}
int main2(void)
{
  WDOG_disable();        /* Disable WDGO*/
  SOSC_init_8MHz();      /* Initialize system oscilator for 8 MHz xtal */
  SPLL_init_160MHz();    /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
  NormalRUNmode_80MHz(); /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
  PORT_init1();           /* Configure ports */

  LPUART_init(LPUART1,115200);        /* Initialize LPUART @ 9600*/
 // LPUART1_transmit_string("Running LPUART example\n\r");     /* Transmit char string */
 // LPUART1_transmit_string("Input character to echo...\n\r"); /* Transmit char string */

  for(;;) {
	  LPUART_transmit_char(LPUART1,'>');  		/* Transmit prompt character*/
	  int c = LPUART_receive_char(LPUART1);
	  if(c!=-1){
		  printf("getc\n");
	  }else{
		  printf(".\n");
	  }
  }
}
int main()
{
	WDOG_disable();
	SOSC_init_8MHz(); /* Initialize system oscilator for 8 MHz xtal */
	SPLL_init_160MHz(); /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz(); /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	NVIC_init_IRQs();        /* Enable desired interrupts and priorities */
	PORT_init();             /* Configure ports */

	LPSPI0_init_master();
	MSD0_SPIHighSpeed(0);

	FLEXCAN_init(0,0xA,500000);
	FLEXCAN_init(1,0xB,500000);
	FLEXCAN_init(2,0xC,500000);

	//LPUART_init(LPUART0, 57600);	//RS485
	LPUART_init(LPUART1, 115200);	//4G
	//LPUART_init(LPUART2, 9600);	//GPS

	init_printf_mutex();
	OS_ASSERT(xTaskCreate(mainTask,"mainTask", 4096/4, NULL , 1, &mainTaskHandle));

	/*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
	#ifdef PEX_RTOS_START
	PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
	#endif

	return 0;
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	os_printf("\n*** OS Task Stack Overflow %s.\n",pcTaskName);
}
void vApplicationMallocFailedHook(void)
{
	os_printf("\n*** OS Malloc Fail.\n");
}


