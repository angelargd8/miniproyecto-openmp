/* Proyecto: simulacion de ecosistema con openmp
 objetivo: objetivo desarrollar una simulación de un ecosistema utilizando OpenMP para paralelizar operaciones

reglas del ecosistema: 
- reproduccion: plantas, hervivoros y carnivoros
- consumo de recursos: hervivoros y carnivoros
- movimiento: plantas, hervivoros y carnivoros
- muerte: plantas, hervivoros y carnivoros
- Interaccion entre especies: depredación, competencia por recursos
*/

// ===================================================
// =============== LIBRERÍAS Y CONSTANTES ============
// ===================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

//tamanios y valores fijos:
#define FILAS 8
#define COLUMNAS 8
#define MAX_TICKS 12

#define RESET   "\033[0m"
#define VERDE   "\033[0;32m"
#define AZUL    "\033[0;34m"
#define ROJO    "\033[0;31m"
#define GRIS    "\033[0;37m"

// ===================================================
// =================== ENUMS Y ESTRUCTURAS ===========
// ===================================================

/*
En esta sección del código se definen estructuras y tipos para representar un ecosistema
    en el que conviven diferentes tipos de seres vivos, con estados 
    y acciones posibles.
*/
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


// ===================================================
// ================== FUNCIONES HELPERS ==============
// ===================================================


/*
    Reserva memoria dinámica para una matriz de celdas que representa 
    el ecosistema y la inicializa con celdas vacías.

    Parámetros:
        - filas: número de filas de la matriz.
        - cols: número de columnas de la matriz.

    Retorna:
        - Un puntero doble a Celda, que apunta a la matriz creada.
*/
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

/*
Crea un ser vivo random 
*/

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

/*Recorre la matriz de celdas y muestra en consola el contenido de cada posición.
    - Si la celda está vacía, imprime una "B" (vacío).
    - Si hay un ser vivo, imprime su símbolo con color según su tipo:
        - P: Planta (VERDE)
        - H: Herbívoro (AZUL)
        - C: Carnívoro (ROJO)

    Parámetros:
        - grid: matriz de celdas (Celda**).
        - filas: número de filas de la matriz.
        - cols: número de columnas de la matriz.
*/
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
/*
    -------------------------
    Recorre la matriz y cuenta cuántos seres vivos de cada tipo hay.
    El conteo se realiza en paralelo usando OpenMP para mejorar el rendimiento.

    Parámetros:
        - grid: matriz de celdas (Celda**).
        - filas: número de filas de la matriz.
        - cols: número de columnas de la matriz.
        - plantas: puntero a entero donde se almacenará el número de plantas.
        - hervivoros: puntero a entero donde se almacenará el número de herbívoros.
        - carnivoros: puntero a entero donde se almacenará el número de carnívoros.

        - Se usa `#pragma omp parallel for reduction(+:p,h,c)` para sumar 
          en paralelo sin condiciones de carrera.
*/
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

// ===================================================
// ============== ESTADO Y LIMPIEZA ==================
// ===================================================
/**
 * @brief Actualiza el estado de todos los seres vivos en la matriz (edad, energía, etc.).
 * 
 * @param grid Matriz de celdas a actualizar.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 */
void actualizarEstado(Celda** grid, int filas, int cols) {
    #pragma omp for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* s = grid[i][j].ocupante;
            if (s != NULL) {
                s->edad += 1;

                if (s->tipo == HERVIVORO || s->tipo == CARNIVORO) {
                    s->energia -= 1.0f;
                }
            }
        }
    }
}


/**
 * @brief Verifica si una planta está rodeada por otros seres vivos.
 * 
 * @param grid Matriz de celdas.
 * @param i Índice de fila de la planta.
 * @param j Índice de columna de la planta.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 * @return int 1 si la planta está rodeada, 0 en caso contrario.
 */
