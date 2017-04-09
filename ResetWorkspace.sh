#!/bin/sh
rm -f *.o
rm -Rf log
rm -Rf pic
rm -Rf vid
rm -Rf aud
mkdir -p log
mkdir -p pic
mkdir -p vid
mkdir -p aud
cp -f restore/log/ReadMe.txt log/ReadMe.txt
cp -f restore/pic/ReadMe.txt pic/ReadMe.txt
cp -f restore/vid/ReadMe.txt vid/ReadMe.txt
cp -f restore/aud/ReadMe.txt aud/ReadMe.txt
echo " "
