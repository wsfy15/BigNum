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
		x->dp[offset] |= ((bn_digit)1 << mod);		//置数

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

/*
在移位操作当中存在两种不同的操作方式：逻辑移位和算术移位。
在逻辑移位当中，移出的位丢失，移入的位取 0。
在算术移位当中，移出的位丢失，左移入的位取 0，右移入的位取符号位，即最高位代表的数据符号保持不变。
*/

/*
左移和右移 b 个数位
简单来说就是乘以或除以 2 ^ (biL *b)，移位操作是以整个数位为单元进行的。
*/

//左移 b 个数位
int bn_lshd(bignum *x, size_t b)
{
/*
左移 b 个数位后，数位增加 b 位，所以 used 值要增加 b。
使用 bn_grow 函数增加 bignum 的精度。
然后用 top 指针指向 bignum 左移后的最高位，bottom 指针指向 bignum 当前的最高位，
然后循环 used - b 次把每一个数位往左移动 b 个数位。
操作结束后，让 top 指针指向最低位，循环 b 次把最低的 b 个数位置 0，完成移位操作
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

//右移 b 个数位
int bn_rshd(bignum *x, size_t b)
{
/*
和左移不同的是，右移精度不会增加，但如果 used 的值小于等于 b，
则 bignum 的最高位都会从右边被移出去，结果为 0。
如果 used > b，让 bottom指向 bignum 的最低数位，top 指针指向第 b + 1 位，
然后循环 used - b 次将每个数位往右挪动 b 个数位，最后将左边剩余的 b 位取 0，完成右移操作。
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
左移和右移 1 个比特位
很好理解，就是乘以 2 或者除以 2。算法完成后会把 x 的值赋值给 y。
*/

//左移 1 位
int bn_lshift_1(bignum *y, const bignum *x)
{
/*
首先算法默认将 y 的精度增加到 x->used + 1 个数位，之所以要加 1，
因为有可能刚好把最高位移到下一个数位中去。olduse 记录当前 y 的数位，
然后将 y 的数位增加到 x 的数位，如果最终还有进位，y->used 还要加一。
shift 变量表明每个数位要往右移动的比特位数，
例如在 32 位系统下，shift = 31，64 位系统下 shift = 63。
变量 r 存储上一个数位最左边的比特位，变量 rr 存储当前数位最左边的比特位。
在循环当中，先将当前数位右移 shift 位来获得最左边的比特位，
然后再将这个数位左移 1 位并且与右边数位的最左边比特位做 OR 操作，
这样当前数位中的每一比特位就往左边移动了 1 位，并且
右边数位的最左边比特位也移动到当前数位的最右位置。循环结束后，
如果 r 不为 0，表明原来 bignum 最左边数位的最左边比特位为 1，
在左移中被移到了新的数位中，所以 used 加一，新的数位值为 1。
最后将 y 的高位设置为 0，把 x 的符号给 y 完成所有操作。
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
		rr = *px >> shift;		//获取最左边bit
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

//右移 1 位
int bn_rshift_1(bignum *y, const bignum *x)
{
/*
右移 1 位精度不会增加，不过仍然要调用 bn_grow 函数增加精度，
因为一开始 y 的精度可能不够。右移 1 位的操作跟左移 1 位的操作比较类似，只是方向相反。
在第一个循环当中，先获取当前数位的最右比特位存放到变量 rr 中，然后当前数位右移 1 位，
接着将左边数位的最右比特位左移 shift 位后与当前数位做 OR 操作，最后将 rr 的值存放到变量 r 中，
这样当前位的所有比特都往右移动了 1 位，并且左边数位的最右边比特也移动到当前数位的最左边。
完成循环后将高位设置为 0，然后设置符号位，最后压缩多余位完成操作。
*/
	int ret;
	size_t i, olduse, shift;
	bn_digit r, rr, *px, *py;

	BN_CHECK(bn_grow(y, x->used));	//y精度可能不够

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
左移和右移 n 个比特位
如果说前面的移位操作都带有特殊性，那么下面这两个操作就是将其一般化而已。
左移或右移 n 位相当于乘以或除以 2^n。
*/

//左移 n 位
int bn_lshift(bignum *y, const bignum *x, size_t count)
{
/*
首先算法检查并增加 y 的精度，接着如果左移的位数 count 大于或等于一个数位的比特数，
调用 bn_lshd 函数将 x 左移 count / biL 个数位，
然后检查是否有剩余的比特位：d = count & (biL - 1)，如果 d 不为 0，表明还有剩余位，
按照左移 1 位的原理提取剩余位向左移动完成左移 n 位的操作。
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

// 右移 n 位
int bn_rshift(bignum *y, const bignum *x, size_t count)
{
/*
和左移 n 位一样，先做整个数位的右移，然后再按照右移 1 位的原理往右移动剩余的比特位。
要注意的是，右移之后，需要压缩多余位来更新 used 的值。
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
绝对值加法
核心操作是：从低位到高位将相同的数位进行相加，进位传递到下一位。所有操作累加和进位操作完成后，
将结果的高位清零，并且将符号位 sign 设置为 1 （绝对值相加，结果必为非负整数）。
*/

/*
算法第一步是找出数位较大的正数，并且让指针 tmp 指向它。
第二步是增加目标整数的精度，设定三个别名指针分别指向输入整数和输出整数的 dp 来提高内存的访问效率。
第三步是通过一个循环将位数较小的整数累加到较大的整数上，并将结果存储到目标整数的相应数位中，
然后使用一个额外的循环和 ADDC_STOP 宏定义传递累加产生的进位，最后将高位清零，
符号位设置成 1 和压缩多余的数位完成计算
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
绝对值减法的做法仍然是笔算算法，从低位开始减，不够的向高位借位，直到所有的数位都处理完毕。
这里规定，算法计算 z = x - y，并且 x 的绝对值大于或等于 y，否则算法返回负数错误。
绝对值减法中，对输入进行排序并不重要，因为前面已经规定 |x| >= |y|，
所以直接把 x->used 给 max， y->used 给 min；t1 和 t2 是临时变量，c 是借位。

在进行计算之前，先检查 x 和 y 的绝对值大小，如果不满足上面约定的条件，返回负数错误。

如果 x 和 y 的绝对值大小检查没问题，那么计算就可以正常进行，
首先把借位的值设为 0，然后设定指针别名来提高内存访问效率。

第一个循环：对位相减。分别把 x 和 y 的每一个数位赋值给临时变量 t1 和 t2，
计算 t1 - t2 - c 的值，然后存放到 z 的对应数位当中，如果 c = 0，表示低位没有向高位借位。
相减完毕后，判断本次相减是否需要向高位借位，如果原来 x 中的某一数位的值小于 y 中对应数位的值，
则比较的结果为 1，c = 1。注意所有的计算都是 mod 2^n。

第二个循环：退位和赋值。如果 max > min，表明 x 的数位要比 y 多，所以还需进行退位计算。
如果 c = 0，则不会有退位了，直接把 x 的剩余数位赋值给 z 的对应数位即可。
如果 c = 1，则还有来自低位的借位，在完成一次退位计算后，判断下一位是否需要退位，
由于 c 的值只可能是 0 或 1，如果本次退位计算前，该数位的值大于 0，则以后的数位都不需要进行退位，
故将 c 的值置为 0，否则保持退位值 1。完成退位计算后，将 x 剩余的数位给 z ，完成减法计算。

第三个循环：高位清零。如果减法计算完毕后，高位还有不为 0 的数位，必须清空，否则结果会出错。

所有循环结束后，把符号为设为 1，因为绝对值减法的最终结果仍然是个非负整数；最后压缩多余位完成计算。
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
有符号数的加法分成两种情况：同号和异号。
1.  如果两个数同号，则执行绝对值加法，如果两个数为非负数，则结果为非负数；
如果两个数都是负数，则结果也为负数。
2.  如果两个数异号，则要执行绝对值减法，用绝对值较大的数去减绝对值较小的数。
最终结果 z 的符号由 x 和 y 的绝对值大小决定：如果 x 的绝对值大于或等于 y，
则 z 的符号与 x 相同（注意这里可能出现 -0 的情况），否则 z 的符号与 x 相反。
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
有符号数减法
和有符号数的加法类似，有符号数减法也分成两种情况：

1. 两个数异号：执行绝对值加法。结果 z 的符号由 x 决定，如果 x 为非负数，则 z 为正数；
如果 x 为负数，则 z 为负数。

2. 两个数同号：执行绝对值减法，用绝对值较大的数去减绝对值较小的数。
结果 z 的符号由 x 和 y 的绝对值大小决定，如果 x 的绝对值大于或等于 y 的绝对值，
则 z 和 x 同号（可能出现 -0），否则 z 与 x 异号。
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
单数位加法和减法
单数位算法，主要是计算一个大整数和一个单精度数的计算。
这两个算法在处理小规模数据的加减会很有用。
对于单数位加法和减法，默认输入是一个大整数和一个有符号的单精度数，结果为一个大整数。
在处理上，并不是重新编写两个算法，而是先将单精度数赋值给一个临时的 bignum 变量，
然后利用上面的两个有符号数算法进行计算。
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

//单数位减法
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
为了方便移植和充分发挥不同平台下的性能，暂时用了三种不同的实现方式：
1、单双精度变量都有的情况。	IN_X86
2、只有单精度变量的情况。	IN_X64
3、可以使用内联汇编的情况。

计算 c = a * b，c0，c1，c2 为单精度变量。
1. 增加 c 到所需要的精度，并且令 c = 0，c->used = a->used + b->used。

2. 令 c2 = c1 = c0 = 0。

3. 对于 i 从 0 到 c->used - 1 循环，执行如下操作：
	3.1  ty = BN_MIN(i, b->used - 1)
	3.2  tx = i - ty
	3.3  k = BN_MIN(a->used - tx, ty + 1)
	3.4  三精度变量右移一个数位：(c2 || c1 || c0)  =  (0 || c2 || c1)
	3.5  对于 j 从 0 到 k - 1 之间执行循环，计算：
		(c2 || c1 || c0) = (c2 || c1 || c0) + a(tx + j) * b(ty - j)
	3.6  c(i) = c0

4. 压缩多余位，返回 c。
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
算法一开始将输入的 x 和 y 拆分成两半，使用三个循环搞定。
为了提高效率，这里在指针的前边加上了 register 关键字来暗示在执行过程中
尽量把这几个指针变量放到 CPU 的寄存器中，以此来加快变量的访问速度。
需要注意的是，每个计算操作（加法，减法，乘法和移位）在执行过程中都有可能出错，
所以必须加上 BN_CHECK 宏进行错误检查，一旦函数调用出错，调到 clean 后面执行内存清理操作
虽然 Karatsuba 乘法执行时所需的单精度乘法比 Comba 方法少，
但是也多了一项 O(n) 级别的开销来解一个方程组，用于计算中间项以及合并最后的结果，
这就使得 Karatsuba 乘法在应对输入比较小的数字时所需的计算时间会更多。
因此在实际操作中，递归计算到一定大小后，就应该改用 Comba 方法计算了。
在 bn_mul_bn 函数中，分割点大小是 80（bn_digit 字长为 32 bit 时）或 
64（bn_digit 字长为 64 bit 时），当输入两个数的规模有一个小于分割点时，
就应该改用 Comba 方法计算乘法，只有当两个数的规模大于或等于分割点才使用 Karatsuba 方法递归计算。
*/
static int bn_mul_karatsuba(bignum *z, const bignum *x, const bignum *y)
{
//算法书2.4

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
















