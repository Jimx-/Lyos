#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

. $DIR/utils.sh

pushd "$DIR" > /dev/null

if [ ! -d "sources" ]; then
    mkdir sources
fi

pushd sources > /dev/null

echo "Donwloading packages..."
download "binutils" "https://mirrors.ustc.edu.cn/gnu/binutils" "binutils-2.31.tar.gz" || cmd_error
download "mpc"  "http://www.multiprecision.org/downloads" "mpc-0.8.1.tar.gz"
download "mpfr" "http://www.mpfr.org/mpfr-2.4.2" "mpfr-2.4.2.tar.gz"
download "gmp"  "ftp://gcc.gnu.org/pub/gcc/infrastructure" "gmp-4.3.2.tar.bz2"
download "gcc" "https://mirrors.ustc.edu.cn/gnu/gcc/gcc-9.2.0" "gcc-9.2.0.tar.gz" || cmd_error
download "gcc-native" "https://mirrors.ustc.edu.cn/gnu/gcc/gcc-4.7.3" "gcc-4.7.3.tar.bz2" || cmd_error
download "newlib" "ftp://sourceware.org/pub/newlib" "newlib-3.0.0.tar.gz" || cmd_error
download "coreutils" "https://mirrors.ustc.edu.cn/gnu/coreutils" "coreutils-8.13.tar.xz" || cmd_error
download "bash" "https://ftp.gnu.org/gnu/bash/" "bash-4.3.tar.gz"
download "ncurses" "https://ftp.gnu.org/pub/gnu/ncurses/" "ncurses-6.2.tar.gz"
download "vim" "ftp://ftp.vim.org/pub/vim/unix" "vim-7.4.tar.bz2"
download "readline" "https://mirrors.ustc.edu.cn/gnu/readline" "readline-8.0.tar.gz"
download "libevdev" "https://www.freedesktop.org/software/libevdev" "libevdev-1.9.0.tar.xz"
download "libdrm" "https://dri.freedesktop.org/libdrm/" "libdrm-2.4.89.tar.bz2"
download "libexpat" "https://github.com/libexpat/libexpat/releases/download/R_2_2_9/" "expat-2.2.9.tar.bz2"
download "libffi" "https://github.com/libffi/libffi/releases/download/v3.3/" "libffi-3.3.tar.gz"
download "wayland" "https://github.com/wayland-project/wayland/archive/" "1.18.0.tar.gz" && cp 1.18.0.tar.gz wayland-1.18.0.tar.gz

echo "Decompressing packages..."
unzip "binutils-2.31.tar.gz" "binutils-2.31"
unzip "gmp-4.3.2.tar.bz2" "gmp-4.3.2"
unzip "mpfr-2.4.2.tar.gz" "mpfr-2.4.2"
unzip "mpc-0.8.1.tar.gz" "mpc-0.8.1"
unzip "gcc-9.2.0.tar.gz" "gcc-9.2.0"
unzip "gcc-4.7.3.tar.bz2" "gcc-4.7.3"
unzip "newlib-3.0.0.tar.gz" "newlib-3.0.0"
unzip "coreutils-8.13.tar.xz" "coreutils-8.13"
unzip "bash-4.3.tar.gz" "bash-4.3"
unzip "ncurses-6.2.tar.gz" "ncurses-6.2"
unzip "vim-7.4.tar.bz2" "vim74"
unzip "readline-8.0.tar.gz" "readline-8.0"
unzip "libevdev-1.9.0.tar.xz" "libevdev-1.9.0"
unzip "libdrm-2.4.89.tar.bz2" "libdrm-2.4.89"
unzip "expat-2.2.9.tar.bz2" "expat-2.2.9"
unzip "libffi-3.3.tar.gz" "libffi-3.3"
unzip "wayland-1.18.0.tar.gz" "wayland-1.18.0"

echo "Patching..."
patc "binutils-2.31"
patc "gmp-4.3.2"
patc "mpfr-2.4.2"
patc "mpc-0.8.1"
patc "gcc-9.2.0"
patc "gcc-4.7.3"
patc "newlib-3.0.0"
patc "coreutils-8.13"
patc "bash-4.3"
patc "ncurses-6.2"
patc "readline-8.0"
patc "libevdev-1.9.0"
patc "libdrm-2.4.89"
patc "expat-2.2.9"
patc "libffi-3.3"

echo "Installing extra files..."
install_newlib "newlib-3.0.0"

popd > /dev/null

if [ ! -d "local" ]; then
    mkdir local
fi

if [ ! -d "binary" ]; then
    mkdir binary
fi

popd > /dev/null
