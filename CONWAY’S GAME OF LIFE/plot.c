#include <stdio.h>

/* Function to plot the 2D array
 * 'gnuplot' is instantiated via a pipe and 
 * the values to be plotted are passed through, along 
 * with gnuplot commands */

FILE *gnu = NULL;

int MeshPlot(int t, int m, int n, char **mesh) {
    int i, j;
    char iter[60];
    sprintf(iter, "\"Iter = %d\"", t);

    if (gnu == NULL) {
        gnu = popen("gnuplot -persist", "w");  // Use -persist to keep the window open
        if (gnu == NULL) {
            fprintf(stderr, "Error: Could not open gnuplot.\n");
            return -1;
        }
    }

    // Send commands to gnuplot
    fprintf(gnu, "set title %s\n", iter);
    fprintf(gnu, "set size square\n");
    fprintf(gnu, "set key off\n");
    fprintf(gnu, "set grid\n");  // Enable grid lines
    fprintf(gnu, "set xrange [0:%d]\n", n - 1);  // Set x-axis range
    fprintf(gnu, "set yrange [0:%d]\n", m - 1);  // Set y-axis range
    fprintf(gnu, "plot '-' with points pointtype 7 pointsize 2\n");  // Plot live cells

    // Send the world data to gnuplot
    for (i = 1; i < m - 1; i++) {
        for (j = 1; j < n - 1; j++) {
            if (mesh[i][j]) {
                fprintf(gnu, "%d %d\n", j, m - i - 1);  // Send (x, y) coordinates
            }
        }
    }

    fprintf(gnu, "e\n");
    fflush(gnu);  // Flush the output to gnuplot

    return 0;
}