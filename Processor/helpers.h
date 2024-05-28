#include <math.h>
#include <time.h>

double calc_time_taken(struct timespec *start_time, struct timespec *end_time) {
	
	struct timespec processing_time;
	
	processing_time.tv_sec = (*end_time).tv_sec - (*start_time).tv_sec;
	processing_time.tv_nsec = (*end_time).tv_nsec - (*start_time).tv_nsec;
	
	if (processing_time.tv_nsec < 0) { //since somehow it sometimes is in the negatives I just remove 1 from seconds and add 1 billion to nanoseconds
		
		processing_time.tv_sec--;
		processing_time.tv_nsec += 1000000000;
	} 
	return processing_time.tv_sec + processing_time.tv_nsec / (double)1000000000; //don't want to see big number but small number :)
}

/*
ORIGINAL CODE
double approx_by_rect(double a, double b, int n, double(*f)(double)) {
	
	double delta_x = (b - a) / n;
	double approx = 0;
	
	for (unsigned int i = 1; i <= n; i++)
		approx += f((a + i * delta_x) - delta_x / 2); //a + delta_x * i = x_i	 and 	x_i - delta_x / 2 = c_i

	return approx * delta_x;
}
*/

//assembly instructions in the loop went from 12 -> 7 (almost doubled efficiency)
double approx_by_rect(double a, double b, unsigned long long int n, double(*f)(double)) {
	
	double delta_x = (b - a) / n;
	double approx = 0;
    double delta_x_mezzi = delta_x / 2;
    double c_i = a + delta_x_mezzi; 
	/*
	x_i = a + delta_x * i;
	c_i = 1/2 * (x_i + x_(i-1))

	c_i = 1/2 * (a + i * delta_x + a + (i-1) * delta_x)
	c_i = 1/2 * (2a + delta_x * (i + i - 1))
	c_i = 1/2 * (2a + delta_x * (2i - 1))
	c_i = (a + delta_x * i + delta_x * (-1/2))
	c_i = (a + delta_x * i - delta_x / 2)
	c_i = x_i - delta_x / 2

	since the loop starts from 1 (x_i would be a + delta_x) and I need to subtract delta_x / 2 
	I just add delta_x / 2
	*/
	
	for (unsigned long long int i = 1; i <= n; i++) {
		approx += f(c_i);
        c_i += delta_x;
    }

	return approx * delta_x;
}

//1 operation per loop saved (8 -> 7)
double approx_by_trap(double a, double b, unsigned long long int n, double(*f)(double)) {
	
	double delta_x = (b - a) / n;
	double approx = f(a) / 2; //i = 0 assigned here
    double x_i = a + delta_x * 2;
	
	for (unsigned long long int i = 2; i < n; i++)  {
		approx += f(x_i);
        x_i += delta_x;
    }

	return (approx + f(b) / 2) * delta_x; //i = n assigned here
}

//13 operations per loop -> 16 operations per loop but half the loops (so 8 basically)
double approx_by_para(double a, double b, unsigned long long int n, double(*f)(double)) {
	
	double delta_x = (b - a) / n;
	double approx =  f(a) / 2; //the first is assigned here as it is one of the only 2 that have 1 as coefficient
    double x_i = a + delta_x + delta_x; //one less line than 2 * delta_x
	
	for (unsigned long long int i = 2; i < n; i+=2) {
        
		approx += f(x_i) * 2; //could be optimized by re-getting the value in assembly (-1 instruction), but idk how to do it
        x_i += delta_x;
        approx += f(x_i);
        x_i += delta_x;
    }
	
	return (approx * 2 + f(b)) * delta_x / 3; //same reasoning for the last one
}
