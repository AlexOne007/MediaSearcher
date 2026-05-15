# Инструкция по сборке
## Требования
* C++20
* CMake
* Crow

Пример сборки
```
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg
cmake --build build -j
```

Запуск:
```
program <seconds> [<directory> [http|file [config_file]]]
    <seconds> - update interval in seconds;
    <directory> - directory to traverse, default value is $HOME;
    <http|file - output mode (http or file), default file;
    <config_file> - file with extensions to treat as media;
```
После запуск программу можно остановить, введя `exit`.
