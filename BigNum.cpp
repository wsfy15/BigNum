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
		x->dp[offset] |= ((bn_digit)1 << mod);		//����

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

/*
����λ�������д������ֲ�ͬ�Ĳ�����ʽ���߼���λ��������λ��
���߼���λ���У��Ƴ���λ��ʧ�������λȡ 0��
��������λ���У��Ƴ���λ��ʧ���������λȡ 0���������λȡ����λ�������λ��������ݷ��ű��ֲ��䡣
*/

/*
���ƺ����� b ����λ
����˵���ǳ��Ի���� 2 ^ (biL *b)����λ��������������λΪ��Ԫ���еġ�
*/

//���� b ����λ
int bn_lshd(bignum *x, size_t b)
{
/*
���� b ����λ����λ���� b λ������ used ֵҪ���� b��
ʹ�� bn_grow �������� bignum �ľ��ȡ�
Ȼ���� top ָ��ָ�� bignum ���ƺ�����λ��bottom ָ��ָ�� bignum ��ǰ�����λ��
Ȼ��ѭ�� used - b �ΰ�ÿһ����λ�����ƶ� b ����λ��
������������ top ָ��ָ�����λ��ѭ�� b �ΰ���͵� b ����λ�� 0�������λ����
*/
	int ret;
	size_t i;
	bn_digit *top, *bottom;

	x->used += b;
	BN_CHECK(bn_grow(x, x->used));

	top = x->dp + x->used - 1;
	bottom = x->dp + x->used - b - 1;

	for (i = x->used; i > b; i--)
		*top-- = *bottom--;

	top = x->dp;

	for (i = 0; i < b; i++)
		*top++ = 0;

clean:
	return ret;
}//bn_lshd

//���� b ����λ
int bn_rshd(bignum *x, size_t b)
{
/*
�����Ʋ�ͬ���ǣ����ƾ��Ȳ������ӣ������ used ��ֵС�ڵ��� b��
�� bignum �����λ������ұ߱��Ƴ�ȥ�����Ϊ 0��
��� used > b���� bottomָ�� bignum �������λ��top ָ��ָ��� b + 1 λ��
Ȼ��ѭ�� used - b �ν�ÿ����λ����Ų�� b ����λ��������ʣ��� b λȡ 0��������Ʋ�����
*/
	int ret = 0;
	size_t i;
	bn_digit *top, *bottom;

	if (x->used <= b)
	{
		BN_CHECK(bn_set_word(x, 0));
		return ret;
	}

	bottom = x->dp;
	top = x->dp + b;

	for (i = 0; i < x->used - b; i++)
		*bottom++ = *top++;

	for (; i < x->used; i++)
		*bottom++ = 0;
	x->used -= b;

clean:
	return ret;
}//bn_rshd

/*
���ƺ����� 1 ������λ
�ܺ���⣬���ǳ��� 2 ���߳��� 2���㷨��ɺ��� x ��ֵ��ֵ�� y��
*/

//���� 1 λ
int bn_lshift_1(bignum *y, const bignum *x)
{
/*
�����㷨Ĭ�Ͻ� y �ľ������ӵ� x->used + 1 ����λ��֮����Ҫ�� 1��
��Ϊ�п��ܸպð����λ�Ƶ���һ����λ��ȥ��olduse ��¼��ǰ y ����λ��
Ȼ�� y ����λ���ӵ� x ����λ��������ջ��н�λ��y->used ��Ҫ��һ��
shift ��������ÿ����λҪ�����ƶ��ı���λ����
������ 32 λϵͳ�£�shift = 31��64 λϵͳ�� shift = 63��
���� r �洢��һ����λ����ߵı���λ������ rr �洢��ǰ��λ����ߵı���λ��
��ѭ�����У��Ƚ���ǰ��λ���� shift λ���������ߵı���λ��
Ȼ���ٽ������λ���� 1 λ�������ұ���λ������߱���λ�� OR ������
������ǰ��λ�е�ÿһ����λ��������ƶ��� 1 λ������
�ұ���λ������߱���λҲ�ƶ�����ǰ��λ������λ�á�ѭ��������
��� r ��Ϊ 0������ԭ�� bignum �������λ������߱���λΪ 1��
�������б��Ƶ����µ���λ�У����� used ��һ���µ���λֵΪ 1��
��� y �ĸ�λ����Ϊ 0���� x �ķ��Ÿ� y ������в�����
*/
	int ret;
	size_t i, olduse, shift;
	bn_digit r, rr, *px, *py;

	BN_CHECK(bn_grow(y, x->used + 1));

	olduse = y->used;
	y->used = x->used;

	px = x->dp;
	py = y->dp;

	r = 0;
	shift = (size_t)(biL - 1);

	for (i = 0; i < x->used; i++)
	{
		rr = *px >> shift;		//��ȡ�����bit
		*py++ = (*px++ << 1) | r;
		r = rr;
	}

	if (r != 0)
	{
		*py = 1;
		y->used++;
	}

	py = y->dp + y->used;

	for (i = y->used; i < olduse; i++)
		*py++ = 0;

	y->sign = x->sign;

clean:
	return ret;
}//bn_lshift_1

