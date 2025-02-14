#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

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

// Mutex and condition variables for synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t plot_done = PTHREAD_COND_INITIALIZER;
int ready_to_plot = 0;
int plot_complete = 0;

// Function prototypes
void *computation_thread_func(void *arg);
void *visualization_thread_func(void *arg);

void *computation_thread_func(void *arg) {
    for (int t = 0; t < maxiter && population[w_plot]; t++) {
        // Update the grid
        population[w_update] = 0;
        for (int i = 1; i < nx - 1; i++) {
            for (int j = 1; j < ny - 1; j++) {
                int nn = currWorld[i + 1][j] + currWorld[i - 1][j] +
                         currWorld[i][j + 1] + currWorld[i][j - 1] +
                         currWorld[i + 1][j + 1] + currWorld[i - 1][j - 1] +
                         currWorld[i - 1][j + 1] + currWorld[i + 1][j - 1];

                nextWorld[i][j] = currWorld[i][j] ? (nn == 2 || nn == 3) : (nn == 3);
                population[w_update] += nextWorld[i][j];
            }
        }

        // Debug print for population
        printf("Computation: Population at iteration %d = %d\n", t, population[w_update]);

        // Signal the visualization thread
        pthread_mutex_lock(&mutex);
        ready_to_plot = 1;
        pthread_cond_signal(&cond);
        while (!plot_complete) {
            printf("Computation: Waiting for visualization to finish (iteration %d)\n", t);
            pthread_cond_wait(&plot_done, &mutex);
        }
        plot_complete = 0;
        pthread_mutex_unlock(&mutex);

        // Swap worlds
        tmesh = nextWorld;
        nextWorld = currWorld;
        currWorld = tmesh;

        // Swap population counts
        w_update = 1 - w_update;
        w_plot = 1 - w_plot;
    }
    pthread_exit(NULL);
}

void *visualization_thread_func(void *arg) {
    for (int t = 0; t < maxiter && population[w_plot]; t++) {
        // Wait for the computation thread to finish updating the grid
        pthread_mutex_lock(&mutex);
        while (!ready_to_plot) {
            printf("Visualization: Waiting for computation thread (iteration %d)\n", t);
            pthread_cond_wait(&cond, &mutex);
        }
        ready_to_plot = 0;
        pthread_mutex_unlock(&mutex);

        // Plot the grid
        printf("Visualization: Plotting iteration %d\n", t);
        MeshPlot(t, nx, ny, currWorld);

        // Signal the computation thread that plotting is done
        pthread_mutex_lock(&mutex);
        printf("Visualization: Signaling computation thread (iteration %d)\n", t);
        plot_complete = 1;
        pthread_cond_signal(&plot_done);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    int i, j, ac;

    /* Set default input parameters */
    float prob = 0.5;   /* Probability of placing a cell */
    long seedVal = 0;
    int game = 0;
    int s_step = 0;
    int disable_display = 0;

    /* Override with command-line input parameters (if any) */
    for (ac = 1; ac < argc; ac++) {
        if (MATCH("-n")) { nx = atoi(argv[++ac]); }
        else if (MATCH("-i")) { maxiter = atoi(argv[++ac]); }
        else if (MATCH("-p")) { prob = atof(argv[++ac]); }
        else if (MATCH("-s")) { seedVal = atof(argv[++ac]); }
        else if (MATCH("-step")) { s_step = 1; }
        else if (MATCH("-d")) { disable_display = 1; }
        else if (MATCH("-g")) { game = atoi(argv[++ac]); }
        else {
            printf("Usage: %s [-n <meshpoints>] [-i <iterations>] [-s seed] [-p prob] [-step] [-g <game #>] [-d]\n", argv[0]);
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

    // Create threads for computation and visualization
    pthread_t computation_thread, visualization_thread;
    pthread_create(&computation_thread, NULL, computation_thread_func, NULL);
    pthread_create(&visualization_thread, NULL, visualization_thread_func, NULL);

    // Wait for threads to finish
    pthread_join(computation_thread, NULL);
    pthread_join(visualization_thread, NULL);

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