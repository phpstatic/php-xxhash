dnl config.m4 for extension xxhash

PHP_ARG_ENABLE([xxhash],
  [whether to enable xxhash support],
  [AS_HELP_STRING([--enable-xxhash],
    [Enable test support])],
  [no])

if test "$PHP_XXHASH" != "no"; then
  dnl In case of no dependencies
  AC_DEFINE(HAVE_XXHASH, 1, [ Have xxhash support ])

  PHP_NEW_EXTENSION(xxhash, php_xxhash.c, $ext_shared)
fi
