#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <cstring>

using namespace std;

int main() {
    sem_t sem;

    srand(time(0));
    int value = 1;
    sem_init(&sem, 1, 1);

    int fd = shm_open("Shared Memory", O_CREAT | O_RDWR, 0666);
    char* mem = (char*)mmap(0, 1024, PROT_WRITE, MAP_SHARED, fd, 0);
    ftruncate(fd, 1024);

    pid_t processID = fork();

    while (value <= 1000) {
        int randomValue = rand() % 2;
        if (randomValue + 1 == 2) {
            sem_wait(&sem);
            sprintf(mem, "%d", value);
            !processID ? cout << "Parent process is printing: " << mem << endl : cout;
            value++;
            sem_post(&sem);
        }
        if (processID != 0) {
            wait(NULL);
        }
        else if (processID == 0) {
            randomValue = rand() % 2;
            if (randomValue + 1 == 2) {
                sem_wait(&sem);
                sprintf(mem, "%d", value);
                cout << "Child process is printing: " << mem << endl;
                value++;
                sem_post(&sem);
            }
        }
    }

    sem_destroy(&sem);
    munmap(mem, 1024);
    close(fd);
    shm_unlink("Shared Memory");

    return 0;
}

