/*
 * convert.h
 *
 *  Created on: 2016��7��25��
 *      Author: houxd
 */

#ifndef __CONVERT_H__
#define	__CONVERT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
	'\x19'	 <==>  19u
*/
extern int bcd2int(unsigned char bcd);
extern unsigned char int2bcd(int d);

/*
 * -9 -> '0'
 * 1 -> '1'
 * 10 -> 'a'
 * 15 -> 'f'
 * 100 -> 'f'
 */
extern char int2asc(int n);
/*
 * -9 -> '0'
 * 1 -> '1'
 * 10 -> 'A'
 * 15 -> 'F'
 * 100 -> 'F'
 */
extern char int2ASC(int n);
extern int asc2int(char asc);
/*
 * '0' -> 0
 * 'B' -> 11
 * 'c' -> 12
 */
extern int asc2int(char asc);
extern uint8_t* hx_dumphex(const void *src,int srclen,void *buf);
extern uint8_t* hx_dumpHEX(const void *src,int srclen,void *buf);
/*
 * like dumphex ,but add a '\0' end of converation
 */
extern char* hx_dumphex2str(const void *src,int srclen,void *buf);
extern char* hx_dumpHEX2str(const void *src,int srclen,void *buf);
/*
	"\xAA\x55"  <==>  "AA55"
*/	
extern char* hx_hexcode2bin(const void *hexcode, int len, void *bin);

/*
 * trim string. “t12t3t” trim 't' => "12t3"
 * hx_strtrim is trim "\t\r\n "
 */
extern char *hx_strtrim(char *s);
extern char *hx_strtrim2(char *p, const char *trim_list);
#define strtrim 	hx_strtrim2
/*
 * uint8_t ARRAY <==>  uint16_t(2Bytes) uint32_t(4Bytes) QWORD(8Bytes)
 * MSB = Big Endian,
 * LSB = Littie Endian
 */
extern uint16_t HX_LSB_B2W(const void *data);
extern uint16_t HX_MSB_B2W(const void *data);
extern uint32_t HX_MSB_B2DW(const void *data);
extern uint32_t HX_LSB_B2DW(const void *data);
extern uint32_t HX_MSB_B2DW24(const void *data);
extern uint32_t HX_LSB_B2DW24(const void *data);
extern uint64_t HX_MSB_B2QW(const void *data);
extern uint64_t HX_LSB_B2QW(const void *data);
extern uint8_t *HX_LSB_W2B(uint16_t v,void *p);
extern uint8_t *HX_MSB_W2B(uint16_t v,void *p);
extern uint8_t *HX_MSB_DW2B(uint32_t v,void *p);
extern uint8_t *HX_LSB_DW2B(uint32_t v,void *p);
extern uint8_t * HX_MSB_DW24_2B(uint32_t v,void *p);
extern uint8_t * HX_LSB_DW24_2B(uint32_t v,void *p);
extern uint8_t *HX_MSB_QW2B(uint64_t v,void *p);
extern uint8_t *HX_LSB_QW2B(uint64_t v,void *p);
extern void HX_W_REV(uint16_t *v);
extern void HX_DW_REV(uint32_t *v);
extern void HX_QW_REV(uint64_t *v);
extern uint64_t HX_W_REV2(uint16_t v);
extern uint64_t HX_DW_REV2(uint32_t v);
extern uint64_t HX_QW_REV2(uint64_t v);

/*
 * bcc8, sum8 ,crc16 checks.
 */
extern uint8_t make_bcc(const void *data, int len);
extern uint8_t make_bcc2(uint8_t init, const void *data, int len);
extern uint8_t make_sum(const void *buf, int len);
extern uint8_t make_sum2(uint8_t init,const void *buf, int len);
extern uint16_t calc_crc(const uint8_t *data, int length);
extern uint16_t calc_crc2(uint16_t init, const void *data, int length);
extern void des_enc(const void *input8byte,void *output8byte,const void *key8byte);
extern void des_dec(const void *input8byte,void *output8byte,const void *key8byte);

/*
 * use for time compare.
 */
