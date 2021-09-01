#!/bin/sh
### note: sh reorder.sh in_fname out_fname  ###

infile=$1
outfile=$2
line_num=`cat $infile | wc -l `
./random $line_num $infile $outfile.tmp
sort $outfile.tmp -k 3 -n -t '	' | cut -f1-2  > $outfile
