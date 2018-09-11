#ifndef __RSA_H
#define __RSA_H


//我们使用 索洛维-斯特拉森 算法测试首数字的精确度.20 在大多数情况下已经够大了 
#define ACCURACY 20

#define FACTOR_DIGITS 100
#define EXPONENT_MAX RAND_MAX
#define BUF_SIZE 1024

//给大数字结构体初始化容量. 可以灵活的改变但是应该比这个大以避免频繁的使用reallocs
#define BIGNUM_CAPACITY 20

//Radix and halfradix. These should be changed if the limb/word type changes
#define RADIX 4294967296UL
#define HALFRADIX 2147483648UL

#define MAX(a,b) ((a) > (b) ? (a) : (b))
/**
 * 基本的 limb 类型. 注意某些计算依赖无符号数溢出 wrap-around of this type.
 * 因此, 这里只能用无符号类型的数,并且上面的 RADIX, HALFRADIX 变量应该做必要的改变
 * Unsigned integer should probably be the most efficient word type, and this
 * is used by GMP for example.
 */
typedef unsigned int word;
//#define word unsigned int
/**
 * Structure for representing multiple precision integers. This is a base "word" LSB
 * representation. In this case the base, word, is 2^32. Length is the number of words
 * in the current representation. Length should not allow for trailing zeros (Things like
 * 000124). The capacity is the number of words allocated for the limb data.
 */
typedef struct _bignum {
	word length;
	word capacity;
	word* data;
} bignum;
/**
 * Some forward delcarations as this was requested to be a single file.
 * See specific functions for explanations.
 */
bignum* bignum_init();
void bignum_iadd(bignum* source, bignum* add);
void bignum_add(bignum* result, bignum* b1, bignum* b2);
void bignum_isubtract(bignum* source, bignum* add);
void bignum_subtract(bignum* result, bignum* b1, bignum* b2);
void bignum_imultiply(bignum* source, bignum* add);
void bignum_multiply(bignum* result, bignum* b1, bignum* b2);
void bignum_idivide(bignum* source, bignum* div);
void bignum_idivider(bignum* source, bignum* div, bignum* remainder);
void bignum_remainder(bignum* source, bignum *div, bignum* remainder);
void bignum_imodulate(bignum* source, bignum* modulus);
void bignum_divide(bignum* quotient, bignum* remainder, bignum* b1, bignum* b2);
int bignum_less(bignum* b1, bignum* b2);
void encode(bignum* m, bignum* e, bignum* n, bignum* result);
void decode(bignum* c, bignum* d, bignum* n, bignum* result);
bignum *encodeMessage(int len, int bytes, char *message, bignum *exponent, bignum *modulus);
int *decodeMessage(int len, int bytes, bignum *cryptogram, bignum *exponent, bignum *modulus);
void bignum_fromstring(bignum* b, char* string);
void bignum_fromint(bignum* b, unsigned int num);

int create_key(const char const *public_key,const char const *private_key);
int save_key(const char *filename,bignum *e,bignum *n);
int read_key(const char *filename,bignum *e,bignum *n);
/**
 * Save some frequently used bigintegers (0 - 10) so they do not need to be repeatedly
 * created. Used as, NUMS[5] = bignum("5"), etc..
 */
extern word DATA0[1]; 
extern word DATA1[1]; 
extern word DATA2[1];
extern word DATA3[1]; 
extern word DATA4[1]; 
extern word DATA5[1];
extern word DATA6[1]; 
extern word DATA7[1]; 
extern word DATA8[1];
extern word DATA9[1]; 
extern word DATA10[1];
extern bignum NUMS[11];


#endif