//���� 1 λ
int bn_rshift_1(bignum *y, const bignum *x)
{
/*
���� 1 λ���Ȳ������ӣ�������ȻҪ���� bn_grow �������Ӿ��ȣ�
��Ϊһ��ʼ y �ľ��ȿ��ܲ��������� 1 λ�Ĳ��������� 1 λ�Ĳ����Ƚ����ƣ�ֻ�Ƿ����෴��
�ڵ�һ��ѭ�����У��Ȼ�ȡ��ǰ��λ�����ұ���λ��ŵ����� rr �У�Ȼ��ǰ��λ���� 1 λ��
���Ž������λ�����ұ���λ���� shift λ���뵱ǰ��λ�� OR ��������� rr ��ֵ��ŵ����� r �У�
������ǰλ�����б��ض������ƶ��� 1 λ�����������λ�����ұ߱���Ҳ�ƶ�����ǰ��λ������ߡ�
���ѭ���󽫸�λ����Ϊ 0��Ȼ�����÷���λ�����ѹ������λ��ɲ�����
*/
	int ret;
	size_t i, olduse, shift;
	bn_digit r, rr, *px, *py;

	BN_CHECK(bn_grow(y, x->used));	//y���ȿ��ܲ���

	olduse = y->used;
	y->used = x->used;

	px = x->dp + y->used - 1;
	py = y->dp + y->used - 1;

	r = 0;
	shift = (size_t)(biL - 1);

	for (i = y->used; i > 0; i--)
	{
		rr = *px & 1;
		*py-- = (*px-- >> 1) | (r << shift);
		r = rr;
	}

	py = y->dp + y->used;

	for (i = y->used; i < olduse; i++)
		*py++ = 0;

	y->sign = x->sign;

	bn_clamp(y);

clean:
	return ret;
}//bn_rshift_1

/*
���ƺ����� n ������λ
���˵ǰ�����λ���������������ԣ���ô�����������������ǽ���һ�㻯���ѡ�
���ƻ����� n λ�൱�ڳ��Ի���� 2^n��
*/

//���� n λ
int bn_lshift(bignum *y, const bignum *x, size_t count)
{
/*
�����㷨��鲢���� y �ľ��ȣ�����������Ƶ�λ�� count ���ڻ����һ����λ�ı�������
���� bn_lshd ������ x ���� count / biL ����λ��
Ȼ�����Ƿ���ʣ��ı���λ��d = count & (biL - 1)����� d ��Ϊ 0����������ʣ��λ��
�������� 1 λ��ԭ����ȡʣ��λ�����ƶ�������� n λ�Ĳ�����
*/
	int ret;
	size_t i, d, shift;
	bn_digit r, rr, *py;

	if (y != x)
		BN_CHECK(bn_copy(y, x));

	BN_CHECK(bn_grow(y, y->used + count / biL + 1));

	if (count >= biL)
		BN_CHECK(bn_lshd(y, count / biL));

	d = count & (biL - 1);

	if (d != 0)
	{
		py = y->dp;
		shift = (size_t)(biL - d);

		r = 0;

		for (i = 0; i < y->used; i++)
		{
			rr = (*py >> shift);
			*py = (*py << d) | r;
			py++;
			r = rr;
		}

		if (r != 0)
			y->dp[y->used++] = r;
	}

clean:
	return ret;
}//bn_lshift

