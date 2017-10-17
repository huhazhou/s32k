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
void os_suspend(void)
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
			os_printf("CAN%d & CAN%d PASS.\n",i,j);
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
	if(xQueueReceive(uartQueueTbl[port], &c, timeout)==pdPASS){
		return c;
	}else{
		return -1;
	}
}
void LPUART_receive_flush(int port)
{
	xQueueReset(uartQueueTbl[port]);
}
char *LPUART_receive_string_timeout(int port,char *buf, int bufsize, int timeout)
{
	char *p = buf;
	char *end = buf+bufsize-1;
	while(p<end){
		int c = LPUART_receive_from_queue(port,timeout);
		if(c==-1)
			return NULL;	/*timeout*/
		if(c=='\r')
			continue;
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
static void test_uart_loop(void *p)
{
	uartQueueTbl[1] = xQueueCreate(128, sizeof(uint8_t));
	os_printf("UART%d send message\n",1);
	LPUART_transmit_string(1, "$$TEST$$\n");
	os_sleep_ms(1000);
	char buf[64];
	LPUART_receive_string_timeout(1, buf, 64, 1000);//(1, buf, 64);
	os_printf("UART%d get message: %s\n",1,buf);
	os_suspend();
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
		os_printf("UART%d get message: %s\n",param->port,buf);
		param->test_rst = 1;
		os_suspend();
	}

}

static int test_uart()
{
	uint32_t testFlag;
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
#define NIC_RST_CLR()   		PTD-> PCOR |= 1<<16
#define NIC_RST_SET()    		PTD-> PSOR |= 1<<16
#define NIC_POWER_ON_CLR()    		PTD-> PCOR |= 1<<15
#define NIC_POWER_ON_SET()   		PTD-> PSOR |= 1<<15
void nic_io_init(){
	PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
	PTD->PDDR |= 1 << 15;
	PTD->PDDR |= 1 << 16;
	PORTD->PCR[15] |= PORT_PCR_MUX(1); /* Port C6: MUX = ALT2,UART1 TX */
	PORTD->PCR[16] |= PORT_PCR_MUX(1); /* Port C7: MUX = ALT2,UART1 RX */
	NIC_RST_CLR();
	NIC_POWER_ON_CLR();
}
void nic_power_onoff(void)
{
	NIC_POWER_ON_CLR();
	os_sleep_ms(300);
	NIC_POWER_ON_SET();		/* ____--2s--___ */
	os_sleep_ms(3000);
	NIC_POWER_ON_CLR();
	os_sleep_ms(300);
}
void nic_reset(void)
{
	NIC_RST_CLR();
	os_sleep_ms(300);
	NIC_RST_SET();		/* ____--600ms--___ */
	os_sleep_ms(800);
	NIC_RST_CLR();
	os_sleep_ms(300);
}
static int test_4g()
{
	nic_power_onoff();
#define UART_4G				(1)
#define UART_4G_BUFSIZE		(128)
	char buf[UART_4G_BUFSIZE];
	char imsi[16];
	const char *apn = "CMNET";
	int sockid=1;
	uint16_t lcport = 5000;
	char *rmip = "180.89.58.27";
	uint16_t rmport = 9020;
	char lcip[16];

	int at_csq(){
		int res;
		if(buf[0] == '+'){
			int a=0,b=0;
			res = sscanf(buf,"+CSQ: %d,%d",&a,&b);
			if(res==2 &&(a>0 && a<99))
				return 0;
		}
		return -1;
	}
	int at_creg(){
		int res;
		if(buf[0] == '+'){
			int a=0,b=0;
			res = sscanf(buf,"+CREG: %d,%d",&a,&b);
			if(res==2 && (b==1||b==5))
				return 0;
		}
		return -1;;
	}
	int at_cimi(){
		if(strlen(buf)==15 && strspn(buf,"0123456789")==15){
			strcpy(imsi,buf);
			return 0;
		}
		return -1;;
	}
	const char* at_miprofile(){
		snprintf(buf,UART_4G_BUFSIZE,"AT+MIPPROFILE=1,\"%s\"",apn);
		return buf;
	}
	const char* at_mipopen(){
		snprintf(buf,UART_4G_BUFSIZE,"AT+MIPOPEN=%d,%hu,\"%s\",%hu,0",
				sockid,lcport,rmip,rmport);
		return buf;
	}

	int at_miopen_r(){
		int res;
		if(buf[0] == '+'){
			int a=0,b=0;
			res = sscanf(buf,"+MIPOPEN=%d,%d",&a,&b);
			if(res==2 && a==sockid && b==1)
				return 0;
		}
		return -1;;
	}
	int at_mipcall(){
		int res;
		if(buf[0] == '+'){
			int a=0;
			res = sscanf(buf,"+MIPCALL:%d, %s",&a,lcip);
			if(res==2 && a==sockid)
				return 0;
		}
		return -1;;
	}
	struct AtTalkTable {
		const char *tx;
		const char *(*tf)();
		const char *rs;
		const char *abort;
		int (*rf)();
		int retry;
		int tmout;
	}attbl[] = {
			/*
AT+CIMI
AT+MIPPROFILE=1,"CMNET"
AT+MIPCALL=1
AT+MIPCALL?
AT+MIPOPEN=1,5000,"180.89.58.27",9020,0
AT+MIPHEX=0
AT+MIPTPS=1,1,5000,600
			 */
			{.tx="ATE1",			.rs="OK",				.retry=60,	.tmout=1000},
			{.tx="ATE0",			.rs="OK",				.retry=5,	.tmout=1000},
			{.tx="AT+CPIN?",		.rs="+CPIN: READY",		.retry=20,	.tmout=1000},
			{.tx="AT+CSQ",			.rs=0,.rf=at_csq,		.retry=20, 	.tmout=1000},
			{.tx="AT+CREG?",		.rs=0,.rf=at_creg,		.retry=20,	.tmout=1000},
			{.tx="AT+CIMI",			.rs=0,.rf=at_cimi,		.retry=20,	.tmout=1000},
			{.tx=0,.tf=at_miprofile,.rs="OK",				.retry=20,	.tmout=1000},
			{.tx="AT+MIPCALL=1",	.rs=0,.rf=at_mipcall,	.retry=5,	.tmout=10000},
			{.tx="AT+MIPCALL?",		.rs="OK",				.retry=5,	.tmout=10000},
			{.tx=0,.tf=at_mipopen,	.rs=0,.rf=at_miopen_r,	.retry=5,	.tmout=5000},
			{.tx="AT+MIPHEX=0",		.rs="OK",				.retry=60,	.tmout=1000},

			//{.tx="AT+CIMI",		.rx="OK",	5,	1000},
			//{.tx="AT+MIPPROFILE=1,\"CMNET\"",	"OK",	5,	1000},
	};
	int step=0;
	for(;step<sizeof(attbl)/sizeof(attbl[0]);step++){
		int need_try = 1;
		for(int r=0;need_try&&r<attbl[step].retry;r++){
			//LPUART_receive_flush(UART_4G);
			if(attbl[step].tx){
				os_printf("put:%s\n",attbl[step].tx);
				LPUART_transmit_string(UART_4G, attbl[step].tx);
			}else{
				const char *ts = attbl[step].tf();
				os_printf("put:%s\n",ts);
				LPUART_transmit_string(UART_4G, ts);
			}
			LPUART_transmit_string(UART_4G,"\r\n");
			os_sleep_ms(500);	/* must delay if not, rx nothing. */
			char *s;
			do{
				s = LPUART_receive_string_timeout(UART_4G, buf, UART_4G_BUFSIZE, attbl[step].tmout);
				if(s){
					os_printf("gets:%s\n",s);
				}
				if(attbl[step].rs){
					if(s && strcmp(s,attbl[step].rs)==0){
						need_try = 0;
					}
				}else{
					need_try = attbl[step].rf();
				}
			}while(s && need_try);
		}
		if(need_try)
			break;
	}
	if(step==sizeof(attbl)/sizeof(attbl[0])){
		os_printf("AT route complete!\n");
		return 0;
	}else{
		os_printf("AT route abort, step=%d cmd=%s want=%s!\n",step,attbl[step].tx,attbl[step].rs);
		return -1;
	}
}

int test_rtc(void)
{
	for(;;){
		os_printf("tickcount=%u\n",get_tick_count());
		os_sleep_ms(1000);
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
			//{"T",								test_uart_loop},
			//{"UART0/1/2 Loop-back Test",		test_uart},
			{"RTC Test",						test_rtc},
//			{"4G Module Test",					test_4g},
//			{"SD Card Test",					test_sd},
//			{"CAN0/1/2 Loop Test",				test_can},
//			{"Buzzer Test\n",					test_buzzer},
	};

	for(int i=0;i<sizeof(test_tbl)/sizeof(test_tbl[0]);i++){
		os_printf("\n");
		os_printf("%s\n",test_tbl[i].testitem);
		test_tbl[i].func();
	}

	for(;;);
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

	uartQueueTbl[0] = xQueueCreate(1, sizeof(uint8_t));
	uartQueueTbl[1] = xQueueCreate(128, sizeof(uint8_t));
	uartQueueTbl[2] = xQueueCreate(1, sizeof(uint8_t));
	//LPUART_init(LPUART0, 57600);	//RS485
	LPUART_init(LPUART1, 115200);	//4G
	//LPUART_init(LPUART2, 9600);	//GPS

	init_printf_mutex();
	OS_ASSERT(xTaskCreate(mainTask,"mainTask", 4096/4, NULL , 5, &mainTaskHandle));

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


