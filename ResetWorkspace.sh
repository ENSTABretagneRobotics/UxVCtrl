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
cp -f restore/log/Readme.txt log/Readme.txt
cp -f restore/pic/Readme.txt pic/Readme.txt
cp -f restore/vid/Readme.txt vid/Readme.txt
cp -f restore/aud/Readme.txt aud/Readme.txt
