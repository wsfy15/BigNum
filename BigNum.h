#pragma once

#ifdef _MSC_VER
typedef __int8              int8;
typedef __int16             int16;
typedef __int32             int32;
typedef __int64             int64;

typedef unsigned __int8     uint8;
typedef unsigned __int16    uint16;
typedef unsigned __int32    uint32;
typedef unsigned __int64    uint64;
#else
typedef signed char         int8;
typedef signed short        int16;
typedef signed int          int32;
typedef signed long long    int64;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;
#endif

typedef uint8 boolean;
#define TRUE                  1
#define FALSE                 0

typedef uint32 bn_digit;         
//定义大整数数位类型，通常情况下长度为操作系统的字长，相当于一个无符号的单精度类型.

typedef uint64 bn_udbl;         
//定义双字长无符号整形，32位系统下就是64位，相当于一个双精度类型.

typedef int32  bn_sint;            
//有符号单精度类型，长度为操作系统的字长.

#define BN_MAX_LIMB		25600		//数位上限值

typedef struct
{
	int sign;             //符号标识，1表示非负整数，-1表示负整数。
	size_t alloc;       //表示数组dp在增加大小之前的可用数位，alloc数必须是非负整数。
	size_t used;       //表示数组dp用了多少位来表示一个指定的整数，used数必须是非负整数。
	bn_digit *dp;    //指针dp指向动态分配的代表指定多精度整数的数组，不足位（alloc-used）全部置0，数组中的数据按照LSB顺序存储（低地址保存低位）
} bignum;

/*
有效的bignum结构：

考虑到效率还有代码的健壮性，给bignum结构的状态指定了几个规则（某些情况下例外）：

1. alloc的值为非负整数，也就是说，dp要么指向一个预先分配有空间的数组，要么为NULL。

2. used的值为非负整数，且used<=alloc。

3.used的值暗示了数组dp中的第used-1位数不能为0，也就是说以0为首的高位必须被截断，数组dp中第used个及以上的位置必须置为0。

4. 如果used是0，则sign = 1，此时bignum的值为0或者dp仅仅初始化但没分配内存。
*/

//Error Value
#define BN_MEMORY_ALLOCATE_FAIL                 -0x0001         //动态内存分配错误
#define BN_EXCEED_MAX_LIMB                      -0x0002         //超出最大数位
#define BN_EXCEED_MAX_BITS                      -0x0003         //超出最大的比特位
#define BN_NEGATIVE_VALUE_ERROR                 -0x0004         //负数错误
#define BN_INVALID_INPUT                        -0x0005         //无效输入
#define BN_DIVISION_BY_ZERO                     -0x0006         //除以0错误
#define BN_BUFFER_TOO_SMALL                     -0x0007         //缓冲区太小
#define BN_READ_FILE_ERROR                      -0x0008         //读文件错误
#define BN_WRITE_FILE_ERROR                     -0x0009         //写文件错误
#define BN_NO_MODULAR_INVERSE                   -0x000A         //模逆不存在

//错误检查宏：

#define BN_CHECK(x)                   if((ret = x) != 0){ goto clean; }

#define ciL					(sizeof(bn_digit))       //(4)  bytes in limb （每个数位的字节大小）
#define biL					(ciL << 3)               //(4*8) bits in limb （每个数位的bit大小）
#define biLH				(ciL << 2)               
//(4*4)  half bits in limb  （每个数位的bit大小的一半，这个后面的模运算会用到）  
#define BN_MALLOC			 malloc             
#define BN_FREE              free               

void bn_init(bignum *x);
int bn_init_size(bignum *x, size_t nlimbs);


