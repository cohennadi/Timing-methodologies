#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define ERROR -1

static inline unsigned long long getcycles(void)
{
    unsigned long low, high;
    asm volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ((low) | (high) << 32);
}

long double get_cpu_speed_in_GHz()
{
    FILE *cpuinfo_file = NULL;
    ssize_t read_characters = 0;
    char *line = NULL;
    double cpu_speed = 0;
    int sscanf_result = 0;
    size_t length = 0;
    const int NUM_OF_VAR_TO_FILL = 1;

    cpuinfo_file = fopen("/proc/cpuinfo", "r");
    if (!cpuinfo_file)
    {
        printf("failed to open /proc/cpuinfo\n");

        return ERROR;
    }

    while (read_characters = getline(&line, &length , cpuinfo_file) > 0)
    {
        if (!strstr(line, "cpu MHz\t"))
        {
            continue;
        }

        sscanf_result = sscanf(line, "cpu MHz\t: %lf", &cpu_speed);
        if (sscanf_result == NUM_OF_VAR_TO_FILL) {
            cpu_speed = cpu_speed / 1000; // 1GHz = 1000MHz

            break;
        }
    }

    if (read_characters == ERROR)
    {
        printf("failed to getline\n");
        fclose(cpuinfo_file);

        return ERROR;
    }

    if (line)
    {
        free(line);
    }

    fclose(cpuinfo_file);

    return cpu_speed;
}

unsigned long long gethosttime(unsigned long long cycles)
{
    double cpu_speed = get_cpu_speed_in_GHz();
    if (cpu_speed == ERROR)
    {
        printf("error getting cpu speed in GHz\n");
        
        return 0;   
    }

    return cycles / cpu_speed;

}

void measure_getcycles()
{
    unsigned long long start = 0;
    unsigned long long end = 0;
    unsigned long long measurement_result = 0;

    start = getcycles();
    getcycles();
    end = getcycles();

    measurement_result = gethosttime(end - start);

    printf("The result measurement of getcycles is %llu\n", measurement_result);
}

void measure_gettimeofday()
{
    unsigned long long start = 0;
    unsigned long long end = 0;
    unsigned long long measurement_result = 0;
    struct timeval tv = {0};

    start = getcycles();
    gettimeofday(&tv, NULL);
    end = getcycles();

    measurement_result = gethosttime(end - start);

    printf("The result measurement of gettimeofday is %llu\n", measurement_result);
}

long double calculate_standard_deviation(long long *measurements, int size, long double mean)
{
    int i = 0;
    long double standard_deviation = 0;
    long double sum = 0;

    for (i = 0; i < size; ++i) {
        sum += pow(measurements[i] - mean, 2);
    }

    standard_deviation = sqrt((1.0 / size) * sum);

    return standard_deviation;
}

void measure_innerloop_by_gethosttime()
{
    const int OUTER_LOOP_SIZE = 1000;
    unsigned long long start = 0;
    unsigned long long end = 0;
    long long measurements[OUTER_LOOP_SIZE];
    int i = 0;
    int j = 0;
    int k = 0;    
    long double mean = 0;

    memset(measurements, 0, sizeof(long long) * OUTER_LOOP_SIZE);

    for (i = 0; i < OUTER_LOOP_SIZE; i++) {
        start = getcycles();

        for (j = 0; j < 100; j++) {  /* inner loop starts here */
            k = i + j;  
        }                          /* inner loop ends here */

        end = getcycles();

        measurements[i] = gethosttime(end - start);
        mean += measurements[i];
    }

    mean /= OUTER_LOOP_SIZE;

    printf("The mean of the measurement of inner loop by gethosttime is %Le\n", mean);
    printf("The standard deviation of the measurement of inner loop by gethosttime is %Le\n", 
            calculate_standard_deviation(measurements, OUTER_LOOP_SIZE, mean));
}

long double diff_timeval(struct timeval* start, struct timeval* end)
{
    const long long SECOND_TO_MICROSECONDS = 1000000;
    const long long MICROSECONDS_TO_NANOSECONDS = 1000;

    return ((end->tv_sec - start->tv_sec) *  SECOND_TO_MICROSECONDS + (end->tv_usec - start->tv_usec)) * MICROSECONDS_TO_NANOSECONDS;
}

void measure_innerloop_by_gettimeofday()
{
    const int OUTER_LOOP_SIZE = 1000;
    struct timeval start = {0};
    struct timeval end = {0};
    long long measurements[OUTER_LOOP_SIZE];
    int i = 0;
    int j = 0;
    int k = 0;    
    long double mean = 0;
    int result = 0;

    memset(measurements, 0, sizeof(long long) * OUTER_LOOP_SIZE);

    for (i = 0; i < OUTER_LOOP_SIZE; i++) {
        gettimeofday(&start, NULL);

        for (j = 0; j < 100; j++) {  /* inner loop starts here */
            k = i + j;  
        }                          /* inner loop ends here */

        gettimeofday(&end, NULL);;

        measurements[i] = diff_timeval(&start, &end);
        mean += measurements[i];
    }

    mean /= OUTER_LOOP_SIZE;
    
    printf("The mean of the measurement of inner loop by gettimeofday is %Le\n", mean);
    printf("The standard deviation of the measurement of inner loop by gettimeofday is %Le\n", 
            calculate_standard_deviation(measurements, OUTER_LOOP_SIZE, mean));
}

int main() 
{   
    measure_getcycles();
    printf("-------------------------------------------\n");
    measure_gettimeofday();
    printf("-------------------------------------------\n");
    measure_innerloop_by_gethosttime();
    printf("-------------------------------------------\n");
    measure_innerloop_by_gettimeofday();

    return 0;
}