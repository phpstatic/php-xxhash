#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_xxhash.h"

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#define XXH_INLINE_ALL
#include "xxhash.h"

#ifdef COMPILE_DL_XXHASH
	ZEND_GET_MODULE(xxhash);
	#ifdef ZTS
		ZEND_TSRMLS_CACHE_DEFINE();
	#endif
#endif

PHP_MINFO_FUNCTION(xxhash)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "xxhash support", "enabled");
	php_info_print_table_row(2, "extension version", PHP_XXHASH_VERSION);
	php_info_print_table_row(2, "xxhash release", "0.8.0");
	php_info_print_table_end();
}

static char * base62_digits="0123456789"
                            "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;

static char *b62_encode(uint64_t num)
{
    /* 64 bits is up to 11 digits in base 62 */
    static char enc[15] ;
    int         i, len ;
    uint64_t    rem ;
    char        c ;

    /* Zero out output buffer */
    memset(enc, 0, sizeof(enc));
    if (num==0) {
        enc[0]='0';
        return enc;
    }
    i=0 ;
    while (num>0) {
        rem = num % 62 ;
        enc[i] = base62_digits[rem];
        i++ ;
        num /= 62 ;
    }
    /* Reverse string */
    len = (int)strlen(enc);
    for (i=0 ; i<len/2 ; i++) {
        c = enc[i] ;
        enc[i] = enc[len-i-1];
        enc[len-i-1] = c ;
    }
    return enc ;
}

uint64_t b62_decode(char * s, size_t len)
{
    uint64_t    num ;
    uint64_t    p ;
    char     *  pos ;
    int         i ;

    if (!s)
        return 0 ;

    p=1;
    num=0;
    for (i=len-1 ; i>=0 ; i--) {
        pos = strchr(base62_digits, (int)s[i]);
        if (!pos) {
            return 0 ;
        }
        num += (uint64_t)(pos-base62_digits) * p ;
        p *= 62 ;
    }
    return num ;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_xxhash, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xxhash_long, 0, 0, 1)
		ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()


PHP_FUNCTION(xxhash32)
{
    zend_string *data;
    zend_string *strg;
	unsigned int sum;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

	// compute the checksum
	sum = XXH32(ZSTR_VAL(data), ZSTR_LEN(data), 0);

	//convert to a hex string
	strg = strpprintf(0, "%08x", sum);

	// return the checksum
	RETURN_STR(strg);
}

PHP_FUNCTION(xxhash64)
{
    zend_string *data;
    zend_string *strg;
	unsigned long long sum;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

	// compute the checksum
	sum = XXH64(ZSTR_VAL(data), ZSTR_LEN(data), 0);

	//convert to a hex string
	strg = strpprintf(0, "%08x%08x", (unsigned int)(sum >> 32), (unsigned int)sum);

	// return the checksum
	RETURN_STR(strg);
}

PHP_FUNCTION(xxhash64_base62)
{
    zend_string *data;
    zend_string *strg;
	uint64_t sum;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

	// compute the checksum
	sum = XXH64(ZSTR_VAL(data), ZSTR_LEN(data), 0);

	//convert to a hex string
	strg = strpprintf(0, "%s", b62_encode(sum));

	// return the checksum
	RETURN_STR(strg);
}

PHP_FUNCTION(base62_encode)
{
    zend_long sum;
    zend_string *strg;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(sum)
     ZEND_PARSE_PARAMETERS_END();

	//convert to a hex string
	strg = strpprintf(0, "%s", b62_encode((uint64_t)sum));

	// return the checksum
	RETURN_STR(strg);
}


PHP_FUNCTION(base62_decode)
{
    zend_long sum;
    zend_string *data;
    zend_string *strg;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

	//convert to a hex string
	sum = (zend_long) b62_decode(ZSTR_VAL(data), ZSTR_LEN(data));

	// return the checksum
	RETURN_LONG(sum);
}

const zend_function_entry xxhash_functions[] = {
	ZEND_FE(xxhash32, arginfo_xxhash)
	ZEND_FE(xxhash64, arginfo_xxhash)
	ZEND_FE(xxhash64_base62, arginfo_xxhash)
	ZEND_FE(base62_encode, arginfo_xxhash_long)
	ZEND_FE(base62_decode, arginfo_xxhash)
	PHP_FE_END
};

zend_module_entry xxhash_module_entry = {
	STANDARD_MODULE_HEADER,
	"xxhash",
	xxhash_functions,
	NULL,
	NULL,
	NULL,
	NULL,
	PHP_MINFO(xxhash),
	PHP_XXHASH_VERSION,
	STANDARD_MODULE_PROPERTIES
};
