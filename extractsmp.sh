#!/bin/bash

if [ $# -lt 2 ]; then
	echo "usage: $0 <smpfile> <offset>"
	exit 1
fi

smpfile="$1"
offset=$2
outfile=$(printf "sample_%08x.raw" $offset)

head=$((`head -c 1 $smpfile | xxd -i`))

if [ $head -ne 0 ]; then dw2=1; else dw2=0; fi

if [ $dw2 = 1 ]; then
	echo "DW2 smp file"
	offset=$(($offset + 4))
else
	echo "DW1 smp file"
fi
	
lb=$(dd if=$smpfile bs=1 count=4 skip=$(($offset)) \
	2>/dev/null | xxd -i)

len=$((`echo $lb | cut -d, -f1` + \
	(`echo $lb | cut -d, -f2` * 256) + \
	(`echo $lb | cut -d, -f3` * 256 * 256) + \
	(`echo $lb | cut -d, -f4` * 256 * 256 * 256) ))

dd if=$smpfile bs=1 count=$len skip=$(($offset + 4)) of=$outfile

echo "length: $len"