// ���� n λ
int bn_rshift(bignum *y, const bignum *x, size_t count)
{
/*
������ n λһ��������������λ�����ƣ�Ȼ���ٰ������� 1 λ��ԭ�������ƶ�ʣ��ı���λ��
Ҫע����ǣ�����֮����Ҫѹ������λ������ used ��ֵ��
*/
	int ret = 0;
	size_t i, d, shift;
	bn_digit r, rr, *py;

	if (y != x)
		BN_CHECK(bn_copy(y, x));

	if (count >= biL) BN_CHECK(bn_rshd(y, count / biL));

	d = count & (biL - 1);

	if (d != 0)
	{
		py = y->dp + y->used - 1;
		shift = (size_t)(biL - d);

		r = 0;
		for (i = y->used; i > 0; i--)
		{
			rr = *py;
			*py = (*py >> d) | (r << shift);
			py--;
			r = rr;
		}
	}
	bn_clamp(y);

clean:
	return ret;
}//bn_rshift


/*
����ֵ�ӷ�
���Ĳ����ǣ��ӵ�λ����λ����ͬ����λ������ӣ���λ���ݵ���һλ�����в����ۼӺͽ�λ������ɺ�
������ĸ�λ���㣬���ҽ�����λ sign ����Ϊ 1 ������ֵ��ӣ������Ϊ�Ǹ���������
*/

/*
�㷨��һ�����ҳ���λ�ϴ��������������ָ�� tmp ָ������
�ڶ���������Ŀ�������ľ��ȣ��趨��������ָ��ֱ�ָ��������������������� dp ������ڴ�ķ���Ч�ʡ�
��������ͨ��һ��ѭ����λ����С�������ۼӵ��ϴ�������ϣ���������洢��Ŀ����������Ӧ��λ�У�
Ȼ��ʹ��һ�������ѭ���� ADDC_STOP �궨�崫���ۼӲ����Ľ�λ����󽫸�λ���㣬
����λ���ó� 1 ��ѹ���������λ��ɼ���
*/
int bn_add_abs(bignum *z, const bignum *x, const bignum *y)
{
	int ret;
	const bignum *tmp;
	bn_digit *px, *py, *pz;
	size_t i, max, min, olduse;

	ADDC_INIT

		if (x->used > y->used)
		{
			max = x->used;
			min = y->used;
			tmp = x;
		}
		else
		{
			max = y->used;
			min = x->used;
			tmp = y;
		}

	olduse = z->used;
	z->used = max + 1;
	BN_CHECK(bn_grow(z, z->used));

	px = x->dp;
	py = y->dp;
	pz = z->dp;

	for (i = 0; i < min; i++)
	{
		ADDC_CORE
		px++;
		py++;
		pz++;
	}

	if (min != max)
	{
		for (; i < max; i++)
		{
			ADDC_FINAL
			pz++;
		}
	}

	ADDC_STOP

	for (i = z->used; i < olduse; i++)
		*pz++ = 0;

	z->sign = 1;
	bn_clamp(z);

clean:
	return ret;
}//bn_add_abs


