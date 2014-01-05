dnl $Id$
dnl config.m4 for extension router

PHP_ARG_ENABLE(router, whether to enable router support,
[  --enable-router           Enable router support])

if test "$PHP_ROUTER" != "no"; then
  PHP_NEW_EXTENSION(router, router.c, $ext_shared)
fi
