#include <stdio.h>
#include <errno.h>   // for errno
#include <limits.h>  // for INT_MAX
#include <stdlib.h>  // for strtol
#include <time.h>
#include <math.h>

#define M_PI 3.14159265358979323846

double function(double x){
    return 10 * ( (x * x) - (10 * cos(2 * M_PI * x)));
}

double calculate_trap(double a, double b, long n, double h, double (*func_ptr)(double i)) {  
    double area_total = ((*func_ptr)(a) + (*func_ptr)(b)) / 2;
    
    for (long i = 1; i < n; ++i) {
        double x_i = a + i * h;
        area_total += (*func_ptr)(x_i);
    }
    return h * area_total;

}

double convert_str_double(char* str) {
    char *p;
    errno = 0;
    double conv = strtod(str, &p);

    if (errno != 0 || *p != '\0' || conv > INT_MAX) {
        printf("%s não é um número!\n", str);
        exit(-1);
    }
    return (double) conv;
}

long convert_str_long(char *str){
    char *p;
    errno = 0;
    long conv = strtol(str, &p, 10);

    if (errno != 0 || *p != '\0')
    {
        printf("%s não é um número!\n", str);
        exit(-1);
    }
    return (long)conv;
}

int main( int argc, char **argv ) {

    if (argc != 4) {
        printf("É necessário informar 3 argumentos na seguinte ordem:\n");
        printf("1º Inicio do intervalo\n2º Fim do intervalo\n3º Número de trapézios a serem usados\n");
        return -1;
    }

    double (*func_ptr)(double i);
    func_ptr = function;

    double a = convert_str_double(argv[1]);
    double b = convert_str_double(argv[2]); 
    long n = convert_str_long(argv[3]);
    double h = (b - a) / n;
    
    clock_t t = clock(); 

    double local_integral = calculate_trap(a, b, n, h, func_ptr);
    
    t = clock() - t; 

    printf("{\"Integral\": %.20lf, \"time\": %.10lf}\n", local_integral, ((double)t) / CLOCKS_PER_SEC);


    return 0;
}  /* main */