/*
����ֵ������������Ȼ�Ǳ����㷨���ӵ�λ��ʼ�������������λ��λ��ֱ�����е���λ��������ϡ�
����涨���㷨���� z = x - y������ x �ľ���ֵ���ڻ���� y�������㷨���ظ�������
����ֵ�����У�������������򲢲���Ҫ����Ϊǰ���Ѿ��涨 |x| >= |y|��
����ֱ�Ӱ� x->used �� max�� y->used �� min��t1 �� t2 ����ʱ������c �ǽ�λ��

�ڽ��м���֮ǰ���ȼ�� x �� y �ľ���ֵ��С���������������Լ�������������ظ�������

��� x �� y �ľ���ֵ��С���û���⣬��ô����Ϳ����������У�
���Ȱѽ�λ��ֵ��Ϊ 0��Ȼ���趨ָ�����������ڴ����Ч�ʡ�

��һ��ѭ������λ������ֱ�� x �� y ��ÿһ����λ��ֵ����ʱ���� t1 �� t2��
���� t1 - t2 - c ��ֵ��Ȼ���ŵ� z �Ķ�Ӧ��λ���У���� c = 0����ʾ��λû�����λ��λ��
�����Ϻ��жϱ�������Ƿ���Ҫ���λ��λ�����ԭ�� x �е�ĳһ��λ��ֵС�� y �ж�Ӧ��λ��ֵ��
��ȽϵĽ��Ϊ 1��c = 1��ע�����еļ��㶼�� mod 2^n��

�ڶ���ѭ������λ�͸�ֵ����� max > min������ x ����λҪ�� y �࣬���Ի��������λ���㡣
��� c = 0���򲻻�����λ�ˣ�ֱ�Ӱ� x ��ʣ����λ��ֵ�� z �Ķ�Ӧ��λ���ɡ�
��� c = 1���������Ե�λ�Ľ�λ�������һ����λ������ж���һλ�Ƿ���Ҫ��λ��
���� c ��ֵֻ������ 0 �� 1�����������λ����ǰ������λ��ֵ���� 0�����Ժ����λ������Ҫ������λ��
�ʽ� c ��ֵ��Ϊ 0�����򱣳���λֵ 1�������λ����󣬽� x ʣ�����λ�� z ����ɼ������㡣

������ѭ������λ���㡣�������������Ϻ󣬸�λ���в�Ϊ 0 ����λ��������գ������������

����ѭ�������󣬰ѷ���Ϊ��Ϊ 1����Ϊ����ֵ���������ս����Ȼ�Ǹ��Ǹ����������ѹ������λ��ɼ��㡣
*/
int bn_sub_abs(bignum *z, const bignum *x, const bignum *y)
{
	int ret;
	bn_digit *px, *py, *pz;
	size_t i, min, max, olduse, t1, t2, c;

	max = x->used;
	min = y->used;

	if (bn_cmp_abs(x, y) < 0)
		return BN_NEGATIVE_VALUE_ERROR;

	olduse = z->used;
	z->used = max;
	BN_CHECK(bn_grow(z, z->used));

	c = 0;
	px = x->dp;
	py = y->dp;
	pz = z->dp;

	for (i = 0; i < min; i++)
	{
		t1 = *px++;
		t2 = *py++;
		*pz++ = t1 - t2 - c;
		if (t1 != t2) c = (t1 < t2) ? 1 : 0;
	}

	for (; i < max; i++)
	{
		t1 = *px++;
		*pz++ = t1 - c;
		if (c != 0 && t1 != 0) c = 0;
	}

	for (i = max; i < olduse; i++)
		*pz++ = 0;

	z->sign = 1;
	bn_clamp(z);

clean:
	return ret;
}//bn_sub_abs

/*
�з������ļӷ��ֳ����������ͬ�ź���š�
1.  ���������ͬ�ţ���ִ�о���ֵ�ӷ������������Ϊ�Ǹ���������Ϊ�Ǹ�����
������������Ǹ���������ҲΪ������
2.  �����������ţ���Ҫִ�о���ֵ�������þ���ֵ�ϴ����ȥ������ֵ��С������
���ս�� z �ķ����� x �� y �ľ���ֵ��С��������� x �ľ���ֵ���ڻ���� y��
�� z �ķ����� x ��ͬ��ע��������ܳ��� -0 ������������� z �ķ����� x �෴��
*/
int bn_add_bn(bignum *z, const bignum *x, const bignum *y)
{
	int ret;
	int sign;

	sign = x->sign;

	if (x->sign == y->sign)
	{
		BN_CHECK(bn_add_abs(z, x, y));
		z->sign = sign;
	}
	else
	{
		if (bn_cmp_abs(x, y) >= 0)
		{
			BN_CHECK(bn_sub_abs(z, x, y));
			z->sign = sign;
		}
		else
		{
			BN_CHECK(bn_sub_abs(z, y, x));
			z->sign = -sign;
		}
	}

	if (BN_IS_ZERO(z))
		z->sign = 1;

clean:
	return ret;
}//bn_add_bn


/*
�з���������
���з������ļӷ����ƣ��з���������Ҳ�ֳ����������

1. ��������ţ�ִ�о���ֵ�ӷ������ z �ķ����� x ��������� x Ϊ�Ǹ������� z Ϊ������
��� x Ϊ�������� z Ϊ������

2. ������ͬ�ţ�ִ�о���ֵ�������þ���ֵ�ϴ����ȥ������ֵ��С������
��� z �ķ����� x �� y �ľ���ֵ��С��������� x �ľ���ֵ���ڻ���� y �ľ���ֵ��
�� z �� x ͬ�ţ����ܳ��� -0�������� z �� x ��š�
*/
int bn_sub_bn(bignum *z, const bignum *x, const bignum *y)
{
	int ret;
	int sign;

	sign = x->sign;

	if (x->sign != y->sign)
	{
		BN_CHECK(bn_add_abs(z, x, y));
		z->sign = sign;
	}
	else
	{
		if (bn_cmp_abs(x, y) >= 0)
		{
			BN_CHECK(bn_sub_abs(z, x, y));
			z->sign = sign;
		}
		else
		{
			BN_CHECK(bn_sub_abs(z, y, x));
			z->sign = -sign;
		}
	}

	if (BN_IS_ZERO(z))
		z->sign = 1;

clean:
	return ret;
}//bn_sub_bn

