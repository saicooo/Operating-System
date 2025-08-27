#!/bin/bash

mkdir dir

size=$(du -s --block-size=1 dir | awk '{print $1}')
echo "Начальный размер директории: $size байт"

for i in {1..50}; do
    touch dir/$i.txt
    echo "qwertyqwerty" >> dir/$i.txt
done

size=$(du -s --block-size=1 dir | cut -f 1)
echo "Размер после создания 50 файлов: $size байт"

for i in {1..25}; do
    rm dir/$i.txt
done

size=$(du -s --block-size=1 dir | cut -f 1)
echo "Размер после удаления 25 файлов: $size байт"

for i in {51..80}; do
    touch dir/$i.txt
    echo "qwertyqwerty" >> dir/$i.txt
done

size=$(du -s --block-size=1 dir | cut -f 1)
echo "Размер после добавления 30 файлов: $size байт"

for i in {51..60}; do
    rm dir/$i.txt
done

size=$(du -s --block-size=1 dir | cut -f 1)
echo "Размер после удаления 10 файлов: $size байт"

for i in {81..180}; do
    touch dir/$i.txt
    echo "qwertyqwerty" >> dir/$i.txt
done

size=$(du -s --block-size=1 dir | cut -f 1)
echo "Размер после добавления 100 файлов: $size байт"

rm -f dir/*.txt

size=$(du -s --block-size=1 dir | cut -f 1)
echo "Размер после удаления всех файлов: $size байт"

rm -rf dir