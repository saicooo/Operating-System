#!/bin/bash

echo "Поиск самого глубокого пути в системе..."
deepest_path=$(sudo find / 2>/dev/null -type d -o -type f | awk -F/ '{ print NF-1, $0 }' | sort -n | tail -n 1)
echo "Самый глубокий путь: $deepest_path"

echo "Создание папки с вложенностью 17 уровней..."
mkdir -p folder1/folder2/folder3/folder4/folder5/folder6/folder7/folder8/folder9/folder10/folder11/folder12/folder13/folder14/folder15/folder16/folder17

echo "Поиск самого глубокого пути в системе..."
deepest_path=$(sudo find / 2>/dev/null -type d -o -type f | awk -F/ '{ print NF-1, $0 }' | sort -n | tail -n 1)
echo "Самый глубокий путь: $deepest_path"

echo "Поиск самого длинного пути в системе..."
longest_path=$(sudo find / 2>/dev/null -type d -o -type f | awk '{ print length, $0 }' | sort -n | tail -n 1)
echo "Самый длинный путь: $longest_path"

echo "Удаление папки..."
rm -rf folder1
echo "Папка удалена."