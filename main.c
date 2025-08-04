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

//tipos de ser vivos
typedef enum {
    VACIO, 
    PLANTA, 
    HERVIVORO, 
    CARNIVORO 
} TipoSerVivo;

//ser vivo
typedef struct {
    TipoSerVivo tipo;
    float vida;
    float energia;
    int edad;
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

    }else if (r < 7) {      
        nuevo->tipo = HERVIVORO;
        nuevo->vida = 100;
        nuevo->energia = 70.00;
        nuevo->edad = 0;

    }else if (r < 9) {
        nuevo->tipo = CARNIVORO;
        nuevo->vida = 100;
        nuevo->energia = 80.00;
        nuevo->edad = 0;
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
                printf("⬜ ");
            } else {
                switch (s->tipo) {
                    case PLANTA: printf("🌿 "); break;
                    case HERVIVORO: printf("🦓 "); break;
                    case CARNIVORO: printf("🦁 "); break;
                    default: printf("⬜ ");
                }
            }
        }
        printf("\n");
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
    imprimirMatriz(mundo, FILAS, COLUMNAS);

    //Para cada tick de la simulación: 
    for (int tick=0; tick < MAX_TICKS; tick++){
        printf("tick: %d\n", tick);
        //para cada celda: actualizar plantas, herbivoros, carnivoros
        //#pragma omp parallel for 
        
        
    }

    //sincronicar datos de especies entre hilos

    //mostrar estado del ecosistema
    

    return 0;
}