# miniproyecto openmp paralela

## como correr el miniproyecto:

### Windows
primero instalr minWG
```
gcc --version
```
Compilar main: (la W es para diferenciar entre linux y windows)
```
gcc main.c -o mainW
```
### Linux
Primero debe tener instalado openmp, verificar con el siguiente comando:
```
gcc -fopenmp --version
```

Compilar main:
```
gcc -fopenmp main.c -o main
```
Luego:
```
./main
```
