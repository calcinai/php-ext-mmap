PHP_ARG_ENABLE(mmap, whether to enable mmap support,
[ --enable-mmap   Enable mmap support])
if test "$PHP_MMAP" = "yes"; then
  AC_DEFINE(HAVE_MMAP, 1, [Whether you have mmap])
  PHP_NEW_EXTENSION(mmap, mmap.c, $ext_shared)
fi