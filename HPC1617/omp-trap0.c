/****************************************************************************
 *
 * omp-trap0.c - Trapezoid rule with OpenMP; reworked version of the
 * code from http://www.cs.usfca.edu/~peter/ipp/
 *
 * Original by Peter Pacheco
 * Modifications 2016, 2017 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
 *
 * To the extent possible under law, the author(s) have dedicated all 
 * copyright and related and neighboring rights to this software to the 
 * public domain worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see 
 * <http://creativecommons.org/publicdomain/zero/1.0/>. 
 *
 * --------------------------------------------------------------------------
 *
 * Compile with:
 * gcc -std=c99 -Wall -Wpedantic -fopenmp omp-trap0.c -o omp-trap0 -lgomp
 *
 * Run with:
 * OMP_NUM_THREADS=4 ./omp-trap0 [a b n]
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/*
 * Function to be integrated
 */
double f( double x )
{
    return 4.0/(1.0 + x*x);
}

int min(int a, int b)
{
    return (a < b ? a : b);
}

/*
 * Compute the area of function f(x) for x=[a, b] using the trapezoid
 * rule. The integration interval [a,b] is partitioned into n
 * subintervals of equal size.
 */
double trap( double a, double b, int n )
{
    const int my_rank = omp_get_thread_num();
    const int thread_count = omp_get_num_threads();
    const double h = (b-a)/n;
    const int local_n_start = n * my_rank / thread_count;
    const int local_n_end = n * (my_rank+1) / thread_count;
    double x = a + local_n_start * h;
    double my_result = 0.0;
    int i;

    for ( i = local_n_start; i<local_n_end; i++ ) {
	my_result += h*(f(x) + f(x+h))/2.0;
	x += h;
    }
    return my_result;
}

int main( int argc, char* argv[] )
{
    const int thread_count = omp_get_max_threads();
    int i, n = 1000000;
    double a = 0.0, b = 1.0, result = 0.0;
    double partial_result[thread_count];
    double tstart, tstop;

    if ( 4 == argc ) {
	a = atof(argv[1]);
	b = atof(argv[2]);
	n = atoi(argv[3]);
    }

    /* All variables declared before the "parallel" block are shared
       by default. All variables declared inside the block are private
       to each thread. */

    tstart = omp_get_wtime();

#pragma omp parallel num_threads(thread_count)
    {
        int thr = omp_get_thread_num();
	partial_result[thr] = trap(a, b, n);
    }
    /* Implicit barrier here */

    /* Only one thread (the master) computes the sum of partial results */
    result = 0.0;
    for (i=0; i<thread_count; i++) {
        result += partial_result[i];
    }

    tstop = omp_get_wtime();

    printf("Area: %f\n", result);
    printf("Elapsed time %f\n", tstop - tstart);
    return 0;
}
