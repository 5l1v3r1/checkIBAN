#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bnsl.h"

/*
 * Validate a number
 * Returns 1 if number is valid, 0 othervise
 */
int bnsl_valid(const char *a)
{
	int R = 0;
	unsigned int len, i;

	if (!a)
		return 0;

	len = strlen(a);

	if (len == 0)
		return 0;

	/* just the sign? */
	if ((a[0] == '-') && (len == 1))
		return 0;

	R = 1;
	for (i = 0; i < len; i++) {
		if (!((a[i] == '-') || (a[i] >= '0') || (a[i] <= '9'))) {
			R = 0;
			break;
		}
	}

	return R;
}

/*
 * Copy s to d, aligning right
 * Returns 0 on success;
 */
int bnsl_copy_r(char *d, unsigned int len_d, const char *s, unsigned int len_s)
{
	unsigned int i;

	if (len_d < len_s)
		return -1;

	for (i = 0; i < len_s; i++)
		d[len_d - len_s + i] = s[i];

	return 0;
}

/*
 * Cut leading 0s.
 */
char *bnsl_cut0(const char *a)
{
	char *R;
	int i, j;

	if (!bnsl_valid(a))
		return NULL;

	R = malloc(strlen(a) + 1 + 1);
	if (!R)
		return NULL;

	i = 0;
	j = 0;
	if (a[0] == '-') {
		R[j++] = '-';
		i++;
	}

	/* skip 0s */
	while (a[i] == '0')
		i++;

	/* test for "0000...0" */
	if (a[i] == '\0')
		i--;

	while (a[i] != '\0')
		R[j++] = a[i++];

	R[j] = '\0';

	return R;
}

/*
 * Change the sign
 */
char *bnsl_change_sign(const char *a1)
{
	char *R, *a2;
	int len, a_len, i, j;

	a2 = bnsl_cut0(a1);
	if (!a2)
		return NULL;

	a_len = strlen(a2);
	if (a2[0] == '-')
		len = a_len - 1;
	else
		len = a_len + 1;
	R = malloc(len + 1);
	if (!R) {
		free(a2);
		return NULL;
	}

	j = 0;
	i = 0;
	if (a2[0] == '-') {
		i++;
	} else {
		R[j++] = '-';
	}

	while (i < a_len) {
		R[j++] = a2[i++];
	}

	R[j] = '\0';

	free(a2);

	return R;
}

/*
 * Put a long to a string
 */
char *bnsl_n2bnsl(long n)
{
	char ret[64];

	snprintf(ret, sizeof(ret), "%ld", n);

	return strdup(ret);
}

/*
 * Compare two bnsl numbers.
 * Returns -1 for a<b, 0 for a=b, 1 for a>b
 * if abs is 1, the sign doesn't matter
 */
int bnsl_compare(const char *a, const char *b, int abs)
{
	int a_sign = 1, b_sign = 1;
	int a_len, b_len;

	if (a[0] == '-') {
		a_sign = 0;
		a++;
	}
	a_len = strlen(a);

	if (b[0] == '-') {
		b_sign = 0;
		b++;
	}
	b_len = strlen(b);

	if (abs == 0) {
		if (a_sign > b_sign)
			return 1;

		if (b_sign > a_sign)
			return -1;
	}

	if (a_len < b_len)
		return -1;

	if (a_len > b_len)
		return 1;

	return strcmp(a, b);
}

/*
 * Add/sub two bnsl numbers
 */
