#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "S32K144.h"
#include "bl.h"
#include "ff.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
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

struct CanTestSt{
	SemaphoreHandle_t mtx;
	int selfPort;
	int targetPort;
	TaskHandle_t handle;
	int rxFlag;
};
static void can_task_proc(void *p)
{
	struct CanTestSt *canSt = (struct CanTestSt*)p;
	int port = canSt->selfPort;
	int targetID = canSt->targetPort + 0xA;

	for (;;) {

		xSemaphoreTake(canSt->mtx,portMAX_DELAY);
		FLEXCAN0_transmit_msg(port,targetID,8,(uint8_t*)"\xAA\x55\xAA\x55\xAA\x55\xAA\x55"); /* Transmit message using MB0 */
		xSemaphoreGive(canSt->mtx);

		uint32_t RxLENGTH;
		uint8_t RxDATA[8];
		xSemaphoreTake(canSt->mtx,portMAX_DELAY);
		int res = FLEXCAN0_receive_msg(port, &RxLENGTH, RxDATA);
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
		struct CanTestSt canA = {
			.mtx = can_mutex,
			.selfPort = i,
			.targetPort = j,
			.rxFlag = 0,
		};
		OS_ASSERT(xTaskCreate(can_task_proc,"can_task_a", 1024/4, &canA , 1, &canA.handle));
		struct CanTestSt canB = {
			.mtx = can_mutex,
			.selfPort = j,
			.targetPort = i,
			.rxFlag = 0,
		};
		OS_ASSERT(xTaskCreate(can_task_proc,"can_task_b", 1024/4, &canB , 1, &canB.handle));

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


static TaskHandle_t mainTaskHandle;
void mainTask(void *p)
{
	os_printf("board layout init ok. build %s %s\n",__DATE__,__TIME__);
	static const struct {
		const char *testitem;
		int (*func)(void);
	} test_tbl[] = {
			{"SD Card Test",					test_sd},
			{"3-Channels CAN Loop Test",		test_can},
			//{"Buzzer Test\n",					test_buzzer},
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

	FLEXCAN0_init(0,0xA,500000);
	FLEXCAN0_init(1,0xB,500000);
	FLEXCAN0_init(2,0xC,500000);

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


