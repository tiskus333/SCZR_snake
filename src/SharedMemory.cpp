#include "SharedMemory.h"

void createSharedMemory(const char *path_name)
{
    // create shared memory structure
    int fd = shm_open(path_name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("main: Cannot create shared memory.");
        exit(EXIT_FAILURE);
    }

    // shrink creared shared memory to frameBuffer size
    if (ftruncate(fd, sizeof(sh_m)) == -1)
    {
        perror("main: ftruncate error.");
        exit(EXIT_FAILURE);
    }

    // map frame buffer onto shared memory
    sh_m *shmp = (sh_m *)mmap(NULL, sizeof(*shmp),
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED)
    {
        perror("main: Cannot map memory.");
        exit(EXIT_FAILURE);
    }

    // initialize semaphores as process-shared
    if (sem_init(&shmp->sem_read, 1, 0) == -1)
    {
        perror("main: Cannot init sem_read.");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&shmp->sem_write, 1, 1) == -1)
    {
        perror("main: Cannot init sem_write.");
        exit(EXIT_FAILURE);
    }
}

sh_m *openSharedMemory(const char *path_name)
{
    // open existing shared memory
    int fd = shm_open(path_name, O_RDWR, 0);
    if (fd == -1)
    {
        perror("Cannot open shared memory.");
        exit(EXIT_FAILURE);
    }

    sh_m *shmp = (sh_m *)mmap(NULL, sizeof(*shmp),
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED)
    {
        perror("Cannot map memory.");
        exit(EXIT_FAILURE);
    }
    return shmp;
}