char *bnsl_add(const char *a1, const char *b1)
{
	int len, len_a, len_b, comp, final_sign;
	int A, B, C, CY;
	int ai, bi, ri, a_sign = 1, b_sign = 1, Sa = 1, Sb = 1;
	char *R, *R2, *a2, *b2;

	/*
	printf("bnsl_add(%s, %s)...\n",
		a1, b1);
	*/

	a2 = bnsl_cut0(a1);
	if (!a2)
		return NULL;

	b2 = bnsl_cut0(b1);
	if (!b2) {
		free(a2);
		return NULL;
	}

	len_a = strlen(a2);
	if (a2[0] == '-') {
		a_sign = 0;
		Sa = -1;
	}

	len_b = strlen(b2);
	if (b2[0] == '-') {
		b_sign = 0;
		Sb = -1;
	}

	len = (len_a > len_b ? len_a : len_b) + 1 + 1 + 1; /* + sign + carray + \0 */

	/*printf("\tAlloc %d bytes for R.\n", len);*/
	R = malloc(len);
	if (!R)
		return NULL;
	R[len - 1] = '\0';

	comp = bnsl_compare(a2, b2, 1);
	if (comp >= 0) { /* a >= b */
		final_sign = a_sign;
		/*
		* a + b, a - b - OK
		* -a + b => -(a-b)
		* -a - b => -(a+b)
		*/
		if (Sa == -1) {
			Sa = -Sa;
			Sb = -Sb;
		}
	} else { /* a < b */
		final_sign = b_sign;
		/*
		* a + b, -a + b - OK
		* a - b => -(-a+b)
		* -a - b => -(a+b)
		*/
		if (Sb == -1) {
			Sa = -Sa;
			Sb = -Sb;
		}
	}
	/*
	printf("\tSigns: a=%d, b=%d, final_sign=%d, comp=%d, Sa=%d, Sb=%d\n",
		a_sign, b_sign, final_sign, comp, Sa, Sb);
	*/

	CY = 0;
	ri = len - 2;
	ai = len_a - 1;
	bi = len_b - 1;
	while (1) {
		/*
		printf("\tai=%d bi=%d ri=%d...\n",
			ai, bi, ri);
		*/
		A = 0;
		if (ai >= 1 - a_sign) {
			A = (a2[ai] - '0') * Sa;
			ai--;
		}

		B = 0;
		if (bi >= 1 - b_sign) {
			B = (b2[bi] - '0') * Sb;
			bi--;
		}


		C = A + B + CY;
		/*
		printf("\t\tC = A(%d) + B(%d) + CY(%d) = %d\n",
			A, B, CY, C);
		*/
		CY = 0;
		if (C > 9) {
			C -= 10;
			CY = 1;
		} else if (C < 0) {
			C = 10 + C;
			CY = -1;
		}
		/*
		printf("\t\tNew CY=%d, C=%d\n", CY, C);

		printf("\t\tR[ri=%d] = %c\n", ri, C + '0');
		*/
		R[ri--] = C + '0';

		/*
		printf("\t\tai = %d, bi = %d, CY = %d\n", ai, bi, CY);
		*/
		if ((ai == 1 - a_sign - 1) && (bi == 1 - b_sign - 1) && (CY == 0))
			break;
	}

	free(b2);
	free(a2);

	while (ri >= 0)
		R[ri--] = '0';

	if (final_sign == 0) {
		/*
		printf("\tAdd - sign on pos 0.\n");
		*/
		R[0] = '-';
	}

	R2 = bnsl_cut0(R);
	free(R);

	/*
	printf("\tRET = [%s]\n", R2);
	*/

	return R2;
}

/*
 * Add/sub a number to a bnsl number
 */
char *bnsl_addi(const char *a1, long n)
{
	char s_n[64], *R, *a2;

	a2 = bnsl_cut0(a1);
	if (!a2)
		return NULL;

	snprintf(s_n, sizeof(s_n), "%ld", n);

	R = bnsl_add(a2, s_n);

	free(a2);

	return R;
}

/*
 * Test for x
 * Returns 1 if is 0
 */
int bnsl_equali(const char *a, char x)
{
	char *a2;
	int i, ret = 0;

	a2 = bnsl_cut0(a);
	if (!a2)
		return -1;

	i = 0;
	if (a2[i] == '-')
		i++;

	if ((strlen(a2 + i) == 1) && (a2[i] == x))
		ret = 1;

	free(a2);

	return ret;
}

/*
 * Multiply "a" with 10
 * @power - the power of 10 - ??? - we need to make is string!
 */
