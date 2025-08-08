/* Proyecto: simulacion de ecosistema con openmp
 objetivo: objetivo desarrollar una simulación de un ecosistema utilizando OpenMP para paralelizar operaciones

reglas del ecosistema: 
- reproduccion: plantas, hervivoros y carnivoros
- consumo de recursos: hervivoros y carnivoros
- movimiento: plantas, hervivoros y carnivoros
- muerte: plantas, hervivoros y carnivoros
- Interaccion entre especies: depredación, competencia por recursos
*/

//librerias:
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <omp.h>

//tamanios y valores fijos:
#define FILAS 8
#define COLUMNAS 8
#define MAX_TICKS 10

#define RESET   "\033[0m"
#define VERDE   "\033[0;32m"
#define AZUL    "\033[0;34m"
#define ROJO    "\033[0;31m"
#define GRIS    "\033[0;37m"


//tipos de ser vivos
typedef enum {
    VACIO, 
    PLANTA, 
    HERVIVORO, 
    CARNIVORO 
} TipoSerVivo;

typedef enum {
    NINGUNA,
    MOVER,
    COMER,
    REPRODUCIRSE,
    MORIR
} Accion;


//ser vivo
typedef struct {
    TipoSerVivo tipo;
    float vida;
    float energia;
    int edad;
    Accion accion;
} SerVivo;


typedef struct {
    SerVivo* ocupante;
} Celda;


Celda** crearMatriz(int filas, int cols) {
    Celda** grid = malloc(filas * sizeof(Celda*));
    for (int i = 0; i < filas; i++) {
        grid[i] = malloc(cols * sizeof(Celda));
        for (int j = 0; j < cols; j++) {
            grid[i][j].ocupante = NULL; //inicia vacio
        }
    }
    return grid;
}

//crear ser vivo random
SerVivo* crearRandom() {
    //random del 0 al 9
    int r = rand() % 10; 
    SerVivo* nuevo = malloc(sizeof(SerVivo));

    if (r < 4) {
        nuevo->tipo = PLANTA;
        nuevo->vida = 100;
        //no tiene energia ni edad, no se deberian de tomar en cuenta, pero estan porque esta en la estructura de ser vivo
        nuevo->energia = 0;
        nuevo->edad = 0;
        nuevo->accion = NINGUNA;

    }else if (r < 7) {      
        nuevo->tipo = HERVIVORO;
        nuevo->vida = 100;
        nuevo->energia = 70.00;
        nuevo->edad = 0;
        nuevo->accion = NINGUNA;

    }else if (r < 9) {
        nuevo->tipo = CARNIVORO;
        nuevo->vida = 100;
        nuevo->energia = 80.00;
        nuevo->edad = 0;
        nuevo->accion = NINGUNA;
    } 
    else { 
        //Free es para liberar la memoria cuando ya no lo necesita
        free(nuevo);
        return NULL;
    }

    return nuevo;
}

//llenar la matriz de seres vivos
void poblarMatriz(Celda** grid, int filas, int cols) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            grid[i][j].ocupante = crearRandom();
        }
    }
}

void imprimirMatriz(Celda** grid, int filas, int cols) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* s = grid[i][j].ocupante;
            if (s == NULL) {
                printf("B ");
            } else {
                switch (s->tipo) {
                    case PLANTA: printf(VERDE "P " RESET); break;
                    case HERVIVORO: printf(AZUL "H " RESET); break;
                    case CARNIVORO: printf(ROJO "C " RESET); break;
                    default: printf(GRIS "B " RESET);
                }
            }
        }
        printf("\n");
    }
}

void contarSeresVivos(Celda** grid, int filas, int cols, int* plantas, int* hervivoros, int* carnivoros) {
    int p = 0, h = 0, c = 0;

    #pragma omp parallel for reduction(+:p,h,c)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* s = grid[i][j].ocupante;
            if (s != NULL) {
                switch (s->tipo) {
                    case PLANTA: p++; break;
                    case HERVIVORO: h++; break;
                    case CARNIVORO: c++; break;
                    default: break;
                }
            }
        }
    }

    *plantas = p;
    *hervivoros = h;
    *carnivoros = c;
}

void limpiarAcciones(Celda** grid, int filas, int cols) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* s = grid[i][j].ocupante;
            if (s != NULL) {
                s->accion = NINGUNA;
            }
        }
    }
}

void reproducirPlantas(Celda** grid, int filas, int cols) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* ocupante = grid[i][j].ocupante;
            if (ocupante && ocupante->tipo == PLANTA && ocupante->accion == NINGUNA) {

                if ((rand() % 100) < 30) {
                    for (int dx = -1; dx <= 1; dx++) {
                        for (int dy = -1; dy <= 1; dy++) {
                            if (dx == 0 && dy == 0) continue;

                            int ni = i + dx;
                            int nj = j + dy;

                            if (ni >= 0 && ni < filas && nj >= 0 && nj < cols) {
                                if (grid[ni][nj].ocupante == NULL) {
                                    SerVivo* nueva = malloc(sizeof(SerVivo));
                                    nueva->tipo = PLANTA;
                                    nueva->vida = 100;
                                    nueva->energia = 0;
                                    nueva->edad = 0;
                                    nueva->accion = NINGUNA;

                                    #pragma omp critical
                                    {
                                        if (grid[ni][nj].ocupante == NULL) {
                                            grid[ni][nj].ocupante = nueva;
                                        } else {
                                            free(nueva);
                                        }
                                    }
                                    ocupante->accion = REPRODUCIRSE;
                                    goto siguiente_planta;
                                }
                            }
                        }
                    }
                }
            }
