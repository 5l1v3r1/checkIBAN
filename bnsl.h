#ifndef _BNSL_H_
#define _BNSL_H_

extern char	*bnsl_cut0(const char *a);
extern int	bnsl_valid(const char *a);
extern char	*bnsl_addi(const char *a, long n);
extern char	*bnsl_add(const char *a, const char *b);
extern char	*bnsl_mul(const char *a, const char *b);
extern char	*bnsl_muli(const char *a, int n);
extern char	*bnsl_n2bnsl(long n);
extern char	*bnsl_change_sign(const char *a);
extern char	*bnsl_mul10(const char *a, unsigned long power);
extern char	*bnsl_div(const char *a, const char *b, char **rest);
extern char	*bnsl_mod(const char *a, const char *b);

#endif
