#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "S32K144.h"
#include "bl.h"
#include "ff.h"
#include "convert.h"
#include "timelib.h"
#include "oswrapper.h"

void uart_send(int port,int len,const uint8_t* data);

int test_buzzer()
{
	for(int i=0;i<3;i++){
		buzzer_ctrl(1);
		os_trace("buzzer_on\n");
		os_sleep_ms(1000);
		buzzer_ctrl(0);
		os_trace("buzzer_off\n");
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

	os_trace("mount filesystem\n");
	res = f_mount(0, &fatfs);
	if (res != FR_OK) {
		os_trace("mount filesystem 0 failed : %d\n", res);
		return -1;
	}

	os_trace("create test.txt\n");
	res = f_open(&f, "test.txt", FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
	if (res != FR_OK){
		os_trace("write error\n");
		return -1;
	}

	os_trace("write: \"test12345\"\n");
	f_lseek(&f, 0);
	res = f_write(&f, "test12345", 9, &flen);
	if (res != FR_OK){
		os_trace("write error\n");
		return -1;
	}


	os_trace("read and check file data\n");
	char buf[128];
	memset(buf,0,128);
	UINT rd;
	f_lseek(&f, 0);
	res = f_read(&f, buf, 9, &rd);
	if(res != FR_OK){
		os_trace("read error\n");
		return -1;
	}
	if(rd!=9){
		os_trace("read len error %d\n",rd);
		return -1;
	}
	if(strcmp(buf,"test12345")){
		os_trace("read data error %s\n",buf);
		return -1;
	}

	os_trace("close file\n");
	res = f_close(&f);
	if(res != FR_OK){
		os_trace("close error\n");
		return -1;
	}
	return 0;
}

struct CanParamSt{
	OS_Mutex mtx;
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

		os_mutex_lock(&(canSt->mtx));
		FLEXCAN_transmit_msg(port,targetID,8,(uint8_t*)"\xAA\x55\xAA\x55\xAA\x55\xAA\x55"); /* Transmit message using MB0 */
		os_mutex_unlock(&(canSt->mtx));

		uint32_t RxLENGTH;
		uint8_t RxDATA[8];
		os_mutex_lock(&(canSt->mtx));
		int res = FLEXCAN_receive_msg(port, &RxLENGTH, RxDATA);
		os_mutex_unlock(&(canSt->mtx));
		if(res==0){
			char buf[32];
			os_trace("CAN%d get message :RxLENGTH=%X RxDATA=%s\n",port,RxLENGTH,hx_dumphex2str(RxDATA, 8, buf));
			canSt->rxFlag = 1;
		}else{

		}

		os_sleep_ms(1000);
	}
}
static int test_can()
{
	uint32_t test_rst = 0U;
	OS_Mutex  can_mutex;
	os_mutex_create(&can_mutex);

	for(int i=0;i<3;i++){
		int j = (i+1)%3;
		os_trace("CAN%d <-> CAN%d\n",i,j);
		struct CanParamSt canA = {
			.mtx = can_mutex,
			.selfPort = i,
			.targetPort = j,
			.rxFlag = 0,
		};
		OS_ASSERT(pdTRUE==xTaskCreate(can_task_proc,"can_task_a", 1024/4, &canA , 5, &canA.handle));
		struct CanParamSt canB = {
			.mtx = can_mutex,
			.selfPort = j,
			.targetPort = i,
			.rxFlag = 0,
		};
		OS_ASSERT(pdTRUE==xTaskCreate(can_task_proc,"can_task_b", 1024/4, &canB , 5, &canB.handle));

		for(int i=0;i<8;i++){
			if(canA.rxFlag && canB.rxFlag)
				break;
			os_sleep_ms(1000);
		}
		if(canA.rxFlag && canB.rxFlag){
			os_trace("CAN%d & CAN%d PASS.\n",i,j);
			test_rst |= 1<<i;
		}
		if(canA.rxFlag==0)
			os_trace("CAN%d receive timeout, Test FAIL***\n",i);
		if(canB.rxFlag==0)
			os_trace("CAN%d receive timeout, Test FAIL***\n",j);

		vTaskDelete(canA.handle);
		vTaskDelete(canB.handle);
		os_sleep_ms(500);
	}

	if(test_rst == 0b111){
		os_trace("CAN test complete\n");
		return 0;
	}else{
		os_trace("CAN test has FAIL***\n");
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

void UartRxIRQHandler(int port,uint8_t rxbyte)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	uint8_t c = rxbyte;
	xQueueSendToBackFromISR(uartQueueTbl[port],&c,&xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken)
}
int uart_getc_timeout(int port,int timeout){
	uint8_t c;
	if(xQueueReceive(uartQueueTbl[port], &c, timeout)==pdPASS){
		return c;
	}else{
		return -1;
	}
}
void uart_rx_flush(int port)
{
	xQueueReset(uartQueueTbl[port]);
}
char *uart_gets_timeout(int port,char *buf, int bufsize, int timeout)
{
	char *p = buf;
	char *end = buf+bufsize-1;
	while(p<end){
		int c = uart_getc_timeout(port, timeout);
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
char *uart_gets_echo_blocked(int port,char *buf, int bufsize)
{
	char *p = buf;
	char *end = buf+bufsize-1;
	while(p<end){
		int c = uart_getc_timeout(port, portMAX_DELAY);
		if(c==-1)
			return NULL;	/*timeout*/
		if(c=='\r' || c=='\n'){
			*p = '\0';
			uart_send(port, 2, (uint8_t*)"\r\n");
			return buf;		/*get*/
		}
		uint8_t cc = c;
		uart_send(port, 1, &cc);
		*p++ = c;
	}
	*p = '\0';
	return buf;		/*full*/
}
char *uart_gets_blocked(int port,char *buf, int bufsize)
{
	return uart_gets_timeout(port,buf,bufsize,portMAX_DELAY);
}
int uart_getdata(int port,uint8_t *buf,int bufsize,int recv_tmout,int frame_tmout)
{
	uint8_t *p = buf;
	uint8_t *end = buf + bufsize;
	int c = uart_getc_timeout(port,recv_tmout);
	if(c==-1)
		return -1;
	*p++ = c;
	do{
		c = uart_getc_timeout(port,frame_tmout);
		if(c==-1)
			break;
		*p++ = c;
	}while(p<end);
	return p-buf;
}
unsigned uart_getdata_blocked(int port,uint8_t *buf,int bufsize,int frame_tmout)
{
	int r = uart_getdata(port,buf,bufsize,portMAX_DELAY,frame_tmout);
	return r<0?0:r;
}
void uart_puts(int port,const char *data_string)  {
	taskENTER_CRITICAL();
	const char *p = data_string;
	int c;
	while ((c=*p++))
		LPUART_transmit_char(port, c);
	taskEXIT_CRITICAL();
}
void uart_send_lf_crlf(int port,int len,const uint8_t* data)
{
	const uint8_t *p = data;
	const uint8_t *end = p+len;
	taskENTER_CRITICAL();
	if(port==UART_RS485)rs485_dir(1);
	while(p<end){
		if(*p=='\n')
			LPUART_transmit_char(port, '\r');
		LPUART_transmit_char(port, *p++);
	}
	if(port==UART_RS485)rs485_dir(0);
	taskEXIT_CRITICAL();
}
void uart_send(int port,int len,const uint8_t* data)
{
	const uint8_t *p = data;
	const uint8_t *end = p+len;
	taskENTER_CRITICAL();
	if(port==UART_RS485)rs485_dir(1);
	while(p<end){
		LPUART_transmit_char(port, *p++);
	}
	if(port==UART_RS485)rs485_dir(0);
	taskEXIT_CRITICAL();
}

static void uart_proc(void *p)
{
	struct UartParamSt* param = (struct UartParamSt*)p;
	if(param->tr=='t'){
		for(int i=0;i<10;i++){
			os_trace("UART%d send message\n",param->port);
			uart_puts(param->port, "$$TEST$$\n");
			os_sleep_ms(1000);
		}
	}else{
		for(;;){
			char buf[64];
			os_trace("UART%d wait data.\n",param->port);
			uart_gets_blocked(param->port, buf, 64);
			os_trace("UART%d get message: %s\n",param->port,buf);
			//param->test_rst = 1;
			os_sleep_ms(100);
		}
	}

}

static int test_uart()
{
	uint32_t testFlag;
	for(int i=0;i<=0;i++){
		struct UartParamSt tx = {
			.tr = 't',
			.port = i,
			.test_rst = 0,
		};
		OS_ASSERT(pdTRUE==xTaskCreate(uart_proc,"uart_tx", 2048/4, &tx , 5, &tx.handle));
		struct UartParamSt rx = {
			.tr = 'r',
			.port = i,
			.test_rst = 0,
		};
		OS_ASSERT(pdTRUE==xTaskCreate(uart_proc,"uart_rx", 2048/4, &rx , 5, &rx.handle));

		for(int i=0;i<10;i++){
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
		os_trace("UART test complete\n");
		return 0;
	}else{
		os_trace("UART test has FAIL***\n");
		return -1;
	}
}
#define NIC_RST_CLR()   		PTD-> PCOR |= 1<<16
#define NIC_RST_SET()    		PTD-> PSOR |= 1<<16
#define NIC_POWER_ON_CLR()    		PTD-> PCOR |= 1<<15
#define NIC_POWER_ON_SET()   		PTD-> PSOR |= 1<<15
#define NIC_VBAT_CLR()				PTD-> PCOR |= 1<<11
#define NIC_VBAT_SET()				PTD-> PSOR |= 1<<11
void nic_io_init(){
	PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
	PORTD->PCR[15] |= PORT_PCR_MUX(1); /* Port C6: MUX = ALT2,UART1 TX */
	PORTD->PCR[16] |= PORT_PCR_MUX(1); /* Port C7: MUX = ALT2,UART1 RX */
	PORTD->PCR[11] |= PORT_PCR_MUX(1); /* Port C7: MUX = ALT2,UART1 RX */
	PTD->PDDR |= 1 << 15;
	PTD->PDDR |= 1 << 16;
	PTD->PDDR |= 1 << 11;
	NIC_RST_CLR();
	NIC_POWER_ON_CLR();
	NIC_VBAT_CLR();
}
static void nic_power_onoff(void)
{
	NIC_VBAT_CLR();
	os_sleep_ms(500);		/* 500 ms might be ok */
	NIC_VBAT_SET();
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
	int test_trys =3;
	test_start:
	nic_power_onoff();
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
		int a=0,b=0;
		res = sscanf(buf,"+CSQ: %d,%d",&a,&b);
		if(res==2 &&(a>0 && a<99))
			return 0;
		return -1;
	}
	int at_creg(){
		int res;
		int a=0,b=0;
		res = sscanf(buf,"+CREG: %d,%d",&a,&b);
		if(res==2 && (b==1||b==5))
			return 0;
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
		int a=0,b=0;
		res = sscanf(buf,"+MIPOPEN=%d,%d",&a,&b);
		if(res==2 && a==sockid && b==1)
			return 0;
		return -1;;
	}
	int at_mipcall(){
		int res;
		int a=0;
		res = sscanf(buf,"+MIPCALL:%d, %s",&a,lcip);
		if(res==2 && a==sockid)
			return 0;
		return -1;;
	}
	struct AtTalkTable {
		const char *ts;
		const char *(*tf)();
		const char *rs;
		const char *abort;
		int (*rf)();
		int retry;
		int tmout;
		/*
		 * when retry tmout not init, use def val
		 */
#define _DFT_AT_RETRY	(5)
#define _DFT_AT_TMOUT	(1000)
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
			{.ts="ATE1",			.rs="OK",				.retry=30, .tmout=1000},
			{.ts="ATE0",			.rs="OK",				},
			{.ts="AT+CPIN?",		.rs="+CPIN: READY",		},
			{.ts="AT+CSQ",			.rf=at_csq,				},
			{.ts="AT+CREG?",		.rf=at_creg,			},
			{.ts="AT+CIMI",			.rf=at_cimi,			},
			{.tf=at_miprofile,		.rs="OK",				},
			{.ts="AT+MIPCALL=1",	.rf=at_mipcall,			.retry=5,	.tmout=10000},
			{.ts="AT+MIPCALL?",		.rs="OK",				.retry=5,	.tmout=2000},
			{.tf=at_mipopen,		.rf=at_miopen_r,		.retry=5,	.tmout=5000},
			{.ts="AT+MIPHEX=0",		.rs="OK",				},

			//{.tx="AT+CIMI",		.rx="OK",	5,	1000},
			//{.tx="AT+MIPPROFILE=1,\"CMNET\"",	"OK",	5,	1000},
	};
	int step=0;
	for(;step<sizeof(attbl)/sizeof(attbl[0]);step++){
		int need_try = 1;
		int retry = attbl[step].retry?:_DFT_AT_RETRY;
		for(int r=0;need_try&&r<retry;r++){
			const char *ts = attbl[step].ts?:attbl[step].tf();
			//LPUART_receive_flush(UART_4G);
			os_trace("put:%s\n",ts);
			uart_puts(UART_4G, ts);
			uart_puts(UART_4G,"\r\n");
			os_sleep_ms(100);	/* must delay if not, rx nothing. */
			char *s;
			do{
				s = uart_gets_timeout(UART_4G, buf, UART_4G_BUFSIZE, attbl[step].tmout?:_DFT_AT_TMOUT);
				if(s){
					os_trace("gets:%s\n",s);
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
		os_sleep_ms(500);	/* next cmd before delay some ms. */
	}
	if(step==sizeof(attbl)/sizeof(attbl[0])){
		os_trace("AT route complete!\n");
		return 0;
	}else{
		os_trace("AT route abort, step=%d cmd=%s want=%s!\n",step,attbl[step].ts,attbl[step].rs);
		if(test_trys-->0)
			goto test_start;
		return -1;
	}
}

void load_time()
{
	struct DS3231_Time dst;
	struct DS3231_Time dst2;
	taskENTER_CRITICAL();
	ds3231_gettime(&dst2);	//local
	do{
		ds3231_gettime(&dst);
	}while(dst.sec==dst2.sec);
	taskEXIT_CRITICAL();
	struct tm ltm = {
		.tm_sec = bcd2int(dst.sec),
		.tm_min = bcd2int(dst.min),
		.tm_hour = bcd2int(dst.hour),
		.tm_wday = bcd2int(dst.wday)-1,
		.tm_mday = bcd2int(dst.mday),
		.tm_mon = bcd2int(dst.mon),
		.tm_year = (2000 + bcd2int(dst.year)) - 1900,
	};
	time_t now = mktime(&ltm);	//to local
	time(&now);
}
void store_time()
{
	time_t t = time(NULL);		//utc
	struct tm ltm;
	localtime_s(&t,&ltm);		//to local
	struct DS3231_Time dst = {
		.sec = 	int2bcd(ltm.tm_sec),
		.min = 	int2bcd(ltm.tm_min),
		.hour = int2bcd(ltm.tm_hour),
		.wday = int2bcd(ltm.tm_wday+1),
		.mday = int2bcd(ltm.tm_mday),
		.mon = 	int2bcd(ltm.tm_mon),
		.year = int2bcd(ltm.tm_year%100),
	};
	taskENTER_CRITICAL();
	ds3231_settime(&dst);
	taskEXIT_CRITICAL();
}

int test_rtc(void)
{
	TaskHandle_t rtc_handle;
	float tempr = ds3231_gettempr();
	os_trace("DS3231 temprature: %f\n", tempr);
	void rtc_proc(void* p){
		(void)p;
		for(;;){
			struct tm ltm;
			localtime_s(NULL,&ltm);
			os_trace("read time: %s",asctime(&ltm));
			os_sleep_ms(1000);
		}
	}
	OS_ASSERT(pdTRUE==xTaskCreate(rtc_proc,"rtc_proc", 1024/4, NULL , 5, &rtc_handle));
	for(int i=0;i<2;i++){
		load_time();		//per 5-sec load time from hardware
		os_sleep_ms(5000);
	}
	vTaskDelete(rtc_handle);
	return 0;
}
int test_ds2411()
{
	for(int i=0;i<5;i++){
		char asc[17];
		char buf[8];
		taskENTER_CRITICAL();
		int res = ds2411_getsn((struct DS2411_SN*)buf);
		taskEXIT_CRITICAL();
		os_trace("res=%d\n",res);
		os_trace("%s\n",hx_dumphex2str(buf,8,asc));
		if(res==0){
			os_trace("read success.\n");
			return 0;
		}
		os_sleep_ms(1000);
	}
	os_trace("read failure.\n");
	return -1;
}

int test_gps()
{
#define UART_GPS_BUFSIZE		(512U)
	uint8_t buf[UART_GPS_BUFSIZE];
	for(int i=0;i<5;i++){
		int rl = uart_getdata(UART_GPS,buf,UART_GPS_BUFSIZE,3000,100);
		if(rl>0){
			if(rl>=512){
				os_trace("received GPS data too long (%d), truncate to %d byte.\n",
						rl,UART_GPS_BUFSIZE -1);
				rl = UART_GPS_BUFSIZE - 1;
			}
			buf[rl] = 0;
			os_trace("received GPS data: len=%u : \n%s\n",rl,buf);
			return 0;
		}else{
			os_trace("receive GPS data timeout.\n");
			return -1;
		}
	}
	return -1;
}

int test_485()
{
#define UART_485_BUFSIZE		(512U)
	uint8_t buf[UART_485_BUFSIZE];
	int rl = uart_getdata(UART_RS485,buf,UART_485_BUFSIZE,5000,100);
	if(rl>0){
		if(rl>=512){
			os_trace("received RS485 data too long (%d), truncate to %d byte.\n",
					rl,UART_485_BUFSIZE -1);
			rl = UART_485_BUFSIZE - 1;
		}
		buf[rl] = 0;
		os_trace("received RS485 data: len=%u : \n%s\n",rl,buf);
		return 0;
	}else{
		os_trace("receive RS485 data timeout.\n");
		return -1;
	}
}

static int test_io_in(void)
{
	int trys = 3;
	while(trys--){
		for(int i=0;i<3;i++){
			int v = get_input_val(i);
			os_trace("IO IN Channel-%d get %s\n",i,v?"H":"L");
		}
		os_sleep_ms(500);
	}
	return 0;
}
static int test_led(void)
{
	int trys = 3;
	while(trys--){
		os_trace("LED pin output H\n");
		for(int i=0;i<6;i++){
			led_ctrl(i,1);
		}
		os_sleep_ms(1000);
		os_trace("LED pin output L\n");
		for(int i=0;i<6;i++){
			led_ctrl(i,0);
		}
		os_sleep_ms(1000);
	}
	return 0;
}

static TaskHandle_t mainTaskHandle;
void mainTask(void *p)
{

	static const struct {
		const char *testitem;
		int (*func)(void);
	} test_tbl[] = {
			//{"T",								test_uart_loop},
			{"LED Output Test",					test_led},
			{"IO_IN Test",						test_io_in},
			{"UART0/1/2 Loop-back Test",		test_uart},
			{"GPS Test",						test_gps},
			{"RS485 loop-back Test",			test_485},
			{"DS2411 SN Read Test",				test_ds2411},
			{"RTC Test",						test_rtc},
			{"4G Module Test",					test_4g},
			{"SD Card Test",					test_sd},
			{"CAN0/1/2 loop-cycle Test",		test_can},
			//{"CAN0/1/2 loop-back Test",			test_can1},
			{"Buzzer Test\n",					test_buzzer},
	};
	const int tbllen = sizeof(test_tbl)/sizeof(test_tbl[0]);
	os_trace("\n\nboard routes init ok.\n");
	for(;;){
		os_trace("dp1000 test program, version:0.1 build: %s %s\n",__DATE__,__TIME__);
		os_trace("-------------------------------------------------\n");
		os_trace("Type \"%u\"-\"%u\" to select test item, or \"a\" for all:\n",0,tbllen);
		os_trace("time setup use command:\n");
		os_trace("      \"ts 20171012120000 \"\n");

		for(int i=0;i<tbllen;i++){
			os_trace("%u.%s\n",(int)(i+1),test_tbl[i].testitem);
		}
		char buf[128];
		again:
		memset(buf,0,128);
		while(os_console_gets(buf,128)==NULL);
		if(strncmp(buf,"ts ",2)==0){
			int yh;
			struct tm t;
			int n = sscanf(buf,"ts %02u%02u%02u%02u%02u%02u%02u",
					&yh,&t.tm_year,&t.tm_mon,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec);
			if(n!=7){
				os_trace("input format error, please again.\n");
				goto again;
			}
			t.tm_year = t.tm_year+2000 - 1900;
			t.tm_mon = t.tm_mon-1;
			time_t set = mktime(&t);
			time(&set);
			store_time();

			localtime_s(&set,&t);
			os_trace("time set: %s",asctime(&t));
			continue;
		}
		if(strcmp(buf,"a")==0 || strcmp(buf,"A")==0){
			for(int i=0;i<sizeof(test_tbl)/sizeof(test_tbl[0]);i++){
				os_trace("%s\n",test_tbl[i].testitem);
				int res = test_tbl[i].func();
				os_trace(">>> %s\n",res==0?"PASS":"FAIL");
			}
			continue;
		}
		char *err;
		int index = strtoul(buf,&err,10);
		if(err!=NULL && *err=='\0' && index>=1 && index<=tbllen){
			index--;
			os_trace("%s\n",test_tbl[index].testitem);
			int res = test_tbl[index].func();
			os_trace(">>> %s\n",res==0?"PASS":"FAIL");
			continue;
		}
		os_trace("input error, please again.\n");
		goto again;
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
	//MSD0_SPIHighSpeed(1);

	FLEXCAN_init(0,0xA,500000);
	FLEXCAN_init(1,0xB,500000);
	FLEXCAN_init(2,0xC,500000);

	uartQueueTbl[0] = xQueueCreate(128, sizeof(uint8_t));
	uartQueueTbl[1] = xQueueCreate(128, sizeof(uint8_t));
	uartQueueTbl[2] = xQueueCreate(128, sizeof(uint8_t));

	LPUART_init(UART_RS485, 9600);
	LPUART_init(UART_4G, 	115200);
	LPUART_init(UART_GPS, 	9600);

	os_trace_init();
	OS_ASSERT(pdTRUE==xTaskCreate(mainTask,"mainTask", 4096/4, NULL , 5, &mainTaskHandle));

	/*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
	#ifdef PEX_RTOS_START
	PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
	#endif

	return 0;
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	os_trace("\n*** OS Task Stack Overflow %s.\n",pcTaskName);
}
void vApplicationMallocFailedHook(void)
{
	os_trace("\n*** OS Malloc Fail.\n");
}
