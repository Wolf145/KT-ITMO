#include <inttypes.h>
#include <stdio.h>
#define Md 2147483647
#define max_uint(a, b) (a >= b ? a : b)
uint32_t multiply_by_Md(uint32_t a, uint16_t b)
{
	uint64_t a_ = a, b_ = b;
	a_ = (a_ * (b_ % Md)) % Md;
	a = a_;
	return a;
}
uint32_t factor(uint16_t n)
{
	if (n == 0)
		return 1;
	uint32_t ans = 1;
	for (uint16_t i = 1; i != n; ++i)
		ans = multiply_by_Md(ans, i);
	ans = multiply_by_Md(ans, n);
	return ans;
}
uint8_t find_len(uint32_t a)
{
	uint8_t cnt = 0;
	do
	{
		++cnt;
		a /= 10;
	} while (a);
	return cnt;
}
uint8_t find_max_len(uint16_t l, uint16_t r)
{
	if (l <= r)
	{
		uint32_t fact = factor(l);
		uint8_t len = find_len(fact);
		if (l == r)
			return len;
		for (uint16_t i = l + 1; i != r; ++i)
		{
			fact = multiply_by_Md(fact, i);
			len = max_uint(len, find_len(fact));
		}
		fact = multiply_by_Md(fact, r);
		len = max_uint(len, find_len(fact));
		return len;
	}
	else
	{
		return max_uint(find_max_len(l, UINT16_MAX), find_max_len(0, r));
	}
}
void print_symb_n_times(uint8_t len, char symb)
{
	for (uint8_t i = 0; i < len; ++i)
		putchar(symb);
}
void print_line(uint8_t len1, uint8_t len2)
{
	printf("+");
	print_symb_n_times(len1 + 2, '-');
	printf("+");
	print_symb_n_times(len2 + 2, '-');
	puts("+");
}
void print_head(uint8_t len1, uint8_t len2, int8_t align)
{
	print_line(len1, len2);
	printf("| ");
	if (align == -1)
	{
		printf("n");
		print_symb_n_times(len1 - 1, ' ');
		printf(" | n!");
		print_symb_n_times(len2 - 2, ' ');
	}
	else if (align == 1)
	{
		print_symb_n_times(len1 - 1, ' ');
		printf("n | ");
		print_symb_n_times(len2 - 2, ' ');
		printf("n!");
	}
	else
	{
		print_symb_n_times(((len1 - 1) >> 1) + ((len1 - 1) & 1), ' ');
		printf("n");
		print_symb_n_times(((len1 - 1) >> 1), ' ');
		printf(" | ");
		print_symb_n_times(((len2 - 2) >> 1) + ((len2 - 2) & 1), ' ');
		printf("n!");
		print_symb_n_times(((len2 - 2) >> 1), ' ');
	}
	puts(" |");
	print_line(len1, len2);
}
void process_ith_row(uint16_t i, uint32_t f_i, uint8_t len1, uint8_t len2, int8_t align)
{
	uint8_t l_i = find_len(i), l_f_i = find_len(f_i);
	printf("| ");
	if (align == -1)
	{
		printf("%" PRIu16, i);
		print_symb_n_times(len1 - l_i, ' ');
		printf(" | %" PRIu32, f_i);
		print_symb_n_times(len2 - l_f_i, ' ');
	}
	else if (align == 1)
	{
		print_symb_n_times(len1 - l_i, ' ');
		printf("%" PRIu16 " | ", i);
		print_symb_n_times(len2 - l_f_i, ' ');
		printf("%" PRIu32, f_i);
	}
	else
	{
		print_symb_n_times(((len1 - l_i) >> 1) + ((len1 - l_i) & 1), ' ');
		printf("%" PRIu16, i);
		print_symb_n_times(((len1 - l_i) >> 1), ' ');
		printf(" | ");
		print_symb_n_times(((len2 - l_f_i) >> 1) + ((len2 - l_f_i) & 1), ' ');
		printf("%" PRIu32, f_i);
		print_symb_n_times(((len2 - l_f_i) >> 1), ' ');
	}
	puts(" |");
}
void print_body(uint16_t begin, uint16_t end, uint8_t len1, uint8_t len2, int8_t align)
{
	if (begin <= end)
	{
		uint32_t f_i = factor(begin);
		process_ith_row(begin, f_i, len1, len2, align);
		if (begin == end)
			return;
		for (uint16_t i = begin + 1; i != end; ++i)
		{
			f_i = multiply_by_Md(f_i, i);
			process_ith_row(i, f_i, len1, len2, align);
		}
		process_ith_row(end, multiply_by_Md(f_i, end), len1, len2, align);
	}
	else
	{
		print_body(begin, UINT16_MAX, len1, len2, align);
		print_body(0, end, len1, len2, align);
	}
}
int main(void)
{
	int32_t n_start_, n_end_;
	uint16_t border = UINT16_MAX;
	int8_t align;
	if (scanf("%" SCNd32 "%" SCNd32 "%" SCNd8, &n_start_, &n_end_, &align) != 3)
	{
		fprintf(stderr,
				"ERROR: required 3 arguements: [n_start(integer; >=0)] "
				"[n_end(integer; >=0)] [align(-1, 0, 1)]");
		return 1;
	}
	if (n_start_ < 0 || n_end_ < 0)
	{
		fprintf(stderr, "ERROR: n_start - integer; >=0 && n_end  - integer; >=0");
		return 1;
	}
	uint16_t n_start = (n_start_ % (UINT16_MAX + 1)), n_end = n_end_ % (UINT16_MAX + 1);
	uint8_t len_1st_col = (n_start <= n_end) ? find_len(n_end) : find_len(border);
	uint8_t len_2nd_col = max_uint(find_max_len(n_start, n_end), 2);
	print_head(len_1st_col, len_2nd_col, align);
	print_body(n_start, n_end, len_1st_col, len_2nd_col, align);
	print_line(len_1st_col, len_2nd_col);
	return 0;
}
