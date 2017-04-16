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

#define BN_HAVE_LONG_LONG		//��˫���ȱ������ڵĻ�����64λϵͳ��

#ifdef BN_HAVE_LONG_LONG
#define BN_MASK0   LL(0xFFFFFFFFFFFFFFFF)
#else
#define BN_MASK0   0xFFFFFFFFUL
#endif

/*
	A. �����Ⱥ�˫���ȱ��������ڵĻ�����
1. ADDC_INIT ����˫�������͵ı��� rr ���ڴ���ۼӵĽ����
��˫���ȵ�ԭ���ǵ������ۼӺ�������Ҫ��˫�������Ͳ��������洢���򵥵���ѧ֤�����£�

�赥����������һ�� n λ�����Σ����ܱ�ʾ������޷��������ǣ�2^n - 1���������������ӣ�
����ǣ�2 * (2^n - 1) = 2^(n + 1) - 2����������������������ܱ�ʾ�ķ�Χ�����������Ҫ˫�������Ͳ���������ʾ��

2. �� ADDC_CORE �У��ȼ��� x �� y ÿһλ�ĺͣ��������Ե�λ�Ľ�λ��ӣ�������� rr �У�
Ȼ�� rr ������ BN_MASK0 ������ȡ����λ�������Ŀ�������Ķ�Ӧ��λ�ϣ�
��󽫱��� rr ���� biL λ�������λ�������ס rr ��˫���ȵģ�
�ۼӽ���ĵͰ벿���Ǳ�λ������߰벿���ǽ�λ�����

3. �� ADDC_FINAL �У���λ���������ʣ����λ�����Ե�λ�Ľ�λ��ӣ�
Ȼ��ӽ������ȡ��λ�洢��Ŀ�������Ķ�Ӧ��λ�ϣ���� rr ���� biL λ�����µĽ�λֵ��

4. ADDC_STOP �꽫ʣ������һ����λ���ݵ�����������λ�ϡ�ע�⣬�����λ������ 0��Ҳ�п��ܲ��� 0��

	B. ֻ�е����ȵĻ�����
1. ADDC_INIT �궨���������������ۼӵı�λ��� rr�� ��ʱ���� t�� ��λֵ c��

2. ����� ADDC_CORE ������΢���ӵ㡣��һ������ x ��ĳһλ��ŵ���ʱ���� t �С�
�ڶ��������� t �����Ե�λ�Ľ�λ c ��ӣ��õ�����ı�λֵ������ڱ��� t �С�
����Ҫע��������б������ǵ��������ͣ����������ڵ����������ܱ�ʾ�ķ�Χ�������������
�൱������һ�� mod 2^n ���㣬���Եõ����ǽ���ı�λֵ��
���������ж�ǰһ���Ƿ�����˽�λ���ڼӷ��У�������������λ����λ��ֵС������������
���ԣ�����н�λ���� t < c���ȽϵĽ��Ϊ 1��
���Ĳ������� t �͵ڶ���������Ӧ��λ�ĺͣ�����ı�λ����ڱ��� rr �С�
���岽���ж��Ƿ������λ��ԭ��͵�����һ�������һ���������յı�λ����͵�Ŀ�������Ķ�Ӧ��λ�ϡ�

3. ADDC_FINAL ���ڴ���ʣ��Ľ�λ��������� ADDC_FINAL ԭ��һ����ֻ�����Ǳ�λ�ͽ�λ��������������ʾ���ѡ�

4. ADDC_STOP �꽫ʣ������һ����λ���ݵ�����������λ�ϡ���ȻҪע�⣬�����λ������ 0��Ҳ�п��ܲ��� 0��
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
//�����������λ���ͣ�ͨ������³���Ϊ����ϵͳ���ֳ����൱��һ���޷��ŵĵ���������.

typedef uint64 bn_udbl;         
//����˫�ֳ��޷������Σ�32λϵͳ�¾���64λ���൱��һ��˫��������.

typedef int32  bn_sint;            
//�з��ŵ��������ͣ�����Ϊ����ϵͳ���ֳ�.

#define BN_MAX_LIMB		25600		//��λ����ֵ

typedef struct
{
	int sign;             //���ű�ʶ��1��ʾ�Ǹ�������-1��ʾ��������
	size_t alloc;       //��ʾ����dp�����Ӵ�С֮ǰ�Ŀ�����λ��alloc�������ǷǸ�������
	size_t used;       //��ʾ����dp���˶���λ����ʾһ��ָ����������used�������ǷǸ�������
	bn_digit *dp;    //ָ��dpָ��̬����Ĵ���ָ���ྫ�����������飬����λ��alloc-used��ȫ����0�������е����ݰ���LSB˳��洢���͵�ַ�����λ��
} bignum;

/*
��Ч��bignum�ṹ��

���ǵ�Ч�ʻ��д���Ľ�׳�ԣ���bignum�ṹ��״ָ̬���˼�������ĳЩ��������⣩��

1. alloc��ֵΪ�Ǹ�������Ҳ����˵��dpҪôָ��һ��Ԥ�ȷ����пռ�����飬ҪôΪNULL��

2. used��ֵΪ�Ǹ���������used<=alloc��

3.used��ֵ��ʾ������dp�еĵ�used-1λ������Ϊ0��Ҳ����˵��0Ϊ�׵ĸ�λ���뱻�ضϣ�����dp�е�used�������ϵ�λ�ñ�����Ϊ0��

4. ���used��0����sign = 1����ʱbignum��ֵΪ0����dp������ʼ����û�����ڴ档
*/

//Error Value
#define BN_MEMORY_ALLOCATE_FAIL                 -0x0001         //��̬�ڴ�������
#define BN_EXCEED_MAX_LIMB                      -0x0002         //���������λ
#define BN_EXCEED_MAX_BITS                      -0x0003         //�������ı���λ
#define BN_NEGATIVE_VALUE_ERROR                 -0x0004         //��������
#define BN_INVALID_INPUT                        -0x0005         //��Ч����
#define BN_DIVISION_BY_ZERO                     -0x0006         //����0����
#define BN_BUFFER_TOO_SMALL                     -0x0007         //������̫С
#define BN_READ_FILE_ERROR                      -0x0008         //���ļ�����
#define BN_WRITE_FILE_ERROR                     -0x0009         //д�ļ�����
#define BN_NO_MODULAR_INVERSE                   -0x000A         //ģ�治����

//������꣺

#define BN_CHECK(x)                   if((ret = x) != 0){ goto clean; }

#define ciL					(sizeof(bn_digit))       //(4)  bytes in limb ��ÿ����λ���ֽڴ�С��
#define biL					(ciL << 3)               //(4*8) bits in limb ��ÿ����λ��bit��С��
#define biLH				(ciL << 2)               
//(4*4)  half bits in limb  ��ÿ����λ��bit��С��һ�룬��������ģ������õ���  
#define BN_MALLOC			 malloc             
#define BN_FREE              free               

#define BN_IS_ZERO(x)                 ((x->used == 0) ? 1 : 0)
#define BN_MIN(x, y)                  (((x) < (y)) ? (x) : (y)) 

void bn_init(bignum *x);
int bn_init_size(bignum *x, size_t nlimbs);

#define IN_X64							//ֻ�е����ȱ��������
//#define IN_X86						//��˫���ȱ������е����

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