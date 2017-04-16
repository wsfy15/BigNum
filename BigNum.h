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

#ifdef _MSC_VER
#define LL(v)   (v##ui64)
#else
#define LL(v)   (v##ULL)
#endif

#define BN_HAVE_LONG_LONG		//有双精度变量存在的环境（64位系统）

#ifdef BN_HAVE_LONG_LONG
#define BN_MASK0   LL(0xFFFFFFFFFFFFFFFF)
#else
#define BN_MASK0   0xFFFFFFFFUL
#endif

/*
	A. 单精度和双精度变量都存在的环境：
1. ADDC_INIT 定义双精度类型的变量 rr 用于存放累加的结果。
用双精度的原因是单精度累加后结果可能要用双精度类型才能完整存储，简单的数学证明如下：

设单精度类型是一个 n 位的整形，则能表示的最大无符号整数是：2^n - 1，两个最大整数相加，
结果是：2 * (2^n - 1) = 2^(n + 1) - 2，结果超出单精度类型所能表示的范围，因此至少需要双精度类型才能完整表示。

2. 在 ADDC_CORE 中，先计算 x 和 y 每一位的和，并和来自低位的进位相加，结果放在 rr 中；
然后将 rr 和掩码 BN_MASK0 相与提取出本位，存放在目标整数的对应数位上；
最后将变量 rr 右移 biL 位计算出进位结果。记住 rr 是双精度的，
累加结果的低半部分是本位结果，高半部分是进位结果。

3. 在 ADDC_FINAL 中，数位多的整数的剩余数位与来自低位的进位相加，
然后从结果中提取本位存储到目标整数的对应数位上，最后 rr 右移 biL 位计算新的进位值。

4. ADDC_STOP 宏将剩余的最后一个进位传递到结果的最高数位上。注意，这个进位可能是 0，也有可能不是 0。

	B. 只有单精度的环境：
1. ADDC_INIT 宏定义了三个变量：累加的本位结果 rr， 临时变量 t， 进位值 c。

2. 这里的 ADDC_CORE 计算稍微复杂点。第一步，将 x 的某一位存放到临时变量 t 中。
第二步，变量 t 和来自低位的进位 c 相加，得到结果的本位值，存放于变量 t 中。
这里要注意的是所有变量都是单精度类型，如果结果大于单精度数所能表示的范围，则会产生溢出，
相当于做了一次 mod 2^n 运算，所以得到的是结果的本位值。
第三步，判断前一步是否产生了进位。在加法中，如果结果产生进位，则本位的值小于两个加数。
所以，如果有进位，则 t < c，比较的结果为 1。
第四步，计算 t 和第二个整数对应数位的和，结果的本位存放于变量 rr 中。
第五步，判断是否产生进位，原理和第三步一样。最后一步，将最终的本位结果送到目标整数的对应数位上。

3. ADDC_FINAL 用于传递剩余的进位，和上面的 ADDC_FINAL 原理一样，只不过是本位和进位是用两个变量表示而已。

4. ADDC_STOP 宏将剩余的最后一个进位传递到结果的最高数位上。仍然要注意，这个进位可能是 0，也有可能不是 0。
*/

#ifdef BN_HAVE_LONG_LONG
#define ADDC_INIT                                  \
    bn_udbl rr = 0;                                \

#define ADDC_CORE                                  \
    rr += (bn_udbl)(*px) + (*py);                  \
    *pz = (bn_digit)(rr & BN_MASK0);               \
    rr >>= biL;                                    \

#define ADDC_FINAL                                 \
    rr += (bn_udbl)(tmp->dp[i]);                   \
    *pz = (bn_digit)(rr & BN_MASK0);               \
    rr >>= biL;                                    \

#define ADDC_STOP                                  \
    z->dp[max] = (bn_digit)(rr & BN_MASK0);        \

#else
#define ADDC_INIT                   \
    bn_digit rr, t, c = 0;          \

#define ADDC_CORE                   \
    t = *px;                        \
    t = (t + c) & BN_MASK0;         \
    c = (t < c);                    \
    rr = (t + *py) & BN_MASK0;      \
    c += (rr < t);                  \
    *pz = rr;                       \

#define ADDC_FINAL                  \
    t = tmp->dp[i];                 \
    t = (t + c) & BN_MASK0;         \
    c = (t < c);                    \
    *pz = t;                        \

#define ADDC_STOP                   \
    z->dp[max] = c;                 \

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

#define BN_IS_ZERO(x)                 ((x->used == 0) ? 1 : 0)
#define BN_MIN(x, y)                  (((x) < (y)) ? (x) : (y)) 

void bn_init(bignum *x);
int bn_init_size(bignum *x, size_t nlimbs);

#define IN_X64							//只有单精度变量的情况
//#define IN_X86						//单双精度变量都有的情况

#ifdef IN_X64
#define COMBA_INIT                                  \
{                                                   \
    bn_digit a0, a1, b0, b1;                        \
    bn_digit t0, t1, r0, r1;                        \

#define COMBA_MULADDC                               \
                                                    \
    a0 = (*px << biLH) >> biLH;                     \
    b0 = (*py << biLH) >> biLH;                     \
    a1 = *px++ >> biLH;                             \
    b1 = *py-- >> biLH;                             \
    r0 = a0 * b0;                                   \
    r1 = a1 * b1;                                   \
    t0 = a1 * b0;                                   \
    t1 = a0 * b1;                                   \
    r1 += (t0 >> biLH);                             \
    r1 += (t1 >> biLH);                             \
    t0 <<= biLH;                                    \
    t1 <<= biLH;                                    \
    r0 += t0;                                       \
    r1 += (r0 < t0);                                \
    r0 += t1;                                       \
    r1 += (r0 < t1);                                \
    c0 += r0;                                       \
    c1 += (c0 < r0);                                \
    c1 += r1;                                       \
    c2 += (c1 < r1);                                \

#define COMBA_STOP                                  \
}

#else

#define COMBA_INIT                                  \
{                                                   \
    bn_udbl r;                                      \

#define COMBA_MULADDC                               \
                                                    \
    r = (bn_udbl)(*px++) * (*py--) + c0;            \
    c0 = (bn_digit)r;                               \
    r = (bn_udbl)c1 + (r >> biL);                   \
    c1 = (bn_digit)r;                               \
    c2 += (bn_digit)(r >> biL);                     \

#define COMBA_STOP                                  \
}

#endif