// planta encerrada
static inline int ansiedadPlantas(Celda** grid, int i, int j, int filas, int cols) {
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int ni = i + dx, nj = j + dy;
            if (ni >= 0 && ni < filas && nj >= 0 && nj < cols) {
                if (grid[ni][nj].ocupante == NULL) {
                    return 0;
                }
            }
        }
    }
    return 1;
}


/**
 * @brief Elimina los seres vivos muertos de la matriz según su estado.
 * 
 * @param grid Matriz de celdas a limpiar.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 */
void limpiarMuertos(Celda** grid, int filas, int cols) {
    #pragma omp for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* s = grid[i][j].ocupante;
            if (s != NULL) {
                int eliminar = 0;

                switch (s->tipo) {
                    case PLANTA:
                        if (s->edad > 10 || ansiedadPlantas(grid, i, j, filas, cols)) eliminar = 1;
                        break;
                    case HERVIVORO:
                        if (s->edad > 15 || s->energia < -3.0f) eliminar = 1;
                        break;
                    case CARNIVORO:
                        if (s->edad > 20 || s->energia < -3.0f) eliminar = 1;
                        break;
                    default:
                        break;
                }

                if (eliminar) {
                    free(s);
                    grid[i][j].ocupante = NULL;
                }
            }
        }
    }
}


/**
 * @brief Reinicia las acciones de todos los seres vivos en la matriz a NINGUNA.
 * 
 * @param grid Matriz de celdas a actualizar.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 */
void limpiarAcciones(Celda** grid, int filas, int cols) {
    #pragma omp for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* s = grid[i][j].ocupante;
            if (s != NULL) {
                s->accion = NINGUNA;
            }
        }
    }
}

// ===================================================
// ==================== REPRODUCCIÓN =================
// ===================================================



/**
 * @brief Maneja la reproducción de las plantas en la matriz.
 * 
 * @param grid Matriz de celdas.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 */
void reproducirPlantas(Celda** grid, int filas, int cols) {
    #pragma omp for collapse(2)
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


/**
 * @brief Maneja la reproducción de los herbívoros en la matriz.
 * 
 * @param grid Matriz de celdas.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 */
void reproducirHervivoros(Celda** grid, int filas, int cols) {
    #pragma omp for collapse(2)
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



/**
 * @brief Maneja la reproducción de los carnívoros en la matriz.
 * 
 * @param grid Matriz de celdas.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 */
void reproducirCarnivoros(Celda** grid, int filas, int cols) {
    #pragma omp for collapse(2)
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

// ===================================================
// ================ CONSUMO DE RECURSOS ==============
// ===================================================



/**
 * @brief Maneja el consumo de plantas por parte de los herbívoros en la matriz.
 * 
 * @param grid Matriz de celdas.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 */
void herbivorosConsume(Celda** grid, int filas, int cols) {
    #pragma omp for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* h = grid[i][j].ocupante;

            if (h && h->tipo == HERVIVORO && h->accion == NINGUNA) {
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) continue;

                        int ni = i + dx;
                        int nj = j + dy;

                        if (ni >= 0 && ni < filas && nj >= 0 && nj < cols) {

                            SerVivo* vecino = grid[ni][nj].ocupante;
                            if (vecino && vecino->tipo == PLANTA) {

                                #pragma omp critical
                                {
                                    if (grid[ni][nj].ocupante && grid[ni][nj].ocupante->tipo == PLANTA && (rand() % 100) < 50) {
                                        free(grid[ni][nj].ocupante);
                                        grid[ni][nj].ocupante = NULL;
                                        h->energia += 1.0f;
                                        h->accion = COMER;
                                    }
                                }
                                goto siguiente_herbivoro;
                            }
                        }
                    }
                }
            }
siguiente_herbivoro:;
        }
    }
}


/**
 * @brief Maneja el consumo de herbívoros o plantas por parte de los carnívoros en la matriz.
 * 
 * @param grid Matriz de celdas.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 */