/*
����λ�ӷ��ͼ���
����λ�㷨����Ҫ�Ǽ���һ����������һ�����������ļ��㡣
�������㷨�ڴ���С��ģ���ݵļӼ�������á�
���ڵ���λ�ӷ��ͼ�����Ĭ��������һ����������һ���з��ŵĵ������������Ϊһ����������
�ڴ����ϣ����������±�д�����㷨�������Ƚ�����������ֵ��һ����ʱ�� bignum ������
Ȼ����������������з������㷨���м��㡣
*/
int bn_add_int(bignum *z, const bignum *x, const bn_sint y)
{
	int ret;
	bignum t[1];
	bn_digit p[1];

	p[0] = (y >= 0) ? y : -y;
	t->used = (y != 0) ? 1 : 0;
	t->sign = (y >= 0) ? 1 : -1;
	t->dp = p;
	t->alloc = 1;

	BN_CHECK(bn_add_bn(z, x, t));

clean:
	return ret;
}//bn_add_int

//����λ����
int bn_sub_int(bignum *z, const bignum *x, const bn_sint y)
{
	int ret;
	bignum t[1];
	bn_digit p[1];

	p[0] = (y >= 0) ? y : -y;
	t->used = (y != 0) ? 1 : 0;
	t->sign = (y >= 0) ? 1 : -1;
	t->dp = p;
	t->alloc = 1;

	BN_CHECK(bn_sub_bn(z, x, t));

clean:
	return ret;
}//bn_sub_int

/*
Ϊ�˷�����ֲ�ͳ�ַ��Ӳ�ͬƽ̨�µ����ܣ���ʱ�������ֲ�ͬ��ʵ�ַ�ʽ��
1����˫���ȱ������е������	IN_X86
2��ֻ�е����ȱ����������	IN_X64
3������ʹ���������������

���� c = a * b��c0��c1��c2 Ϊ�����ȱ�����
1. ���� c ������Ҫ�ľ��ȣ������� c = 0��c->used = a->used + b->used��

2. �� c2 = c1 = c0 = 0��

3. ���� i �� 0 �� c->used - 1 ѭ����ִ�����²�����
	3.1  ty = BN_MIN(i, b->used - 1)
	3.2  tx = i - ty
	3.3  k = BN_MIN(a->used - tx, ty + 1)
	3.4  �����ȱ�������һ����λ��(c2 || c1 || c0)  =  (0 || c2 || c1)
	3.5  ���� j �� 0 �� k - 1 ֮��ִ��ѭ�������㣺
		(c2 || c1 || c0) = (c2 || c1 || c0) + a(tx + j) * b(ty - j)
	3.6  c(i) = c0

4. ѹ������λ������ c��
*/
static void bn_mul_comba(bignum *z, const bignum *x, const bignum *y)
{
	bn_digit c0, c1, c2;
	bn_digit *px, *py, *pz;
	size_t nc, i, j, k, tx, ty;

	pz = z->dp;
	nc = z->used;

	c0 = c1 = c2 = 0;

	for (i = 0; i < nc; i++)
	{
		ty = BN_MIN(i, y->used - 1);
		tx = i - ty;
		k = BN_MIN(x->used - tx, ty + 1);

		px = x->dp + tx;
		py = y->dp + ty;

		c0 = c1;
		c1 = c2;
		c2 = 0U;

		//Comba 32
		for (j = k; j >= 32; j -= 32)
		{
			COMBA_INIT
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC

				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC

				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC

				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
			COMBA_STOP
		}
		//Comba 16
		for (; j >= 16; j -= 16)
		{
			COMBA_INIT
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC

				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
			COMBA_STOP
		}
		//Comba 8
		for (; j >= 8; j -= 8)
		{
			COMBA_INIT
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
			COMBA_STOP
		}
		//Comba 4
		for (; j >= 4; j -= 4)
		{
			COMBA_INIT
				COMBA_MULADDC    COMBA_MULADDC
				COMBA_MULADDC    COMBA_MULADDC
			COMBA_STOP
		}
		//Comba 1
		for (; j > 0; j--)
		{
			COMBA_INIT
				COMBA_MULADDC
			COMBA_STOP
		}

		*pz++ = c0;
	} //for i

	bn_clamp(z);
}//bn_mul_comba