extern int ymd2days(int y,int m,int d);
extern long long ymdhms2sec(int y,int m,int d,int H,int M,int S);
extern int ymdbcd2days(uint8_t *yyyymmdd);
extern long long ymdhmsbcd2sec(uint8_t *yyyymmddHHMMSS);

/*
 * pack data calls
 */
extern void* pk_fill(void *to, int len, int d);
extern void* pk_add(void *to, int len, const void *from);
extern void* pk_get(void*from,int len,void* to);

/*
 * Debug info for packet add/get/fill
 */
#ifndef HX_DBG_PRINT
#define  HX_DBG_PRINT(...)    
#endif

#define PK_SHOW_TAB_SIZE			(8)
#define PK_SHOW_NAME_LEN			(PK_SHOW_TAB_SIZE*4)
#define PK_DEBUG_FILL(p,n,x)	\
{\
	const char *name = #x;\
	int len = strlen(name);\
	len = PK_SHOW_NAME_LEN - len;\
	HX_DBG_PRINT("\t%s",name);\
	for(int i=0;i<len;i+=PK_SHOW_TAB_SIZE)\
		HX_DBG_PRINT("\t");\
	HX_DBG_PRINT("%d\t ",(int)(n));\
	HX_DBG_PRINT("{%02X}",(int)(x));\
	HX_DBG_PRINT("\r\n");\
}
#define PK_DEBUG_ARRAY(p,n,x)	\
{\
	const char *name = #x;\
	int len = strlen(name);\
	len = PK_SHOW_NAME_LEN - len;\
	HX_DBG_PRINT("\t%s",name);\
	for(int i=0;i<len;i+=PK_SHOW_TAB_SIZE)\
		HX_DBG_PRINT("\t");\
	HX_DBG_PRINT("%d\t ",(int)(n));\
	for(int i=0;i<n;i++){\
		HX_DBG_PRINT("%02X",(int)(((char*)(x))[i]));\
	}\
	HX_DBG_PRINT("("); \
	for(int i=0;i<n;i++){\
		char c = ((char*)(x))[i]; \
		if(hx_isprint(c)) \
			HX_DBG_PRINT("%c",c); \
		else \
			HX_DBG_PRINT(".");\
	}\
	HX_DBG_PRINT(")"); \
	HX_DBG_PRINT("\r\n");\
}
#define PK_DEBUG_BYTE(p,x)	\
{\
	const char *name = #x;\
	int len = strlen(name);\
	len = PK_SHOW_NAME_LEN - len;\
	HX_DBG_PRINT("\t%s",name);\
	for(int i=0;i<len;i+=PK_SHOW_TAB_SIZE)\
		HX_DBG_PRINT("\t");\
	HX_DBG_PRINT("1\t %02X(%u)\r\n",(int)(x),(int)x);\
}
#define PK_DEBUG_WORD(p,x)	\
{\
	const char *name = #x;\
	int len = strlen(name);\
	len = PK_SHOW_NAME_LEN - len;\
	HX_DBG_PRINT("\t%s",name);\
	for(int i=0;i<len;i+=PK_SHOW_TAB_SIZE)\
		HX_DBG_PRINT("\t");\
	HX_DBG_PRINT("2\t %04X\r\n",(int)(x));\
}
#define PK_DEBUG_DWORD(p,x)	\
{\
	const char *name = #x;\
	int len = strlen(name);\
	len = PK_SHOW_NAME_LEN - len;\
	HX_DBG_PRINT("\t%s",name);\
	for(int i=0;i<len;i+=PK_SHOW_TAB_SIZE)\
		HX_DBG_PRINT("\t");\
	HX_DBG_PRINT("4\t %08X\r\n",(int)(x));\
}

#define PK_ADD_ARRAY(p,n,x)	\
{\
	p = pk_add((p), (n), (x));	\
	PK_DEBUG_ARRAY((p),(n),(x));	\
}	
#define PK_FILL(p,n,x)	\
{\
	p = pk_fill((p), (n), (x));	\
	PK_DEBUG_FILL((p),(n),(x));	\
}	
#define PK_ADD_BYTE(p,x)	\
{\
	*p++ = (x); \
	PK_DEBUG_BYTE((p),(x));	\
}	
#define PK_ADD_WORD(p,x)	\
{\
	HX_MSB_W2B(x,(p)); \
	p += 2; \
	PK_DEBUG_WORD((p),(x));	\
}	
#define PK_ADD_DWORD(p,x)	\
{\
	HX_MSB_DW2B((x),(p)); \
	p += 4; \
	PK_DEBUG_DWORD((p),(x));	\
}	
#define PK_ADD_DWORD_LSB(p,x)	\
{\
	HX_LSB_DW2B((x),(p)); \
	p += 4; \
	PK_DEBUG_DWORD((p),(x));	\
}	




