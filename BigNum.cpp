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

/*
绝对值比较，有符号数比较，多精度数和单精度数比较。
约定：所有的比较操作，如果 x > y，返回 1；x == y，返回 0；x < y，返回 -1。
*/

/*
先比较位数，位数大的绝对值更大，如果位数相同，从高位到低位依次比较两个大整数中相同位的数位大小
*/
int bn_cmp_abs(const bignum *x, const bignum *y)
{
	size_t i;

	if (x->used > y->used) return  1;
	if (x->used < y->used) return -1;

	//x 和 y 的位数相同，依次比较相同位的大小。
	for (i = x->used - 1; i >= 0; i--)
	{
		if (x->dp[i] > y->dp[i]) return  1;
		if (x->dp[i] < y->dp[i]) return -1;
	}
	return 0;
}//bn_cmp_abs

/*
有符号数比较
1. x 和 y 两个数异号：非负数大于负数。
2. x 和 y 两个数同号：若 x 和 y 都是非负整数，比较 x 和 y 的绝对值大小；
若 x 和 y 都是负整数，比较 y 和 x 的绝对值大小。
*/

int bn_cmp_bn(const bignum *x, const bignum *y)
{
	if (x->sign == 1 && y->sign == -1) return  1;
	if (y->sign == 1 && x->sign == -1) return -1;

	if (x->sign == 1)			//都是正数
		return bn_cmp_abs(x, y);
	else						//都是负数
		return bn_cmp_abs(y, x);
}//bn_cmp_bn

/*
多精度数和单精度数比较
简单来说就是将一个大整数与一个有符号的单精度数（bn_sint类型）进行比较。
有了前面的大整数与大整数之间的有符号比较操作，直接把有符号的单精度数赋值给一个临时的大整数类型变量t，
然后调用bn_cmp_bn函数进行比较。
*/
int bn_cmp_int(const bignum *x, const bn_sint y)
{
	bignum t[1];
	bn_digit p[1];

	p[0] = (y >= 0) ? y : -y;
	t->sign = (y >= 0) ? 1 : -1;
	t->used = (y == 0) ? 0 : 1;   //注意 bignum 为 0 时，规定 used = 0.
	t->alloc = 1;
	t->dp = p;

	return bn_cmp_bn(x, t);
}//bn_cmp_int

//返回指定下标 pos 的比特位
boolean bn_get_bit(const bignum *x, const size_t pos)
{
	if (x->alloc*biL <= pos)
		return 0;
	boolean bit;

	//bit = 1 & (x->dp[pos / biL] >> (pos%biL));		//每个数位的比特从0 到 31 
	bit = (x->dp[pos / biL] >> (pos & (biL - 1))) & 1;	//直接做位运算更快
	//在 2 的补码运算中，计算 a mod (2^n) 等价于 a AND (2^n - 1)。

	return bit;
}//bn_get_bit

/*
给定下标值 pos 设置对应位置的比特位
先找到 pos 对应的比特位的位置。假设 pos = 35，则 offset = pos / biL = 1， mod = pos / biL = 3。
找到对应的位置后，要先把该位的值清空，具体的做法是：将 1 左移 mod = 3 位，然后取反，
这时操作结果的二进制就会是 1111 1111 1111 1111 1111 1111 1111 0111，
与大整数的第二个数位（对应数组下标为 1）进行与运算，便清空该位的值。
然后将函数传入的 boolean 类型的 value 左移 mod = 3 位后与大整数的第二个数位做或运算
即可把新的比特位设置好。
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
		//如果pos超过了分配的数位并且value为0，则无需操作，因为高位默认为0，否则需要增加精度。
		BN_CHECK(bn_grow(x, offset + 1));
	}

	x->dp[offset] &= ~((bn_digit)1 << mod);				//对指定位清零
	if(value != FALSE)
		x->dp[offset] |= ((bn_digit)1 << mod);		//对指定位置数

	x->used = x->alloc;  
	//如果bignum的精度增加，则需要从最左边的数位开始往右检查有效数位，保证used值的正确。
	bn_clamp(x);  //压缩多余位

clean:
	return ret;
}//bn_set_bit



/*
返回  有效比特  位   的数量
这个算法的意义在于可以计算出一个 bignum 的实际比特大小
有效比特位是指从左起第一个不为 0 的比特位开始到最右比特位为止的中间所有比特位。
*/
size_t bn_msb(const bignum *x)
{
	size_t i, j;

	i = x->used - 1;
	//最高数位不能全为0
	for (j = biL; j > 0; j--)
		if (((x->dp[i] >> (j - 1)) & 1) != 0)
			break;

	return biL * i + j;
}//bn_msb

/*
返回最低有效比特位前 0 的数量
最低有效比特位是指从右起第一个不为 0 的比特位
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
返回 bignum 的字节大小
这里要注意的是：这个操作不是返回 bignum 占用了多少内存，例如对于 bignum x，
即使一开始分配了 3 个单元的内存，其字节大小仍然是 8 byte。
计算 bignum 的字节大小，只需要将 x 的比特大小加上 7 除以 8 即可。
加上 7 的目的是避免 x 的比特位不是 8 的倍数时除完之后少了一个字节
*/
size_t bn_size(const bignum *x)
{
	return ((bn_msb(x) + 7) >> 3);
}//bn_size





