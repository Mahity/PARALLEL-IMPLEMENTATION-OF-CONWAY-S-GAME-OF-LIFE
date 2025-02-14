#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h> // Include Pthreads header

#define MATCH(s) (!strcmp(argv[ac], (s)))

int MeshPlot(int t, int m, int n, char **mesh);

double real_rand();
int seed_rand(long sd);

static char **currWorld = NULL, **nextWorld = NULL, **tmesh = NULL;
static int maxiter = 200; /* number of iteration timesteps */
static int population[2] = {0, 0}; /* number of live cells */

int nx = 100;      /* number of mesh points in the x dimension */
int ny = 100;      /* number of mesh points in the y dimension */

static int w_update = 0;
static int w_plot = 1;

double getTime();
extern FILE *gnu;

typedef struct {
    int start_row;
    int end_row;
    int nx;
    int ny;
    char **currWorld;
    char **nextWorld;
    int *population;
} ThreadData;

void *update_world_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int i, j, nn;
    int local_population = 0;

    for (i = data->start_row; i < data->end_row; i++) {
        for (j = 1; j < data->ny - 1; j++) {
            nn = data->currWorld[i + 1][j] + data->currWorld[i - 1][j] +
                 data->currWorld[i][j + 1] + data->currWorld[i][j - 1] +
                 data->currWorld[i + 1][j + 1] + data->currWorld[i - 1][j - 1] +
                 data->currWorld[i - 1][j + 1] + data->currWorld[i + 1][j - 1];

            data->nextWorld[i][j] = data->currWorld[i][j] ? (nn == 2 || nn == 3) : (nn == 3);
            local_population += data->nextWorld[i][j];
        }
    }

    // Update the global population count
    *(data->population) += local_population;
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    int i, j, ac;

    /* Set default input parameters */
    float prob = 0.5;   /* Probability of placing a cell */
    long seedVal = 0;
    int game = 0;
    int s_step = 0;
    int numthreads = 1;
    int disable_display = 0;

    /* Override with command-line input parameters (if any) */
    for (ac = 1; ac < argc; ac++) {
        if (MATCH("-n")) { nx = atoi(argv[++ac]); }
        else if (MATCH("-i")) { maxiter = atoi(argv[++ac]); }
        else if (MATCH("-t")) { numthreads = atof(argv[++ac]); }
        else if (MATCH("-p")) { prob = atof(argv[++ac]); }
        else if (MATCH("-s")) { seedVal = atof(argv[++ac]); }
        else if (MATCH("-step")) { s_step = 1; }
        else if (MATCH("-d")) { disable_display = 1; }
        else if (MATCH("-g")) { game = atoi(argv[++ac]); }
        else {
            printf("Usage: %s [-n <meshpoints>] [-i <iterations>] [-s seed] [-p prob] [-t numthreads] [-step] [-g <game #>] [-d]\n", argv[0]);
            return -1;
        }
    }

    int rs = seed_rand(seedVal);

    /* Increment sizes to account for boundary ghost cells */
    nx = nx + 2;
    ny = nx;

    /* Allocate contiguous memory for two 2D arrays */
    currWorld = (char **)malloc(sizeof(char *) * nx + sizeof(char) * nx * ny);
    for (i = 0; i < nx; i++)
        currWorld[i] = (char *)(currWorld + nx) + i * ny;

    nextWorld = (char **)malloc(sizeof(char *) * nx + sizeof(char) * nx * ny);
    for (i = 0; i < nx; i++)
        nextWorld[i] = (char *)(nextWorld + nx) + i * ny;

    /* Set boundary ghost cells to '0' */
    for (i = 0; i < nx; i++) {
        currWorld[i][0] = 0;
        currWorld[i][ny - 1] = 0;
        nextWorld[i][0] = 0;
        nextWorld[i][ny - 1] = 0;
    }
    for (i = 0; i < ny; i++) {
        currWorld[0][i] = 0;
        currWorld[nx - 1][i] = 0;
        nextWorld[0][i] = 0;
        nextWorld[nx - 1][i] = 0;
    }

    // Generate a world
    if (game == 0) { // Random input
        for (i = 1; i < nx - 1; i++) {
            for (j = 1; j < ny - 1; j++) {
                currWorld[i][j] = (real_rand() < prob);
                population[w_plot] += currWorld[i][j];
            }
        }
    } else if (game == 1) { // Block, still life
        printf("2x2 Block, still life\n");
        int nx2 = nx / 2;
        int ny2 = ny / 2;
        currWorld[nx2][ny2] = 1;
        currWorld[nx2 + 1][ny2] = 1;
        currWorld[nx2][ny2 + 1] = 1;
        currWorld[nx2 + 1][ny2 + 1] = 1;
        population[w_plot] = 4;
    } else if (game == 2) { // Gosper Glider Gun
        printf("Gosper Glider Gun\n");

        // Initialize the world with DEAD cells
        for (i = 1; i < nx - 1; i++) {
            for (j = 1; j < ny - 1; j++) {
                currWorld[i][j] = 0;
            }
        }

        // Gosper Glider Gun pattern
        int gun[36][2] = {
            {5, 1}, {5, 2}, {6, 1}, {6, 2},  // Left square
            {5, 11}, {6, 11}, {7, 11},       // Right square
            {4, 12}, {8, 12},                // Right square extensions
            {3, 13}, {9, 13},                // Right square extensions
            {3, 14}, {9, 14},                // Right square extensions
            {6, 15},                         // Right square extensions
            {4, 16}, {8, 16},                // Right square extensions
            {5, 17}, {6, 17}, {7, 17},       // Right square extensions
            {6, 18},                         // Right square extensions
            {3, 21}, {4, 21}, {5, 21},       // Left gun structure
            {3, 22}, {4, 22}, {5, 22},       // Left gun structure
            {2, 23}, {6, 23},                // Left gun structure
            {1, 25}, {2, 25}, {6, 25}, {7, 25},  // Left gun structure
            {3, 35}, {4, 35}, {3, 36}, {4, 36}   // Left gun structure
        };

        // Place the gun in the world
        for (i = 0; i < 36; i++) {
            int x = gun[i][0] + nx / 2 - 20;  // Center the gun horizontally
            int y = gun[i][1] + ny / 2 - 20;  // Center the gun vertically
            if (x >= 1 && x < nx - 1 && y >= 1 && y < ny - 1) {
                currWorld[x][y] = 1;
            }
        }

        population[w_plot] = 36;  // Number of live cells in the Gosper Glider Gun
    } else {
        printf("Unknown game %d\n", game);
        exit(-1);
    }

    printf("probability: %f\n", prob);
    printf("Random # generator seed: %d\n", rs);

    /* Plot the initial data */
    if (!disable_display)
        MeshPlot(0, nx, ny, currWorld);

    /* Perform updates for maxiter iterations */
    double t0 = getTime();
    int t;

    for (t = 0; t < maxiter && population[w_plot]; t++) {
        /* Use currWorld to compute the updates and store it in nextWorld */
        population[w_update] = 0;

        pthread_t threads[numthreads];
        ThreadData thread_data[numthreads];
        int rows_per_thread = (nx - 2) / numthreads;

        for (i = 0; i < numthreads; i++) {
            thread_data[i].start_row = 1 + i * rows_per_thread;
            thread_data[i].end_row = (i == numthreads - 1) ? nx - 1 : 1 + (i + 1) * rows_per_thread;
            thread_data[i].nx = nx;
            thread_data[i].ny = ny;
            thread_data[i].currWorld = currWorld;
            thread_data[i].nextWorld = nextWorld;
            thread_data[i].population = &population[w_update];
            pthread_create(&threads[i], NULL, update_world_thread, &thread_data[i]);
        }

        for (i = 0; i < numthreads; i++) {
            pthread_join(threads[i], NULL);
        }

        /* Pointer Swap: nextWorld <-> currWorld */
        tmesh = nextWorld;
        nextWorld = currWorld;
        currWorld = tmesh;

        /* Start the new plot */
        if (!disable_display)
            MeshPlot(t, nx, ny, currWorld);

        if (s_step) {
            printf("Finished with step %d\n", t);
            printf("Press enter to continue.\n");
            getchar();
        }
    }

    double t1 = getTime();
    printf("Running time for the iterations: %f sec.\n", t1 - t0);
    printf("Press enter to end.\n");
    getchar();

    if (gnu != NULL)
        pclose(gnu);

    /* Free resources */
    free(nextWorld);
    free(currWorld);

    return 0;
}