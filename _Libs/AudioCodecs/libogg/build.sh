#!/bin/bash

# User parameters

PREFIX=$1
TARGET=libogg.a
X_CFLAGS="-O2 -DNDEBUG -I$PWD/include"
FILES=(framing.c bitwise.c)
INCLUDE_TO_COPY="-a include/ogg"

# System parameters

if [[ $PREFIX == "" ]];
then
    PREFIX=./bin
fi

if [[ "$OSTYPE" != "msys" ]]; then
    X_CFLAGS="$X_CFLAGS -fPIC"
fi

if [[ "$OSTYPE" == "darwin"* ]]; then
    #use Clang on OS X hosts
    CC="clang"
else
    #use on any other hosts GCC
    CC="gcc"
fi
LD="ar -cqs"


# ===========Ready? GO!!!===========

mkdir -p temp
TO_LINK=
	
errorofbuildOGG()
{
	printf "\n\n=========ERROR!!===========\n\n"
	exit 1
}

Build()
{
    echo $CC -c $X_CFLAGS src/$1 -o temp/$1.o

    $CC -c $X_CFLAGS src/$1 -o temp/$1.o

    if [ ! $? -eq 0 ]
    then
        errorofbuildOGG
    fi

    TO_LINK="$TO_LINK temp/$1.o"
}

printf "\nCompiling...\n\n"

for i in ${FILES[@]}; do
    Build ${i}
done

rm -f $PREFIX/lib/libmad.a
printf "\nLinking...\n\n"
echo $LD $TARGET $TO_LINK
$LD $TARGET $TO_LINK

if [ ! $? -eq 0 ]
then
    errorofbuildMAD
fi

echo "Installing into $PREFIX..."
mkdir -p $PREFIX/lib
mkdir -p $PREFIX/include

mv $TARGET $PREFIX/lib
cp $INCLUDE_TO_COPY $PREFIX/include

rm -Rf temp
printf "\nEVERYTHING HAS BEEN COMPLETED!\n\n"


