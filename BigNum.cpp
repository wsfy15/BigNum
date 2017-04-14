#include"BigNum.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

void bn_init(bignum *x);			
int bn_init_size(bignum *x, size_t nlimbs);	
void bn_free(bignum *x);
int bn_grow(bignum* x, size_t nlimbs);
int bn_copy(bignum *x, const bignum *y);
int bn_set_word(bignum *x, const bn_digit word);
void bn_swap(bignum *x, bignum *y);
void bn_clamp(bignum *x);
int bn_cmp_abs(const bignum *x, const bignum *y);
int bn_cmp_bn(const bignum *x, const bignum *y);
boolean bn_get_bit(const bignum *x, const size_t pos);
int bn_set_bit(bignum *x, const size_t pos, boolean value);
size_t bn_lsb(const bignum *x);
size_t bn_msb(const bignum *x);
size_t bn_size(const bignum *x);




//bignum�ṹ��ʼ����δ�����ڴ档
void bn_init(bignum *x)
{
	if (x != NULL)
	{
		x->alloc = 0;
		x->sign = 1;
		x->used = 0;
		x->dp = NULL;
	}
}//bn_init

/*
 bignum�ṹ��ʼ���������ڴ档
 ����ʼ��ʧ�ܣ��ڴ������󣩣�����BN_MEMORY_ALLOCATE_FAIL
 ��������λ���ޣ�����BN_EXCEED_MAX_LIMB
*/
int bn_init_size(bignum *x, size_t nlimbs)
{
	if (nlimbs > BN_MAX_LIMB)
		return BN_EXCEED_MAX_LIMB;

	bn_digit *p;
	if (x != NULL)
	{
		p = (bn_digit*)BN_MALLOC(nlimbs*ciL);
		if (p == NULL)
			return BN_MEMORY_ALLOCATE_FAIL;
		memset(p, 0x00, nlimbs*ciL);
		x->sign = 1;
		x->alloc = nlimbs;
		x->used = 0;
		x->dp = p;
	}
	return 0;
}//bn_init_size

void bn_free(bignum *x)
{
	if (x != NULL)
	{
		if (x->dp != NULL)
		{
			memset(x->dp, 0x00, x->alloc*ciL);
			BN_FREE(x->dp);
		}
		x->sign = 1;
		x->alloc = 0;		
		x->used = 0;
		x->dp = NULL;
	}

}//bn_free

/*
��������
bn_grow�������ȼ����Ҫ����λ��nlimbs���Ƿ����alloc��������ǣ����������Ӿ��ȣ��������ý���
���nlimbs����alloc����ô��Ҫ���·���һ�γ���Ϊnlimbs���ڴ�ռ䣬
��ʼ��Ϊ0���ԭ��dpָ����ڴ��е����ݸ��Ƶ��µĿռ��У������ͷ�ԭ��dpָ����ڴ档
*/
int bn_grow(bignum* x, size_t nlimbs)
{
	if (nlimbs > BN_EXCEED_MAX_LIMB)
		return BN_EXCEED_MAX_LIMB;

	bn_digit *p;
	if (nlimbs > x->alloc)
	{
		p = (bn_digit*)BN_MALLOC(nlimbs*ciL);
		if (p == NULL)
			return BN_MEMORY_ALLOCATE_FAIL;

		memset(p, 0, nlimbs*ciL);
		if (x->dp != NULL)
		{
			memcpy(p, x->dp, ciL*x->alloc);
			memset(x->dp, 0, ciL*x->alloc);
			BN_FREE(x->dp);
		}
		x->dp = p;
		x->alloc = nlimbs;
	}
	return 0;
}//bn_grow


/*
���Ʋ���
��bignum y���Ƹ�bignum x��ע�������x->dpָ���µ��ڴ棬������ָ��y->dp��
*/
int bn_copy(bignum *x, const bignum *y)
{
	int ret;
	size_t nlimbs;

	if (x == y)
		return 0;

	x->sign = y->sign;
	x->used = y->used;
	nlimbs = (y->used == 0) ? 1 : y->used;		//��� y Ϊ0��Ĭ�ϸ� x ����һλ�Ŀռ�
	BN_CHECK(bn_grow(x, nlimbs));
	
	memset(x->dp, 0, ciL*nlimbs);
	if (y->dp != NULL && y->used != 0)
		memcpy(x->dp, y->dp, ciL*y->used);

clean:
	return ret;
}//bn_copy

