#!/bin/sh

export PATH=$PWD/toolchain/local/bin:$PATH

ARCH=$1
RELEASE=$2
CC=$3
LOCAL_VERSION=$4
LOCAL_VERSION_AUTO=$5
SMP=$6

if [ -r $PWD/.version ]; then
    VERSION=`cat .version`
else
    VERSION=0
    echo 0 > .version
fi

if [ -n "$LOCAL_VERSION" ] ; then
    VERSION="$VERSION-$LOCAL_VERSION"
fi

if [ "$LOCAL_VERSION_AUTO" = "y" ]; then
    HEAD=`git rev-parse --verify --short HEAD 2>/dev/null`
    if [ -n $HEAD ]; then
        VERSION="$VERSION-g$HEAD"
    fi
fi

if [ "$SMP" = "y" ]; then
    VERSION="$VERSION SMP"
fi

TIMESTAMP=`date`
VERSION="$VERSION $TIMESTAMP"

( 
  echo \/\* Generated by gencompile.sh \*\/
  echo \#define ARCH $ARCH
  echo
  echo \#define UTS_MACHINE \"$ARCH\"
  echo \#define UTS_VERSION \"#$VERSION\"
  echo \#define UTS_RELEASE \"$RELEASE\"
  echo 
  echo \#define LYOS_COMPILE_BY \"`whoami`\"
  echo \#define LYOS_COMPILE_HOST \"`hostname`\"
  echo \#define LYOS_COMPILER \"`$CC -v 2>&1 | tail -n 1`\"
  echo \#define LYOS_LOCAL_VERSION \"$LOCAL_VERSION\"
) > ./include/lyos/compile.h
