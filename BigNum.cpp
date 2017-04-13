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