/*�����ȸ�ֵ����
��bignum���ó�һ����Խ�С�ĵ���������(0 -- 2^32 - 1) 
ͨ������1����λ���ڴ棬�ѵ���������ֵ��������׸���Ԫ���ɡ�
��������Ĭ�ϵĵ����������޷��ŵģ����Ǹ��������������Ҫ��ֵһ������������ֱ�Ӱ�sign��Ϊ-1���ɡ�
*/
int bn_set_word(bignum *x, const bn_digit word)
{
	int ret;
	BN_CHECK(bn_grow(x, 1));	//����һ����λ���ڴ�

	memset(x->dp, 0, ciL*x->alloc);
	x->dp[0] = word;
	x->used = 1;
	x->sign = 1;		//����Ǹ������ں�����������sign���-1����

clean:
	return ret;
}//bn_set_word

/*
��������
�⺯�������úܼ򵥣����ǽ������������� x �� y�������ʵ��ԭ��Ҳ�ܼ򵥣������ṹ���е�ÿ����Ա����
*/
void bn_swap(bignum *x, bignum *y)
{
	int tmp_sign;
	size_t tmp_alloc;
	size_t tmp_used;
	bn_digit *tmp_dp;

	tmp_sign = x->sign;
	tmp_alloc = x->alloc;
	tmp_used = x->used;
	tmp_dp = x->dp;

	x->sign = y->sign;
	x->alloc = y->alloc;
	x->used = y->used;
	x->dp = y->dp;

	y->sign = tmp_sign;
	y->alloc = tmp_alloc;
	y->used = tmp_used;
	y->dp = tmp_dp;
}//bn_swap

/*
ѹ������λ
��һ��������Ԥ�����Ϊ n λʱ����򵥵������Ǽٶ���������ʹ�õ�λ������ n λ��
�������ڼ�������м�����Ƿ���ˡ�����һ�� a λ�Ĵ��������� b λ�Ĵ�������������Ϊ a + b λ��
���ǽ����ȫ�п�����һ�����û�н�λ�� a + b - 1 λ����
�������ڴ�Ž����ĳһ�����ȱ���չ�� a + b - 1 λ��Ȼ����ֽ�λ�������չ 1 λ���⽫�˷Ѻܶ�ʱ�䣬
��Ϊ���ϵĲ����Ƚ�����������õİ취�Ǽٶ����Ϊ a + b λ���ں����������� used��
����ֻ��һ���ڴ���䣬Ч����ߣ�Ȼ��������Խ�����м�飬��λ���ܻ��ж���� 0��
����ķ����ܼ򵥣���дһ���򵥵Ķ���λѹ���㷨���ɡ�
*/
void bn_clamp(bignum *x)
{
	while (x->used > 0 && x->dp[x->used - 1] == 0)
		x->used--;

	if (x->used == 0)
		x->sign = 1;
}//bn_clamp

/*
����ֵ�Ƚϣ��з������Ƚϣ��ྫ�����͵��������Ƚϡ�
Լ�������еıȽϲ�������� x > y������ 1��x == y������ 0��x < y������ -1��
*/

/*
�ȱȽ�λ����λ����ľ���ֵ�������λ����ͬ���Ӹ�λ����λ���αȽ���������������ͬλ����λ��С
*/
int bn_cmp_abs(const bignum *x, const bignum *y)
{
	size_t i;

	if (x->used > y->used) return  1;
	if (x->used < y->used) return -1;

	//x �� y ��λ����ͬ�����αȽ���ͬλ�Ĵ�С��
	for (i = x->used - 1; i >= 0; i--)
	{
		if (x->dp[i] > y->dp[i]) return  1;
		if (x->dp[i] < y->dp[i]) return -1;
	}
	return 0;
}//bn_cmp_abs

/*
�з������Ƚ�
1. x �� y ��������ţ��Ǹ������ڸ�����
2. x �� y ������ͬ�ţ��� x �� y ���ǷǸ��������Ƚ� x �� y �ľ���ֵ��С��
�� x �� y ���Ǹ��������Ƚ� y �� x �ľ���ֵ��С��
*/

int bn_cmp_bn(const bignum *x, const bignum *y)
{
	if (x->sign == 1 && y->sign == -1) return  1;
	if (y->sign == 1 && x->sign == -1) return -1;

	if (x->sign == 1)			//��������
		return bn_cmp_abs(x, y);
	else						//���Ǹ���
		return bn_cmp_abs(y, x);
}//bn_cmp_bn

/*
�ྫ�����͵��������Ƚ�
����˵���ǽ�һ����������һ���з��ŵĵ���������bn_sint���ͣ����бȽϡ�
����ǰ��Ĵ������������֮����з��űȽϲ�����ֱ�Ӱ��з��ŵĵ���������ֵ��һ����ʱ�Ĵ��������ͱ���t��
Ȼ�����bn_cmp_bn�������бȽϡ�
*/
int bn_cmp_int(const bignum *x, const bn_sint y)
{
	bignum t[1];
	bn_digit p[1];

	p[0] = (y >= 0) ? y : -y;
	t->sign = (y >= 0) ? 1 : -1;
	t->used = (y == 0) ? 0 : 1;   //ע�� bignum Ϊ 0 ʱ���涨 used = 0.
	t->alloc = 1;
	t->dp = p;

	return bn_cmp_bn(x, t);
}//bn_cmp_int

