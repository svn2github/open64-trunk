ARGS=""
for i in $*
do
        case $1 in
                --srcdir=*)
                        srcdir=`expr "x$1" : 'x[^=]*=\(.*\)'`
                        ;;
                --prefix=*)
                        prefix=`expr "x$1" : 'x[^=]*=\(.*\)'`
                        ;;
                *)
                        ARGS="$ARGS $1"
        esac
        shift 1
done

export CC="$GCC_CONFIGURE_COMPILER $GCC_CONFIGURE_CFLAGS"
$srcdir/configure  --prefix=$prefix/open64-gcc-4.2.0 --with-gnu-as --with-gnu-ld --enable-languages=c,c++ --disable-bootstrap --disable-libmudflap --disable-libssp --disable-checking --enable-threads=posix --enable-tls --with-system-zlib --enable-__cxa_atexit --srcdir=$srcdir --host=$GCC_CONFIGURE_HOST --target=$GCC_CONFIGURE_TARG
