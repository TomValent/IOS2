#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#define MAPIT(pointer) {(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAPIT(pointer) {munmap(pointer, sizeof(pointer));}

#define SANTASEM "xvalen27.sem.santa"
#define SANTA2SEM "xvalen27.sem.santa2"
#define ELFSEM "xvalen27.sem.elf"
#define ELF2SEM "xvalen27.sem.elf2"
#define RDSEM "xvalen27.sem.rd"

sem_t *santaSem = NULL;
sem_t *santa2Sem = NULL;
sem_t *elfSem = NULL;
sem_t *elf2Sem = NULL;
sem_t *rdSem = NULL;
sem_t *mutex = NULL;

int *sharedCnt = 0;
int *elfCnt = 0;
int *rdCnt = 0;
FILE *file;

int convert(char *argument)
{
    char *string = NULL;
    int num = strtol(argument, &string, 10);
    if(string[0] != '\0')
    {
        fprintf(stderr, "Error: Argument was not a number.\n");
        return 1;
    }
    return num;
}

int init()
{
    srand(time(0));

    MAPIT(sharedCnt)
    MAPIT(elfCnt)
    MAPIT(rdCnt)

    file = fopen("proj2.out", "w");
    setbuf(file, NULL);
    if(file == NULL)
    {
        fprintf(stderr, "Error: Output file cannot be opened.\n");
        exit(-1);
    }
    if ((santaSem = sem_open(SANTASEM, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((santa2Sem = sem_open(SANTA2SEM, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((elfSem = sem_open(ELFSEM, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((elf2Sem = sem_open(ELF2SEM, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((rdSem = sem_open(RDSEM, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((mutex = sem_open("xvalen27.sem.mutex", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    return 0;
}

void closeSems()
{
    sem_close(santaSem);
    sem_close(santa2Sem);
    sem_close(elfSem);
    sem_close(elf2Sem);
    sem_close(rdSem);
    sem_close(mutex);

    sem_unlink(SANTASEM);
    sem_unlink(SANTA2SEM);
    sem_unlink(ELFSEM);
    sem_unlink(ELF2SEM);
    sem_unlink(RDSEM);
    sem_unlink("xvalen27.sem.mutex");
}

void cleanup()
{
    UNMAPIT(sharedCnt)
    UNMAPIT(elfCnt)
    UNMAPIT(rdCnt)

    closeSems();
}

void santa()
{
    fprintf(file, "%d: Santa: going to sleep\n", ++*sharedCnt);
    fflush(file);

    sem_wait(santaSem);

    fprintf(file, "%d: Santa: helping elves\n", ++*sharedCnt);
    fflush(file);
    fprintf(file, "%d: Santa: going to sleep\n",++*sharedCnt);
    fflush(file);
    *elfCnt = 0;

    sem_wait(santa2Sem);
    fprintf(file, "%d: Santa: closing workshop\n",++*sharedCnt);
    fflush(file);
    fprintf(file, "%d: Santa: Christmas started\n",++*sharedCnt);
    fflush(file);

    exit(0);
}

void elf(int NE, int TE, int i)
{
    fprintf(file, "%d: Elf %d: started\n", ++*sharedCnt, i);
    fflush(file);

    usleep((rand() % TE) * 1000);

    fprintf(file, "%d: Elf %d: need help\n", ++*sharedCnt, i);
    fflush(file);
    *elfCnt = *elfCnt + 1;
    if(*elfCnt < 3)
    {
        sem_wait(elfSem);
    }
    if(*elfCnt == 3)
    {
        for(int j = 1; j <NE; j++)
            sem_post(elfSem);
    }
    fprintf(file, "%d: Elf %d: get help\n", ++*sharedCnt, i);
    fflush(file);
    sem_post(santaSem);
    sem_wait(elf2Sem);
    fprintf(file, "%d: Elf %d: taking holidays\n", ++*sharedCnt, i);
    fflush(file);
    exit(0);
    
}

void RD(int NR, int TR, int i)
{
    fprintf(file, "%d: RD %d: rstarted\n", ++*sharedCnt, i);
    fflush(file);
    (*rdCnt)++;
    usleep((rand()% TR + TR)*1000);
    fprintf(file, "%d: RD %d: return home\n", ++*sharedCnt, i);
    fflush(file);

    if(*rdCnt < NR)
    {
        sem_wait(rdSem);
    }
    else if(*rdCnt == NR)
    {
        sem_post(santa2Sem);
        for (int j = 1; j < NR; j++)
            sem_post(rdSem);
        fprintf(file, "%d: RD %d: get hitched\n", ++*sharedCnt, i);
        fflush(file);

        sem_post(elf2Sem);
    }
    exit(0);
}

void createElfs(int NE, int TE)
{
    for(int i = 1; i <= NE; i++)
    {
        pid_t elfI = fork();
        if(elfI == 0)
        {
            elf(NE, TE, i);
        }
    }
    exit(0);
}

void createRaindeers(int NR, int TR)
{
    for(int i = 1; i <= NR; i++)
    {
        pid_t rdI = fork();
        if(rdI == 0)
        {
            RD(NR, TR, i);
        }
    }
    exit(0);
}

int main(int argc, char **argv)
{
    if(argc != 5)
    {
        fprintf(stderr, "Error: Wrong count of arguments.\n");
        return 1;
    }
    int NE = convert(argv[1]);
    int NR = convert(argv[2]);
    int TE = convert(argv[3]);
    int TR = convert(argv[4]);

    if(NE <= 0 || NE >= 1000 || NR <= 0 || NR >= 20 || TE < 0 || TE > 1000 || TR < 0 || TR > 1000)
    {
        fprintf(stderr, "Error: Wrong value of arguments\nExpected:\n0 < arg2 < 1000\n0 < arg3 < 20\n0 <= arg4 <= 1000\n0 <= arg5 <= 1000\n");
        return 1;
    }

    init();

    pid_t santaProcess = fork();
    if(santaProcess == 0)
    {
        santa();
    }

    pid_t elfProcess = fork();
    if(elfProcess == 0)
    {
        createElfs(NE, TE);
    }
    pid_t rdProcess = fork();
    if(rdProcess == 0)
    {
        createRaindeers(NR, TR);
    }

    cleanup();
    exit(0);
}