char *bnsl_mul10(const char *a, unsigned long power)
{
	char *R, *a2;
	unsigned int a2_len, i;

	a2 = bnsl_cut0(a);
	if (!a2)
		return NULL;
	a2_len = strlen(a2);

	R = malloc(a2_len + power + 1);
	if (!R) {
		free(a2);
		return NULL;
	}

	i = 0;
	while (i < a2_len) {
		R[i] = a2[i];
		i++;
	}
	while (power > 0) {
		R[i++] = '0';
		power--;
	}
	R[i] = '\0';

	free(a2);

	return R;
}

/*
 * Multiply a bnsl number with a single digit (or ten). It can be signed.
 * Returns NULL if something is wrong.
 */
char *bnsl_muli(const char *a1, int n)
{
	char *R, *R2, *a2, *tmp;
	unsigned int a_len, len, CY, M;
	int A, i, j, a_sign = 1;
	int final_sign = 1;

	/*
	printf("bnsl_muli(%s, %d)\n", a1, n);
	*/

	/* common cases */
	if (n == 0)
		return strdup("0");
	else if (n == 1)
		return strdup(a1);
	else if (n == -1)
		return bnsl_change_sign(a1);
	else if (n == 10)
		return bnsl_mul10(a1, 1);
	else if (n == -10) {
		tmp = bnsl_change_sign(a1);
		R = bnsl_mul10(tmp, 1);
		free(tmp);
		return R;
	}

	if ((n > 9) || (n < -9)) {
		return NULL;
	}

	a2 = bnsl_cut0(a1);
	if (!a2)
		return NULL;
	a_len = strlen(a2);

	if (n < 0) {
		final_sign = 0;
		n = -n;
	}

	if (a2[0] == '-') {
		final_sign = 1 - final_sign;
		a_sign = 0;
	}

	len = a_len + 1 + 1; /*  + 1 because of mul, the other for \0 */

	/*
	printf("\tAllocating %d bytes for R...\n", len);
	*/
	R = malloc(len);
	if (!R) {
		free(a2);
		return NULL;
	}
	memset(R, '0', len - 1);
	R[len - 1] = '\0';

	CY = 0;
	j = len - 2;
	/*
	printf("\tParse a from %u to %u...\n", a_len - 1, (1 - a_sign));
	*/
	for (i = a_len - 1; i >= (1 - a_sign); i--) {
		A = a2[i] - '0';
		M = A * n + CY;
		CY = M / 10;
		M = M % 10;
		R[j--] = M + '0';
		/*
		printf("\tCY=%u a2[%d]=%c A=%u M=%u R[%d]=%c\n",
			CY, i, a2[i], A, M, j + 1, R[j + 1]);
		*/
	}

	/* final add */
	if (CY > 0) {
		/*
		printf("\tAdd last CY (%u) to R[%d]...\n",
			CY, j);
		*/
		R[j--] = CY + '0';
	}

	/* sign */
	if (final_sign == 0) {
		/*
		printf("\tAdd '-' sign on position %d...\n", j);
		*/
		R[j--] = '-';
	}

	R2 = bnsl_cut0(R);
	free(R);
	R = R2;

	/*
	printf("\tR=[%s]\n", R);
	*/

	free(a2);

	return R;
}

/*
 * Multiply two numbers
 * Returns NULL if something is wrong
 */