#define PK_DEBUG_BYTE_ERROR(p,x)\
{\
	const char *name = #x;\
	HX_DBG_PRINT("\t%s !!! want %02X, get %02X.\r\n",name,(int)(x),(int)*(p));\
}
#define PK_DEBUG_WORD_ERROR(v,x)\
{\
	const char *name = #x;\
	HX_DBG_PRINT("\t%s !!! want %04X, get %04X.\r\n",name,(int)(x),(int)(v));\
}
#define PK_DEBUG_DWORD_ERROR(v,x)\
{\
	const char *name = #x;\
	HX_DBG_PRINT("\t%s !!! want %08X, get %08X.\r\n",name,(int)(x),(int)(v));\
}
#define PK_DEBUG_ARRAY_ERROR(p,n,x)\
{\
	const char *name = #x;\
	HX_DBG_PRINT("\t%s !!! want ",name);\
	for(int i=0;i<(n);i++)\
		HX_DBG_PRINT("%02X", (int)(((char*)(x))[i]));\
	HX_DBG_PRINT(", get ");\
	for(int i=0;i<(n);i++)\
		HX_DBG_PRINT("%02X", (int)(((char*)(p))[i]));\
	HX_DBG_PRINT(".\r\n");\
}

#define PK_WANT_BYTE(p,x,callback)	\
{\
	if(*(p) != (x)){\
		PK_DEBUG_BYTE_ERROR((p),(x));\
		callback;\
	}else{\
		(p)++;\
		PK_DEBUG_BYTE((p),(x));\
	}\
}
#define PK_WANT_WORD(p,x,callback)	\
{\
	int v = HX_MSB_B2W(p);\
	if(v != (x)){\
		PK_DEBUG_WORD_ERROR((v),(x));\
		callback;\
	}else{\
		p+=2;\
		PK_DEBUG_WORD((v),(x));\
	}\
}

#define PK_WANT_DWORD(p,x,callback)	\
{\
	int v = HX_MSB_B2DW(p);\
	if(v != (x)){\
		PK_DEBUG_DWORD_ERROR(v,(x));\
		callback;\
	}else{\
		p+=4;\
		PK_DEBUG_DWORD(v,(x));\
	}\
}
#define PK_WANT_ARRAY(p,n,x,callback)	\
{\
	if(memcmp(p,(x),(n))){\
		PK_DEBUG_ARRAY_ERROR((p),(n),(x));\
		callback;\
	}else{\
		p+=n;\
		PK_DEBUG_ARRAY((p),(n),(x));\
	}\
}
#define PK_GET_BYTE(p,x)	\
{\
	(x) = *p++;\
	PK_DEBUG_BYTE(p,(x));\
}
#define PK_GET_WORD(p,x)	\
{\
	(x) = HX_MSB_B2W(p);\
	p+=2;\
	PK_DEBUG_WORD((p),(x));\
}
#define PK_GET_DWORD(p,x)	\
{\
	(x) = HX_MSB_B2DW(p);\
	(p)+=4;\
	PK_DEBUG_DWORD((p),(x));\
}
#define PK_COPY_ARRAY(p,n,x)	\
{\
	memcpy((x),(p),(n));\
	(p)+=(n);\
	PK_DEBUG_ARRAY((p),(n),(x));\
}
#define PK_REF_ARRAY(p,n,x)	\
{\
	(x)=p;\
	(p)+=(n);\
	PK_DEBUG_ARRAY((p),(n),(x));\
}
#define PK_GET_ARRAY(p,n,x)		PK_COPY_ARRAY(p,n,x)



#ifdef __cplusplus
}
#endif


#endif