siguiente_planta:;
        }
    }
}

void reproducirHervivoros(Celda** grid, int filas, int cols) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* h = grid[i][j].ocupante;

            if (h && h->tipo == HERVIVORO && h->accion == NINGUNA && h->energia >= 3.0f) {
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) continue;

                        int ni = i + dx;
                        int nj = j + dy;

                        if (ni >= 0 && ni < filas && nj >= 0 && nj < cols) {
                            if (grid[ni][nj].ocupante == NULL) {
                                SerVivo* hijo = malloc(sizeof(SerVivo));
                                hijo->tipo = HERVIVORO;
                                hijo->vida = 100;
                                hijo->energia = 2.0f;
                                hijo->edad = 0;
                                hijo->accion = NINGUNA;

                                #pragma omp critical
                                {
                                    if (grid[ni][nj].ocupante == NULL) {
                                        grid[ni][nj].ocupante = hijo;
                                        h->energia -= 2.0f;
                                        h->accion = REPRODUCIRSE;
                                    } else {
                                        free(hijo);
                                    }
                                }
                                goto siguiente;
                            }
                        }
                    }
                }
            }
siguiente:;
        }
    }
}

void reproducirCarnivoros(Celda** grid, int filas, int cols) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* c = grid[i][j].ocupante;

            if (c && c->tipo == CARNIVORO && c->accion == NINGUNA && c->energia >= 3.0f) {
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) continue;

                        int ni = i + dx;
                        int nj = j + dy;

                        if (ni >= 0 && ni < filas && nj >= 0 && nj < cols) {
                            if (grid[ni][nj].ocupante == NULL) {
                                SerVivo* hijo = malloc(sizeof(SerVivo));
                                hijo->tipo = CARNIVORO;
                                hijo->vida = 100;
                                hijo->energia = 2.0f;
                                hijo->edad = 0;
                                hijo->accion = NINGUNA;

                                #pragma omp critical
                                {
                                    if (grid[ni][nj].ocupante == NULL) {
                                        grid[ni][nj].ocupante = hijo;
                                        c->energia -= 2.0f;
                                        c->accion = REPRODUCIRSE;
                                    } else {
                                        free(hijo);
                                    }
                                }
                                goto siguiente;
                            }
                        }
                    }
                }
            }
siguiente:;
        }
    }
}



/*
Pseudocodigo del sistema:
Inicializar cuadrícula y especies
Para cada tick de la simulación: 
    #pragma omp parallel for 
    Para cada celda en la cuadrícula: 
        Actualizar estado de las plantas 
        Actualizar estado de los herbívoros 
        Actualizar estado de los carnívoros 
Sincronizar datos de especies entre hilos 
Mostrar estado del ecosistema 
Fin Para
*/
int main(){
    //Inicializar cuadrícula y especies
    int semilla = 60;
    srand(semilla);
    Celda** mundo = crearMatriz(FILAS, COLUMNAS);
    poblarMatriz(mundo, FILAS, COLUMNAS);
    int plantas = 0, hervivoros = 0, carnivoros = 0;
    contarSeresVivos(mundo, FILAS, COLUMNAS, &plantas, &hervivoros, &carnivoros);
    printf("\nPlantas: %d\nHervivoros: %d\nCarnivoros: %d\n", plantas, hervivoros, carnivoros);
    printf("Distribucion inicial:\n");
    imprimirMatriz(mundo, FILAS, COLUMNAS);
    printf("\n\n");

    //Para cada tick de la simulación: 
    for (int tick=0; tick < MAX_TICKS; tick++){
        printf("tick: %d\n", tick);
        //para cada celda: actualizar plantas, herbivoros, carnivoros
        //#pragma omp parallel for 

        // Reproduccion
        reproducirPlantas(mundo, FILAS, COLUMNAS); 
        reproducirHervivoros(mundo, FILAS, COLUMNAS);
        reproducirCarnivoros(mundo, FILAS, COLUMNAS);

        plantas = 0;
        hervivoros = 0;
        carnivoros = 0;

        contarSeresVivos(mundo, FILAS, COLUMNAS, &plantas, &hervivoros, &carnivoros);

        printf("Plantas: %d\nHervivoros: %d\nCarnivoros: %d\n", plantas, hervivoros, carnivoros);
        printf("Distribucion:\n");
        imprimirMatriz(mundo, FILAS, COLUMNAS);
        printf("\n\n");
        

    }

    //sincronicar datos de especies entre hilos

    //mostrar estado del ecosistema
    

    return 0;
}