char *bnsl_mul(const char *a1, const char *b1)
{
	char *R, *TOTAL, *TOTAL2, *a2, *a3, *b2, *b3, *factor, *factor2, *factor2_dec;
	char *SUM, *SUM2;
	unsigned int len, len_a, len_b;
	int B;
	int i, a_sign = 1, b_sign = 1, sign, S = 1, Sa = 1, Sb = 1;

	/*
	printf("bnsl_mul(%s, %s)...\n",
		a1, b1);
	*/

	a2 = bnsl_cut0(a1);
	if (!a2)
		return NULL;

	b2 = bnsl_cut0(b1);
	if (!b2) {
		free(a2);
		return NULL;
	}

	len_a = strlen(a2);
	a3 = a2;
	if (a2[0] == '-') {
		a_sign = 0;
		Sa = -1;
		a3++;
	}

	len_b = strlen(b2);
	b3 = b2;
	if (b2[0] == '-') {
		b_sign = 0;
		Sb = -1;
		b3++;
	}

	sign = Sa * Sb;
	S = 1;
	if (sign == 0)
		S = -1;

	len = len_a * len_b - S + 1;

	R = malloc(len);
	if (!R) {
		free(b2);
		free(a2);
		return NULL;
	}
	memset(R, '0', len - 1);
	R[len -1] = '\0';

	/*
	printf("\tSigns: a=%d, b=%d, sign=%d S=%d Sa=%d Sb=%d\n",
		a_sign, b_sign, sign, S, Sa, Sb);
	*/

	TOTAL = bnsl_n2bnsl(0);
	factor = bnsl_n2bnsl(1);
	for (i = len_b - 1; i >= 1 - b_sign; i--) {
		/*
		printf("\tMultiply [%s] with %c...\n", a3, b2[i]);
		*/

		B = b2[i] - '0';

		SUM = bnsl_muli(a3, B);
		if (!SUM) {
			/* TODO: leak!!!! */
			return NULL;
		}

		/*
		printf("\tMultimply SUM by 10 for %s times...\n", factor);
		*/
		factor2 = strdup(factor);
		while (!bnsl_equali(factor2, '1')) {
			SUM2 = bnsl_mul10(SUM, 1);
			free(SUM);
			SUM = SUM2;
			if (!SUM) {
				/* LEAK!!! */
				return NULL;
			}

			factor2_dec = bnsl_addi(factor2, -1);
			free(factor2);
			factor2 = factor2_dec;
			if (!factor2) {
				/* LEAK!!! */
				return NULL;
			}
		}

		/*
		printf("\tAdd SUM=%s to TOTAL=%s...\n", SUM, TOTAL);
		*/
		TOTAL2 = bnsl_add(TOTAL, SUM);
		free(TOTAL);
		free(SUM);
		TOTAL = TOTAL2;
		if (!TOTAL) {
			/* LEAK!!! */
			return NULL;
		}
		/*
		printf("\tTOTAL=%s.\n", TOTAL);
		*/

		factor2 = bnsl_addi(factor, 1);
		free(factor);
		factor = factor2;
		if (!factor) {
			/* LEAK!!! */
			return NULL;
		}
		/*
		printf("\tIncrementing factor to %s...\n", factor);
		*/
	}

	free(b2);
	free(a2);

	R = bnsl_cut0(TOTAL);
	free(TOTAL);

	return R;
}

/*
 * Returns a^power.
 */
char *bnsl_power(const char *a, const char *power)
{
	char *a2, *D, *D2;
	int a_len;
	char *power2, *power2_after_dec;

	a2 = bnsl_cut0(a);
	if (!a2)
		return NULL;
	a_len = strlen(a2);

	if (bnsl_equali(a2, '0')) {
		free(a2);
		return strdup("1");
	}

	power2 = bnsl_cut0(power);
	if (!power2) {
		free(a2);
		return NULL;
	}

	D = bnsl_cut0(a2);
	while (!bnsl_equali(power2, '1')) {
		D2 = bnsl_mul(D, a2);
		free(D);
		D = D2;
		if (!D) {
			free(power2);
			free(a2);
			return NULL;
		}

		power2_after_dec = bnsl_addi(power2, -1);
		free(power2);
		power2 = power2_after_dec;
		if (!power2) {
			free(D);
			free(a2);
			return NULL;
		}
	}

	free(power2);
	free(a2);

	return D;
}

/*
 * Divide a with b and returns the result. In @rest we will
 * put the rest.
 * TODO: must optimize it!
 */