void carnivorosConsume(Celda** grid, int filas, int cols) {
    #pragma omp for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* c = grid[i][j].ocupante;

            if (c && c->tipo == CARNIVORO && c->accion == NINGUNA) {
                
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) continue;

                        int ni = i + dx;
                        int nj = j + dy;

                        if (ni >= 0 && ni < filas && nj >= 0 && nj < cols) {
                            
                            SerVivo* vecino = grid[ni][nj].ocupante;
                            if (vecino && vecino->tipo == HERVIVORO) {

                                #pragma omp critical
                                {
                                    if (grid[ni][nj].ocupante && grid[ni][nj].ocupante->tipo == HERVIVORO && (rand() % 100) < 50) {

                                        free(grid[ni][nj].ocupante);
                                        grid[ni][nj].ocupante = NULL;
                                        c->energia += 2.0f;
                                        c->accion = COMER;
                                    }
                                }
                                goto siguiente_carnivoro;
                            } else if (vecino && vecino->tipo == PLANTA) {

                                #pragma omp critical
                                {
                                    if (grid[ni][nj].ocupante && grid[ni][nj].ocupante->tipo == PLANTA && (rand() % 100) < 50) {

                                        free(grid[ni][nj].ocupante);
                                        grid[ni][nj].ocupante = NULL;
                                        c->energia += 1.0f;
                                        c->accion = COMER;
                                    }
                                }
                                goto siguiente_carnivoro;
                            }
                        }
                    }
                }
            }
siguiente_carnivoro:;
        }
    }
}


// ===================================================
// ======================== MOVIMIENTO =====================
// ===================================================



/**
 * @brief Mueve a los herbívoros en la matriz, evitando depredadores.
 * 
 * @param grid Matriz de celdas.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 */
