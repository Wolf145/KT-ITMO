#include "return_codes.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define VALID_SINGLE_CHAR_STRING(s) ((s)[0] != '\0' && (s)[1] == '\0')

const char* args_check(int argc, char* argv[])
{
	if (argc != 4 && argc != 6)
		return "incorrect amount of arguments\n";
	if (!VALID_SINGLE_CHAR_STRING(argv[1]) || argv[1][0] != 'f' && argv[1][0] != 'h')
		return "incorrect format\n";
	if (!VALID_SINGLE_CHAR_STRING(argv[2]) || argv[2][0] != '0' && argv[2][0] != '1' && argv[2][0] != '2' && argv[2][0] != '3')
		return "incorrect rounding\n";
	if (argc == 6 &&
		(!VALID_SINGLE_CHAR_STRING(argv[4]) || argv[4][0] != '*' && argv[4][0] != '/' && argv[4][0] != '+' && argv[4][0] != '-'))
		return "invalid operation\n";
	return NULL;
}
typedef struct Num	  /// OK
{
	int8_t sign, shift, rest1, rest2, shift_frac, shift_exp, shift_zeros;
	int16_t exp;
	uint64_t frac;
} Num;
bool is_infty(Num* num)	   /// OK
{
	return (num->exp == (1 << (num->shift_exp - 1))) && !(num->frac % (1 << num->shift_frac));
}
bool is_nan(Num* num)	 /// OK
{
	return (num->exp == (1 << (num->shift_exp - 1))) && (num->frac % (1 << num->shift_frac));
}
bool is_zero(Num* num)	  /// OK
{
	return (num->exp == 2 - (1 << (num->shift_exp - 1))) && !(num->frac % (1 << num->shift_frac));
}
void normalize(Num* num, int8_t flag)	 /// OK
{
	if (is_nan(num) || is_infty(num) || is_zero(num))
		return;

	while ((num->frac >> num->shift_frac) >= 2)
	{
		num->rest1 = num->frac & 1;
		if (num->rest1 && (num->frac >> (num->shift_frac + 1)) != 1)
			num->rest2 = 1;
		num->exp += flag;
		num->frac >>= 1;
	}
	while (!(num->frac >> num->shift_frac))
	{
		num->exp -= flag;
		num->frac <<= 1;
	}
}
void round_num(Num* a, char rounding)	 /// OK
{
	if (rounding == '1' && a->rest1)
	{
		if (a->rest2)
			++a->frac;
		else
			a->frac += (a->frac & 1);
	}
	else if ((a->rest1 | a->rest2) && (rounding == '2' && !(a->sign) || rounding == '3' && a->sign))
		++a->frac;
	normalize(a, 1);
}
void construct(Num* a, uint32_t num, char type)	   /// OK
{
	a->rest1 = 0, a->rest2 = 0;
	if (type == 'h')
		a->shift_frac = 10, a->shift_exp = 5, a->shift_zeros = 3, a->shift = 2;
	else
		a->shift_frac = 23, a->shift_exp = 8, a->shift_zeros = 6, a->shift = 1;
	a->sign = num >> (a->shift_frac + a->shift_exp);
	a->exp = ((num >> a->shift_frac) & ((1 << a->shift_exp) - 1));
	a->frac = num & ((1 << a->shift_frac) - 1);
	if (!a->exp)
		++a->exp;
	else
		a->frac |= (1 << a->shift_frac);
	a->exp -= (1 << (a->shift_exp - 1)) - 1;
	normalize(a, 1);
}
void extra_cases_print(Num* a, char rounding, char* s1, char* s2)
{
	if (a->sign)
	{
		putchar('-');
		if (rounding == '0' || rounding == '3')
			printf("%s", s1);
		else
			printf("%s", s2);
	}
	else
	{
		if (rounding == '0' || rounding == '3')
			printf("%s", s2);
		else
			printf("%s", s1);
	}
}
void print_num(Num* a, char rounding)	 /// OK
{
	char str[5] = "0x1.";
	if (a->exp > (1 << (a->shift_exp - 1)))
	{
		extra_cases_print(a, rounding, "inf\n", (a->shift_exp == 5 ? "0x1.ffcp+15\n" : "0x1.fffffep+127\n"));
		return;
	}
	else if (a->exp < 2 - (1 << (a->shift_exp - 1)) - a->shift_frac)
	{
		extra_cases_print(a, rounding, (a->shift_exp == 5 ? "0x1.000p-24\n" : "0x1.000000p-149\n"), (a->shift_exp == 5 ? "0x0.000p+0\n" : "0x0.000000p+0\n"));
		return;
	}
	if (is_nan(a))
	{
		printf("nan\n");
		return;
	}
	else if (is_infty(a))
	{
		(a->sign ? printf("-inf\n") : printf("inf\n"));
		return;
	}
	else if (is_zero(a))
	{
		str[2] = '0';
		a->exp = 0;
	}
	else
	{
		round_num(a, rounding);
		a->frac ^= 1 << (a->shift_frac);
		a->frac <<= a->shift;
	}
	if (a->sign)
		putchar('-');
	printf(
		"%s"
		"%0*x"
		"p%+" PRId32 "\n",
		str,
		a->shift_zeros,
		(unsigned int)a->frac,
		a->exp);
}
void mul(Num* a, Num* b, char rounding)	   /// OK
{
	Num c = {
		.shift_frac = a->shift_frac,
		.shift_exp = a->shift_exp,
		.shift_zeros = a->shift_zeros,
		.shift = a->shift,
		.rest1 = 0,
		.rest2 = 0,
		.sign = a->sign ^ b->sign,
		.exp = 2 - (1 << (a->shift_exp - 1)),
		.frac = 0
	};
	if (is_nan(a) || is_nan(b))
	{
		// make NaN
		c.exp = 1 << (c.shift_exp - 1);
		c.frac = 1;
	}
	else if (is_infty(a) || is_infty(b))
	{
		// make Inf
		c.exp = 1 << (c.shift_exp - 1);
		c.frac = 0;
		if (is_zero(a) || is_zero(b))
		{
			// make NaN
			c.exp = 1 << (c.shift_exp - 1);
			c.frac = 1;
		}
	}
	else if (!(is_zero(a) || is_zero(b)))
	{
		c.frac = a->frac * b->frac;
		int8_t overflow = (c.frac >> (2 * c.shift_frac + 1));
		c.exp = a->exp + b->exp + overflow;
	}
	normalize(&c, 0);
	print_num(&c, rounding);
}
void division(Num* a, Num* b, char rounding)
{
	Num c = {
		.shift_frac = a->shift_frac,
		.shift_exp = a->shift_exp,
		.shift_zeros = a->shift_zeros,
		.shift = a->shift,
		.rest1 = 0,
		.rest2 = 0,
		.sign = a->sign ^ b->sign,
		.exp = 2 - (1 << (a->shift_exp - 1)),
		.frac = 0
	};
	if (is_nan(a) || is_nan(b) || is_zero(a) && is_zero(b) || is_infty(a) && is_infty(b))
	{
		// make NaN
		c.exp = 1 << (c.shift_exp - 1);
		c.frac = 1;
	}
	else if (is_infty(a) && !is_zero(b) || !is_zero(a) && is_zero(b))
	{
		// make Inf
		c.exp = 1 << (c.shift_exp - 1);
		c.frac = 0;
	}
	else if (!(is_infty(b) && !is_zero(a)))
	{
		c.frac = (a->frac << (63 - (c.shift_frac + 1))) / b->frac;
		int8_t underflow = ((c.frac >> (63 - (c.shift_frac + 1))) ? 0 : 1);
		c.exp = a->exp - b->exp - underflow;
	}
	normalize(&c, 0);
	print_num(&c, rounding);
}
void add(Num*, Num*, char);
void sub(Num* a, Num* b, char rounding, int8_t invert)
{
	Num c = {
		.shift_frac = a->shift_frac,
		.shift_exp = a->shift_exp,
		.shift_zeros = a->shift_zeros,
		.shift = a->shift,
		.rest1 = 0,
		.rest2 = 0,
		.sign = a->sign ^ b->sign,
		.exp = 2 - (1 << (a->shift_exp - 1)),
		.frac = 0
	};
	if (is_nan(a) || is_nan(b))
	{
		// make NaN
		c.exp = 1 << (c.shift_exp - 1);
		c.frac = 1;
		print_num(&c, rounding);
		return;
	}
	if (is_infty(a) || is_infty(b))
	{
		if (is_infty(a) && is_infty(b) && !(a->sign ^ b->sign))
		{
			// make NaN
			c.exp = 1 << (c.shift_exp - 1);
			c.frac = 1;
		}
		else if (is_infty(b))
		{
			c = *b;
			c.sign = 1 ^ b->sign;
		}
		else
		{
			c = *a;
		}
		print_num(&c, rounding);
		return;
	}
	else if (is_zero(a) || is_zero(b))
	{
		if (is_zero(a) && is_zero(b))
		{
			print_num(&c, rounding);
			return;
		}
		if (is_zero(b))
		{
			c = *a;
		}
		else
		{
			c = *b;
			c.sign = 1 ^ b->sign;
		}
		print_num(&c, rounding);
		return;
	}
	if (a->sign ^ b->sign)
	{
		b->sign ^= 1;
		add(a, b, rounding);
	}
	else if (a->sign)
	{
		a->sign = 0;
		b->sign = 0;
		sub(b, a, rounding, invert);
		return;
	}
	if (a->exp < b->exp)
	{
		sub(b, a, rounding, 1);
		return;
	}
	int16_t diff_exp = a->exp - b->exp;
	uint64_t frac_a = a->frac, frac_b = b->frac;
	if (diff_exp < 62 - c.shift_frac)
	{
		frac_a <<= diff_exp;
		c.frac = frac_a <= frac_b ? frac_b - frac_a : frac_a - frac_b;
		c.sign = frac_a <= frac_b;
	}
	else
	{
		frac_b = a->frac;
		frac_a = b->frac >> diff_exp;
		if (b->frac && !frac_a)
		{
			c.frac = frac_a - 1;
			c.rest1 = 1;
			c.sign = 0;
		}
		else
		{
			for (int16_t i = 0; i < diff_exp; ++i)
			{
				c.rest1 = b->frac & 1;
				if (b->frac && i + 1 != diff_exp)
					c.rest2 = 1;
				b->frac >>= 1;
			}
			c.frac = frac_a <= frac_b ? frac_b - frac_a : frac_a - frac_b;
			c.sign = frac_a <= frac_b;
		}
	}
	if (c.rest1 || c.rest2)
		--c.frac;
	c.sign ^= invert;
	normalize(&c, 1);
	print_num(&c, rounding);
}
void add(Num* a, Num* b, char rounding)
{
	Num c = {
		.shift_frac = a->shift_frac,
		.shift_exp = a->shift_exp,
		.shift_zeros = a->shift_zeros,
		.shift = a->shift,
		.rest1 = 0,
		.rest2 = 0,
		.sign = 0,
		.exp = 2 - (1 << (a->shift_exp - 1)),
		.frac = 0
	};
	if (is_nan(a) || is_nan(b))
	{
		// make NaN
		c.exp = 1 << (c.shift_exp - 1);
		c.frac = 1;
		print_num(&c, rounding);
		return;
	}
	if (is_infty(a) || is_infty(b))
	{
		if (is_infty(a) && is_infty(b) && (a->sign ^ b->sign))
		{
			// make NaN
			c.frac = 1;
		}
		else
		{
			// make Inf
			is_infty(a) ? (c.sign = a->sign) : (c.sign = b->sign);
			c.frac = 0;
		}
		c.exp = 1 << (c.shift_exp - 1);
		print_num(&c, rounding);
		return;
	}
	else if (is_zero(a) || is_zero(b))
	{
		if (!(is_zero(a) && is_zero(b)))
			is_zero(a) ? (c = *b) : (c = *a);
		print_num(&c, rounding);
		return;
	}
	if (a->sign ^ b->sign)
	{
		if (a->sign)
		{
			a->sign ^= 1;
			sub(b, a, rounding, 0);
		}
		else
		{
			b->sign ^= 1;
			sub(a, b, rounding, 0);
		}
		return;
	}
	if (a->exp < b->exp)
	{
		add(b, a, rounding);
		return;
	}
	c.sign = a->sign;
	int16_t diff_exp = a->exp - b->exp;
	if (diff_exp < 62 - c.shift_frac)
	{
		c.exp = b->exp;
		c.frac = b->frac + (a->frac << diff_exp);
	}
	else
	{
		c.exp = a->exp;
		c.frac = a->frac + (b->frac >> diff_exp);
		for (int16_t i = 0; i < diff_exp; ++i)
		{
			c.rest1 = b->frac & 1;
			if (b->frac && i + 1 != diff_exp)
				c.rest2 = 1;
			b->frac >>= 1;
		}
	}
	normalize(&c, 1);
	print_num(&c, rounding);
}
int main(int argc, char* argv[])
{
	const char* msg = args_check(argc, argv);
	if (msg != NULL)
	{
		fprintf(stderr, "%s", msg);
		return ERROR_ARGUMENTS_INVALID;
	}
	if (argc == 4)
	{
		uint32_t num;
		if (!sscanf(argv[3], "%" SCNx32, &num))
		{
			fprintf(stderr, "can't read arguments\n");
			return ERROR_ARGUMENTS_INVALID;
		}
		Num a;
		construct(&a, num, argv[1][0]);
		print_num(&a, argv[2][0]);
	}
	else
	{
		uint32_t num1, num2;
		if (!sscanf(argv[3], "%" SCNx32, &num1) || !sscanf(argv[5], "%" SCNx32, &num2))
		{
			fprintf(stderr, "can't read arguments\n");
			return ERROR_ARGUMENTS_INVALID;
		}
		Num a, b;
		construct(&a, num1, argv[1][0]);
		construct(&b, num2, argv[1][0]);
		switch (argv[4][0])
		{
		case '*':
			mul(&a, &b, argv[2][0]);
			break;
		case '/':
			division(&a, &b, argv[2][0]);
			break;
		case '+':
			add(&a, &b, argv[2][0]);
			break;
		case '-':
			sub(&a, &b, argv[2][0], 0);
			break;
		}
	}
}
