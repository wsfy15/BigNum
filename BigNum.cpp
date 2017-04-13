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


//bignum结构初始化，未分配内存。
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
 bignum结构初始化并分配内存。
 若初始化失败（内存分配错误），返回BN_MEMORY_ALLOCATE_FAIL
 若超出数位上限，返回BN_EXCEED_MAX_LIMB
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
精度增加
bn_grow函数首先检查需要的数位（nlimbs）是否大于alloc，如果不是，那无需增加精度，函数调用结束
如果nlimbs大于alloc，那么就要重新分配一段长度为nlimbs的内存空间，
初始化为0后把原来dp指向的内存中的内容复制到新的空间中，并且释放原先dp指向的内存。
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
复制操作
把bignum y复制给bignum x，注意这里的x->dp指向新的内存，而不是指向y->dp。
*/
int bn_copy(bignum *x, const bignum *y)
{
	int ret;
	size_t nlimbs;

	if (x == y)
		return 0;

	x->sign = y->sign;
	x->used = y->used;
	nlimbs = (y->used == 0) ? 1 : y->used;		//如果 y 为0，默认给 x 分配一位的空间
	BN_CHECK(bn_grow(x, nlimbs));
	
	memset(x->dp, 0, ciL*nlimbs);
	if (y->dp != NULL && y->used != 0)
		memcpy(x->dp, y->dp, ciL*y->used);

clean:
	return ret;
}//bn_copy

/*单精度赋值操作
把bignum设置成一个相对较小的单精度数，(0 -- 2^32 - 1) 
通过分配1个数位的内存，把单精度数赋值给数组的首个单元即可。
在这里我默认的单精度数是无符号的（即非负整数），如果想要赋值一个负数，可以直接把sign设为-1即可。
*/
int bn_set_word(bignum *x, const bn_digit word)
{
	int ret;
	BN_CHECK(bn_grow(x, 1));	//分配一个数位的内存

	memset(x->dp, 0, ciL*x->alloc);
	x->dp[0] = word;
	x->used = 1;
	x->sign = 1;		//如果是负数，在函数调用完后把sign设成-1即可

clean:
	return ret;
}//bn_set_word

/*
交换操作
这函数的作用很简单，就是交换两个大整数 x 和 y，具体的实现原理也很简单，交换结构体中的每个成员即可
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
压缩多余位
当一个函数的预期输出为 n 位时，最简单的做法是假定整个函数使用的位数都是 n 位，
而不用在计算过程中检查它是否如此。比如一个 a 位的大整数乘以 b 位的大整数，结果最多为 a + b 位。
但是结果完全有可能是一个最后没有进位的 a + b - 1 位数。
假设用于存放结果的某一整数先被扩展成 a + b - 1 位，然后出现进位后继续扩展 1 位，这将浪费很多时间，
因为堆上的操作比较慢。所以最好的办法是假定结果为 a + b 位，在函数结束后处理 used，
这样只需一次内存分配，效率提高，然而如果不对结果进行检查，高位可能会有多余的 0。
解决的方法很简单，编写一个简单的多余位压缩算法即可。
*/
void bn_clamp(bignum *x)
{
	while (x->used > 0 && x->dp[x->used - 1] == 0)
		x->used--;

	if (x->used == 0)
		x->sign = 1;
}//bn_clamp