// Movimiento de Herbívoros
void moverHerbivoros(Celda** grid, int filas, int cols) {
    #pragma omp for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* h = grid[i][j].ocupante;
            if (h && h->tipo == HERVIVORO && h->accion == NINGUNA) {
                
                int peligro = 0;
                // Detectar si hay un carnívoro cerca
                for (int dx = -1; dx <= 1 && !peligro; dx++) {
                    for (int dy = -1; dy <= 1 && !peligro; dy++) {
                        if (dx == 0 && dy == 0) continue;
                        int ni = i + dx, nj = j + dy;
                        if (ni >= 0 && ni < filas && nj >= 0 && nj < cols) {
                            SerVivo* vecino = grid[ni][nj].ocupante;
                            if (vecino && vecino->tipo == CARNIVORO) {
                                peligro = 1;
                            }
                        }
                    }
                }

                // Intentar moverse a celda vacía
                int dirs[8][2] = {
                    {-1, 0}, {1, 0}, {0, -1}, {0, 1},
                    {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
                };
                int mov_realizado = 0;
                for (int intento = 0; intento < 8 && !mov_realizado; intento++) {
                    // Selecciona aleatoriamente una de las 8 direcciones posibles
                    int idx = rand() % 8; // aleatorio
                    int ni = i + dirs[idx][0];
                    int nj = j + dirs[idx][1];
                    if (ni >= 0 && ni < filas && nj >= 0 && nj < cols) {
                        if (grid[ni][nj].ocupante == NULL) {  // Comprueba si la celda destino está vacía
                            #pragma omp critical
                            {
                                // cambio de celda
                                if (grid[ni][nj].ocupante == NULL) {
                                    grid[ni][nj].ocupante = h;
                                    grid[i][j].ocupante = NULL;
                                    h->accion = MOVER;
                                    mov_realizado = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}




/**
 * @brief Mueve a los carnívoros en la matriz, buscando presas.
 * 
 * @param grid Matriz de celdas.
 * @param filas Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 */
// Movimiento de Carnívoros
void moverCarnivoros(Celda** grid, int filas, int cols) {
    #pragma omp for collapse(2)
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            SerVivo* c = grid[i][j].ocupante;
            if (c && c->tipo == CARNIVORO && c->accion == NINGUNA) {

                int presa_cerca = 0;
                // Detectar si hay herbívoro cerca
                for (int dx = -1; dx <= 1 && !presa_cerca; dx++) {
                    for (int dy = -1; dy <= 1 && !presa_cerca; dy++) {
                        if (dx == 0 && dy == 0) continue;
                        int ni = i + dx, nj = j + dy;
                        if (ni >= 0 && ni < filas && nj >= 0 && nj < cols) {
                            SerVivo* vecino = grid[ni][nj].ocupante;
                            if (vecino && vecino->tipo == HERVIVORO) {
                                presa_cerca = 1;
                            }
                        }
                    }
                }

                // Si no hay presa cerca, moverse
                if (!presa_cerca) {
                    int dirs[8][2] = {
                        {-1, 0}, {1, 0}, {0, -1}, {0, 1},
                        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
                    };
                    int mov_realizado = 0;
                    for (int intento = 0; intento < 8 && !mov_realizado; intento++) {
                        int idx = rand() % 8;
                        int ni = i + dirs[idx][0];
                        int nj = j + dirs[idx][1];
                        if (ni >= 0 && ni < filas && nj >= 0 && nj < cols) {
                            if (grid[ni][nj].ocupante == NULL) {
                                #pragma omp critical
                                {
                                    if (grid[ni][nj].ocupante == NULL) {
                                        grid[ni][nj].ocupante = c;
                                        grid[i][j].ocupante = NULL;
                                        c->accion = MOVER;
                                        mov_realizado = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
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
// ===================================================
// ======================== MAIN =====================
// ===================================================
int main(){
    // Inicializar cuadrícula y especies
    int semilla = 60;
    srand(semilla);
    Celda** mundo = crearMatriz(FILAS, COLUMNAS);
    poblarMatriz(mundo, FILAS, COLUMNAS);
    int plantas = 0, hervivoros = 0, carnivoros = 0;
    
    printf("Distribucion inicial:\n");
    contarSeresVivos(mundo, FILAS, COLUMNAS, &plantas, &hervivoros, &carnivoros);
    printf("\nPlantas: %d\nHervivoros: %d\nCarnivoros: %d\n", plantas, hervivoros, carnivoros);
    imprimirMatriz(mundo, FILAS, COLUMNAS);
    printf("\n\n");

    // Para cada tick de la simulación
    for (int tick = 0; tick < MAX_TICKS; tick++){
        printf("tick: %d\n", tick);

        #pragma omp parallel
        {
            // Movimiento (huida/búsqueda)
            moverHerbivoros(mundo, FILAS, COLUMNAS);
            moverCarnivoros(mundo, FILAS, COLUMNAS);

            // Consumo de recursos
            herbivorosConsume(mundo, FILAS, COLUMNAS);
            carnivorosConsume(mundo, FILAS, COLUMNAS);

            // Reproducción
            reproducirPlantas(mundo, FILAS, COLUMNAS); 
            reproducirHervivoros(mundo, FILAS, COLUMNAS);
            reproducirCarnivoros(mundo, FILAS, COLUMNAS);

            // Actualización y limpieza
            actualizarEstado(mundo, FILAS, COLUMNAS);
            limpiarMuertos(mundo, FILAS, COLUMNAS);
        }

        // Contar y mostrar estado
        plantas = hervivoros = carnivoros = 0;
        printf("Distribucion:\n");
        contarSeresVivos(mundo, FILAS, COLUMNAS, &plantas, &hervivoros, &carnivoros);
        printf("Plantas: %d\nHervivoros: %d\nCarnivoros: %d\n", plantas, hervivoros, carnivoros);
        imprimirMatriz(mundo, FILAS, COLUMNAS);
        printf("\n\n");
    }

    return 0;
}
