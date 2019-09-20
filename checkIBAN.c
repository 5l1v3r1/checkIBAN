/*
 * Description: IBAN checker
 * Author: Catalin(ux aka Dino) BOIE 2005
 * http://kernel.umbrella.ro/
 * License: LGPL
 */

#define _GNU_SOURCE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bnsl.h"


static int is_alpha(char c)
{
	if (((c >= '0') && (c <= '9'))
		|| ((c >= 'a') && (c <= 'z'))
		|| ((c >= 'A') && (c <= 'Z')))
		return 1;

	return 0;
}

static int is_upper(char c)
{
	if ((c >= 'A') && (c <= 'Z'))
		return 1;

	return 0;
}

/* A - 10, G=16, Q=26, Z=35 */
static char *iban_convert_a2i(char *s)
{
	unsigned int i, j, v;
	static char ret[128];

	j = 0;
	for (i = 0; i < strlen(s); i++) {
		if (is_upper(s[i])) {
			v = s[i] - 'A' + 10;
			ret[j++] = '0' + v / 10;
			ret[j++] = '0' + v % 10;
		} else {
			ret[j++] = s[i];
		}
	}
	ret[j] = '\0';

	return ret;
}

int main(int argc, char *argv[])
{
	char iban[128], iban2[128];
	char *iban3, *rem;
	unsigned int i, j, len;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s \"<IBAN code>\"\n",
			argv[0]);
		return 1;
	}

	snprintf(iban, 64, "%s", argv[1]);
	printf("INPUT: [%s]\n", iban);

	/* delete non alpha */
	i = 0; j = 0;
	while (iban[i] != '\0') {
		if (is_alpha(iban[i]))
			iban[j++] = iban[i];
		i++;
	}
	iban[j] = '\0';
	printf("After deleting non-alpha: [%s]\n", iban);
	len = strlen(iban);
	if (len == 0) {
		printf("Error: invalid IBAN (len 0)!\n");
		return 1;
	}

	/* move first 4 to the end */
	for (i = 0; i < len - 4; i++)
		iban2[i] = iban[i + 4];
	for (i = 0; i < 4; i++)
		iban2[i + len - 4] = iban[i];
	iban2[len] = '\0';
	printf("After moving 4 bytes to the end: [%s]\n", iban2);

	/* convert letters to numbers */
	/* A - 10, ..., G=16, ..., Q=26, ..., Z=35 */
	iban3 = iban_convert_a2i(iban2);
	printf("After converting letters to numbers [%s]\n",
		iban3);

	/* Apply mod 97-10 (ISO 7064) - rem must be 1 */
	rem = bnsl_mod(iban3, "97");
	if (!rem) {
		printf("Error in modulo!\n");
		return 1;
	}

	printf("After bnsl_mod [%s]\n",
		rem);

	if (strcmp(rem, "1") == 0) {
		printf("OK!\n");
	} else {
		printf("NOT OK!\n");
	}
	free(rem);


	return 0;
}

#ifdef __cplusplus
}
#endif