char *bnsl_div(const char *a1, const char *b1, char **rest)
{
	char *a2, *b2;
	int comp;
	char *M, *tmp, *tmp2, *count, *count2, *sub;
	unsigned int len, len_a, len_b;
	int S = 1, Sa = 1, Sb = 1;
	int digits_a, digits_b, zeroes;

	/*
	printf("bnsl_div(%s, %s)...\n",
		a1, b1);
	*/

	a2 = bnsl_cut0(a1);
	if (!a2)
		return NULL;

	b2 = bnsl_cut0(b1);
	if (!b2) {
		free(a2);
		return NULL;
	}

	len_a = strlen(a2);
	digits_a = len_a;
	if (a2[0] == '-') {
		Sa = -1;
		digits_a--;
	}

	len_b = strlen(b2);
	digits_b = len_b;
	if (b2[0] == '-') {
		Sb = -1;
		digits_b--;
	}

	S = Sa * Sb;

	len = (len_a > len_b) ? len_a : len_b;
	len += (S == -1) ? 1 : 0;
	len += 1;

	/*
	printf("\tSigns: S=%d Sa=%d Sb=%d\n",
		S, Sa, Sb);
	*/

	comp = bnsl_compare(a2, b2, 1);
	if (comp == 0) { /* equal numbers */
		*rest = strdup("0");
		return strdup("1"); /* ??? Hm! Or -1! */
	} else if (comp == -1) { /* a2 < b2 */
		/* leak?! ??? */
		*rest = a2;
		return strdup("0");
	}

	/* a2 > b2 */

	/* change sign of a2 to '+' if is '-'*/
	if (Sa == -1) {
		tmp = bnsl_change_sign(a2);
		free(a2);
		a2 = tmp;
		if (a2 == NULL) {
			/* leak!!! */
			return NULL;
		}
	}

	/* change sign of b2 to '-' if is '+' */
	if (Sb == 1) {
		tmp = bnsl_change_sign(b2);
		free(b2);
		b2 = tmp;
		if (b2 == NULL) {
			/* leak!!! */
			return NULL;
		}
	}

	/* add as many as zeroes are needed so substractor must be less than a */
	/* substract a = a - sub as long as a > sub */
	/* if b > a - done */
	/* else, again */

	zeroes = digits_a - digits_b - 1;
	M = a2;
	count = strdup("0");
	sub = bnsl_mul10(b2, zeroes);
	while (1) {
		/*
		printf("M = %s, sub = %s\n", M, sub);
		*/

		comp = bnsl_compare(M, sub, 1);
		if (comp < 0) { /* M < sub */
			if (zeroes < 0)
				break;

			zeroes--;
			if (zeroes >= 0) {
				free(sub);
				sub = bnsl_mul10(b2, zeroes);
			}

			continue;
		}

		/*
		printf("zeroes=%d, sub=%s\n", zeroes, sub);
		*/

		if (zeroes <= 0)
			tmp = bnsl_add(count, "1");
		else {
			tmp2 = bnsl_mul10("1", zeroes);
			tmp = bnsl_add(count, tmp2);
			free(tmp2);
		}
		free(count);
		count = tmp;
		if (count == NULL) {
			/* leak??? */
			return NULL;
		}

		tmp = bnsl_add(M, sub);
		free(M);
		M = tmp;
		if (M == NULL) {
			/* leak??? */
			return NULL;
		}
	}

	/*printf("After several subs M = [%s]\n", M);*/

	/* rest's sign is a sign */
	if (Sa == -1) {
		tmp = bnsl_change_sign(M);
		free(M);
		M = tmp;
		if (M == NULL) {
			/* leak!!! */
			return NULL;
		}
	}
	*rest = M;

	/* "cit"'s sign is the (a * b) sign */
	if (S == -1) {
		count2 = bnsl_change_sign(count);
		free(count);
		count = count2;
		if (count == NULL) {
			/* leak!!! */
			return NULL;
		}
	}
	return count;
}

/*
 * Computes the modulo operation
 */
char *bnsl_mod(const char *a, const char *b)
{
	char *R;
	char *rest;

	/*
	printf("bnsl_mod(%s, %s)...\n",
		a, b);
	*/

	R = bnsl_div(a, b, &rest);
	if (R == NULL)
		return NULL;

	free(R);

	return rest;
}