/*
�㷨һ��ʼ������� x �� y ��ֳ����룬ʹ������ѭ���㶨��
Ϊ�����Ч�ʣ�������ָ���ǰ�߼����� register �ؼ�������ʾ��ִ�й�����
�������⼸��ָ������ŵ� CPU �ļĴ����У��Դ����ӿ�����ķ����ٶȡ�
��Ҫע����ǣ�ÿ������������ӷ����������˷�����λ����ִ�й����ж��п��ܳ���
���Ա������ BN_CHECK ����д����飬һ���������ó������� clean ����ִ���ڴ��������
��Ȼ Karatsuba �˷�ִ��ʱ����ĵ����ȳ˷��� Comba �����٣�
����Ҳ����һ�� O(n) ����Ŀ�������һ�������飬���ڼ����м����Լ��ϲ����Ľ����
���ʹ�� Karatsuba �˷���Ӧ������Ƚ�С������ʱ����ļ���ʱ�����ࡣ
�����ʵ�ʲ����У��ݹ���㵽һ����С�󣬾�Ӧ�ø��� Comba ���������ˡ�
�� bn_mul_bn �����У��ָ���С�� 80��bn_digit �ֳ�Ϊ 32 bit ʱ���� 
64��bn_digit �ֳ�Ϊ 64 bit ʱ�����������������Ĺ�ģ��һ��С�ڷָ��ʱ��
��Ӧ�ø��� Comba ��������˷���ֻ�е��������Ĺ�ģ���ڻ���ڷָ���ʹ�� Karatsuba �����ݹ���㡣
*/
static int bn_mul_karatsuba(bignum *z, const bignum *x, const bignum *y)
{
//�㷨��2.4

	int ret;
	size_t i, B;
	register bn_digit *pa, *pb, *px, *py;
	bignum x0[1], x1[1], y0[1], y1[1], t1[1], x0y0[1], x1y1[1];

	B = BN_MIN(x->used, y->used);
	B >>= 1;

	BN_CHECK(bn_init_size(x0, B));
	BN_CHECK(bn_init_size(x1, x->used - B));
	BN_CHECK(bn_init_size(y0, B));
	BN_CHECK(bn_init_size(y1, y->used - B));
	BN_CHECK(bn_init_size(t1, B << 1));
	BN_CHECK(bn_init_size(x0y0, B << 1));
	BN_CHECK(bn_init_size(x1y1, B << 1));

	x0->used = y0->used = B;
	x1->used = x->used - B;
	y1->used = y->used - B;

	px = x->dp;
	py = y->dp;
	pa = x0->dp;
	pb = y0->dp;

	for (i = 0; i < B; i++)
	{
		*pa++ = *px++;
		*pb++ = *py++;
	}

	pa = x1->dp;
	pb = y1->dp;

	for (i = B; i < x->used; i++)
		*pa++ = *px++;

	for (i = B; i < y->used; i++)
		*pb++ = *py++;

	bn_clamp(x0);
	bn_clamp(y0);

	BN_CHECK(bn_mul_bn(x0y0, x0, y0));
	BN_CHECK(bn_mul_bn(x1y1, x1, y1));

	BN_CHECK(bn_add_abs(t1, x0, x1));
	BN_CHECK(bn_add_abs(x0, y0, y1));
	BN_CHECK(bn_mul_bn(t1, x0, t1));

	BN_CHECK(bn_add_abs(x0, x0y0, x1y1));
	BN_CHECK(bn_sub_abs(t1, t1, x0));

	BN_CHECK(bn_lshd(t1, B));
	BN_CHECK(bn_lshd(x1y1, B << 1));

	BN_CHECK(bn_add_abs(t1, x0y0, t1));
	BN_CHECK(bn_add_abs(z, t1, x1y1));

clean:
	bn_free(x0);
	bn_free(x1);
	bn_free(y0);
	bn_free(y1);
	bn_free(t1);
	bn_free(x0y0);
	bn_free(x1y1);

	return ret;
}//bn_mul_karatsuba
