//����ָ���±� pos �ı���λ
boolean bn_get_bit(const bignum *x, const size_t pos)
{
	if (x->alloc*biL <= pos)
		return 0;
	boolean bit;

	//bit = 1 & (x->dp[pos / biL] >> (pos%biL));		//ÿ����λ�ı��ش�0 �� 31 
	bit = (x->dp[pos / biL] >> (pos & (biL - 1))) & 1;	//ֱ����λ�������
	//�� 2 �Ĳ��������У����� a mod (2^n) �ȼ��� a AND (2^n - 1)��

	return bit;
}//bn_get_bit

/*
�����±�ֵ pos ���ö�Ӧλ�õı���λ
���ҵ� pos ��Ӧ�ı���λ��λ�á����� pos = 35���� offset = pos / biL = 1�� mod = pos / biL = 3��
�ҵ���Ӧ��λ�ú�Ҫ�ȰѸ�λ��ֵ��գ�����������ǣ��� 1 ���� mod = 3 λ��Ȼ��ȡ����
��ʱ��������Ķ����ƾͻ��� 1111 1111 1111 1111 1111 1111 1111 0111��
��������ĵڶ�����λ����Ӧ�����±�Ϊ 1�����������㣬����ո�λ��ֵ��
Ȼ�󽫺�������� boolean ���͵� value ���� mod = 3 λ����������ĵڶ�����λ��������
���ɰ��µı���λ���úá�
*/
int bn_set_bit(bignum *x, const size_t pos, boolean value)
{
	int ret = 0;
	size_t offset, mod;

	//pos is zero base number

	if (value != TRUE && value != FALSE)
		return BN_INVALID_INPUT;

	offset = pos / biL;
	mod = pos & (biL - 1);

	if (pos >= x->alloc * biL)
	{
		if (value == FALSE) return 0; 
		//���pos�����˷������λ����valueΪ0���������������Ϊ��λĬ��Ϊ0��������Ҫ���Ӿ��ȡ�
		BN_CHECK(bn_grow(x, offset + 1));
	}

	x->dp[offset] &= ~((bn_digit)1 << mod);				//��ָ��λ����
	if(value != FALSE)
		x->dp[offset] |= ((bn_digit)1 << mod);		//��ָ��λ����

	x->used = x->alloc;  
	//���bignum�ľ������ӣ�����Ҫ������ߵ���λ��ʼ���Ҽ����Ч��λ����֤usedֵ����ȷ��
	bn_clamp(x);  //ѹ������λ

clean:
	return ret;
}//bn_set_bit



/*
����  ��Ч����  λ   ������
����㷨���������ڿ��Լ����һ�� bignum ��ʵ�ʱ��ش�С
��Ч����λ��ָ�������һ����Ϊ 0 �ı���λ��ʼ�����ұ���λΪֹ���м����б���λ��
*/
size_t bn_msb(const bignum *x)
{
	size_t i, j;

	i = x->used - 1;
	//�����λ����ȫΪ0
	for (j = biL; j > 0; j--)
		if (((x->dp[i] >> (j - 1)) & 1) != 0)
			break;

	return biL * i + j;
}//bn_msb

/*
���������Ч����λǰ 0 ������
�����Ч����λ��ָ�������һ����Ϊ 0 �ı���λ
*/
size_t bn_lsb(const bignum *x)
{
	size_t i, j;

	for (i = 0; i < x->used; i++)
	{
		if (x->dp[i] != 0)
		{
			for (j = 0; j < biL; j++)
			{
				if (((x->dp[i] >> j) & 1) != 0)
					return i * biL + j;
			}
		}
	}
	return 0;
}//bn_lsb

/*
���� bignum ���ֽڴ�С
����Ҫע����ǣ�����������Ƿ��� bignum ռ���˶����ڴ棬������� bignum x��
��ʹһ��ʼ������ 3 ����Ԫ���ڴ棬���ֽڴ�С��Ȼ�� 8 byte��
���� bignum ���ֽڴ�С��ֻ��Ҫ�� x �ı��ش�С���� 7 ���� 8 ���ɡ�
���� 7 ��Ŀ���Ǳ��� x �ı���λ���� 8 �ı���ʱ����֮������һ���ֽ�
*/
size_t bn_size(const bignum *x)
{
	return ((bn_msb(x) + 7) >> 3);
}//bn_size





