#include <sg/math/random.h>
#include <sg/sys/time.h>
#include <stdlib.h>
#include <limits.h>
#ifndef WIN32
# include <fcntl.h>
# include <sys/types.h>
# include <unistd.h>
#endif


static void random_seed(void)
{
    size_t seed;
    size_t read_bytes = 0;

#ifndef WIN32
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd) {
        char* data = (char*)&seed;
        while (read_bytes < sizeof(seed)) {
            int real = read(fd, data + read_bytes, sizeof(seed) - read_bytes);
            read_bytes += real;
        }
        close(fd);
    }
#endif

    // init seed using clock if read failed?
    if (read_bytes != sizeof(seed)) {
        // get clock
        struct timespec ts = sg_unix_time_ns();
        seed = (size_t)((ts.tv_nsec >> 32) ^ ts.tv_nsec);
        // xor the stack address
        seed ^= (size_t)(&seed);
    }

    // set seed
    srand(seed);
}

long sg_random_l(void)
{
    random_seed();
    return rand();
}

float sg_random_f(void)
{
    random_seed();
    return rand() / (float)RAND_MAX;
}

long sg_random_range_l(long min, long max)
{
    if (min > max)
        return min;

    return (min + (long)((size_t)sg_random_l() % (max - min)));
}

float sg_random_range_f(float min, float max)
{
    if (min > max)
        return min;

    float factor = (float)sg_random_range_l(0, INT_MAX) / (float)INT_MAX;
    return min + (max - min) * factor;
}

