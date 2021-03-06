#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
#define SANTAHELP "xvalen27.sem.santahelp"
#define ELFSEM "xvalen27.sem.elf"
#define ELFWAIT "xvalen27.sem.elfWait"
#define RDSEM "xvalen27.sem.rd"
#define RD2SEM "xvalen27.sem.rd2"
#define RD3SEM "xvalen27.sem.rd3"

sem_t *santaSem = NULL;
sem_t *santa2Sem = NULL;
sem_t *santaHelp = NULL;
sem_t *elfSem = NULL;
sem_t *elfWait = NULL;
sem_t *rdSem = NULL;
sem_t *rdSem2 = NULL;
sem_t *rdSem3 = NULL;
sem_t *mutex = NULL;

int *sharedCnt = 0;
int *elfCnt = 0;
int *rdCnt = 0;
FILE *file;
bool christmas = 0;

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
    if ((santaHelp = sem_open(SANTAHELP, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((elfSem = sem_open(ELFSEM, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((elfWait = sem_open(ELFWAIT, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((rdSem = sem_open(RDSEM, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((rdSem2 = sem_open(RD2SEM, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((rdSem3 = sem_open(RD3SEM, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        return -1;
    }
    if ((mutex = sem_open("xvalen27.sem.mutex", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
        return -1;
    }
    return 0;
}

void closeSems()
{
    sem_close(santaSem);
    sem_close(santa2Sem);
    sem_close(santaHelp);
    sem_close(elfSem);
    sem_close(elfWait);
    sem_close(rdSem);
    sem_close(rdSem2);
    sem_close(rdSem3);
    sem_close(mutex);

    sem_unlink(SANTASEM);
    sem_unlink(SANTA2SEM);
    sem_unlink(SANTAHELP);
    sem_unlink(ELFSEM);
    sem_unlink(ELFWAIT);
    sem_unlink(RDSEM);
    sem_unlink(RD2SEM);
    sem_unlink(RD3SEM);
    sem_unlink("xvalen27.sem.mutex");
}

void cleanup()
{
    UNMAPIT(sharedCnt)
    UNMAPIT(elfCnt)
    UNMAPIT(rdCnt)

    closeSems();
}

void santa(int NR, int NE)
{
    sem_wait(mutex);
    fprintf(file, "%d: Santa: going to sleep\n", ++*sharedCnt);
    fflush(file);
    sem_post(mutex);

    sem_wait(santaSem);

    sem_wait(mutex);
    fprintf(file, "%d: Santa: helping elves\n", ++*sharedCnt);
    fflush(file);
    sem_post(mutex);

    sem_post(elfWait);
    sem_post(elfWait);
    sem_post(elfWait);

    sem_wait(santaHelp);
    sem_wait(santaHelp);
    sem_wait(santaHelp);

    sem_wait(mutex);
    fprintf(file, "%d: Santa: going to sleep\n", ++*sharedCnt);
    fflush(file);
    sem_post(mutex);
    *elfCnt = 0;

    sem_wait(santa2Sem);
    christmas = 1;
    sem_wait(mutex);
    fprintf(file, "%d: Santa: closing workshop\n", ++*sharedCnt);
    fflush(file);
    sem_post(mutex);

    for(int j = 1; j <= NE; j++)
    {
        sem_post(elfSem);
        sem_wait(mutex);
        fprintf(file, "%d: Elf %d: taking holidays\n", ++*sharedCnt, j);
        fflush(file);
        sem_post(mutex);
    }

    for (int j = 1; j <= NR; j++)
        sem_post(rdSem2);

    for (int j = 1; j <= NR; j++)
        sem_wait(rdSem3);

    sem_wait(mutex);
    fprintf(file, "%d: Santa: Christmas started\n", ++*sharedCnt);
    fflush(file);
    sem_post(mutex);

    exit(0);
}

void elf(int TE, int i)
{
    sem_wait(mutex);
    fprintf(file, "%d: Elf %d: started\n", ++*sharedCnt, i);
    fflush(file);
    sem_post(mutex);

    usleep((rand() % TE) * 1000);

    sem_wait(mutex);
    fprintf(file, "%d: Elf %d: need help\n", ++*sharedCnt, i);
    fflush(file);
    sem_post(mutex);

    *elfCnt = *elfCnt + 1;
    if (*elfCnt < 3)
    {
        if (christmas == 1)
        {
            sem_wait(mutex);
            fprintf(file, "%d: Elf %d: taking holidays\n", ++*sharedCnt, i);
            fflush(file);
            sem_post(mutex);
            exit(0);
        }
        else
            sem_wait(elfSem);
    }

    sem_post(santaSem);     //santa pomaha elfom
    if (*elfCnt == 3)
    {
        for (int j = 1; j <= 3; j++)
            sem_post(elfSem);
    }
    sem_wait(elfWait);

    sem_wait(mutex);
    fprintf(file, "%d: Elf %d: get help\n", ++*sharedCnt, i);
    fflush(file);
    sem_post(mutex);

    sem_post(santaHelp);        //po pomoci vypise ze pomohol

    exit(0);
}

void RD(int NR, int TR, int i)
{
    sem_wait(mutex);
    fprintf(file, "%d: RD %d: rstarted\n", ++*sharedCnt, i);
    fflush(file);
    sem_post(mutex);

    usleep((rand()% TR + TR)*1000);

    sem_wait(mutex);
    fprintf(file, "%d: RD %d: return home\n", ++*sharedCnt, i);
    fflush(file);
    sem_post(mutex);

    *rdCnt = *rdCnt + 1;
    if(*rdCnt < NR)
    {
        sem_wait(rdSem);
    }
    sem_post(santa2Sem);

    for (int j = 1; j < NR; j++)
        sem_post(rdSem);

    sem_wait(rdSem2);

    sem_wait(mutex);
    fprintf(file, "%d: RD %d: get hitched\n", ++*sharedCnt, i);
    fflush(file);
    sem_post(mutex);

    sem_post(rdSem3);

    exit(0);
}

void createElfs(int NE, int TE)
{
    for(int i = 1; i <= NE; i++)
    {
        pid_t elfI = fork();
        if(elfI == 0)
        {
            elf(TE, i);
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
        santa(NR, NE);
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
    fclose(file);
    exit(0);
}
