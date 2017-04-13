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

void bn_init(bignum *x);
int bn_init_size(bignum *x, size_t nlimbs);


