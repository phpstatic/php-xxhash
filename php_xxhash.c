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
	php_info_print_table_row(2, "xxhash release", XXH_versionNumber());
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

PHP_FUNCTION(xxhash32)
{
    char *arg = NULL;
    size_t arg_len, len;
    zend_string *strg;
	unsigned int sum;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE || arg_len < 1) {
        return;
    }

	// compute the checksum
	sum = XXH32(arg, arg_len, 0);

	//convert to a hex string
	strg = strpprintf(0, "%08x", sum);

	// return the checksum
	RETURN_STR(strg);
}

PHP_FUNCTION(xxhash64)
{
	char *arg = NULL;
    size_t arg_len, len;
    zend_string *strg;
	unsigned long long sum;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE || arg_len < 1) {
        return;
    }

	// compute the checksum
	sum = XXH64(arg, arg_len, 0);

	//convert to a hex string
	strg = strpprintf(0, "%08x%08x", (unsigned int)(sum >> 32), (unsigned int)sum);

	// return the checksum
	RETURN_STR(strg);
}

PHP_FUNCTION(xxhash64_base62)
{
	char *arg = NULL;
    size_t arg_len, len;
    zend_string *strg;
	unsigned long long sum;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE || arg_len < 1) {
        return;
    }

	// compute the checksum
	sum = XXH64(arg, arg_len, 0);

	//convert to a hex string
	strg = strpprintf(0, "%s", b62_encode(sum));

	// return the checksum
	RETURN_STR(strg);
}

const zend_function_entry xxhash_functions[] = {
	ZEND_FE(xxhash32, NULL)
	ZEND_FE(xxhash64, NULL)
	ZEND_FE(xxhash64_base62, NULL)
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