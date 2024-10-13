#include <cstdio>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <semaphore.h>
#include <unistd.h>
#include <mutex>
using namespace std;

/* -- The Christmas Problem --
There are three kinds of processes: 1 Santa process, 9 reindeer processes, and several (>3) elf processes. The reindeer pull Santa's sleigh at the time of Christmas and take vacation in the South Pacific rest of the time. The elves make toys all the time. The Santa sleeps in his shop at the North Pole and can be woken up: 
a. by an elf when some elves are having difficulty making toys, or  
b. by a reindeer when all 9 reindeer are back from their vacation and are ready to go around the world to celebrate Christmas.  
To allow Santa to get some sleep, the elves can only wake him when three of them have problems. When three elves are having their problems solved, any other elves wishing to visit Santa must wait for those elves to return. 
As reindeer return from their vacation, they wait in a warming hut until the last reindeer arrives, who wakes up Santa. It is assumed that the reindeer don't want to leave the tropics, and therefore they stay there until the last possible moment. 
If Santa wakes up to find three elves waiting at his shop's door, along with the last reindeer having come back from the tropics, Santa has decided that the elves can wait until after Christmas, because it is more important to have his sleigh ready.  
You have to write three procedures: Santa(), elf(), and reindeer(). Santa executes Santa(), elves execute elf(), and reindeer execute reindeer(). Use semaphores for synchronization. 
*/

int n = 4;
thread elves[n]; // >3 elves
int elf_id = 1;
int problem_count = 0; // keep track of the problems that elves have
thread reindeer[9]; // 9 reindeers
int reindeer_id = 1;
int warming_up_count = 0; // count of reindeers warming up in the hut
thread santa; // 1 santa thread
sem_t sleeping; // santa has a semaphore to keep track of when he is sleeping
sem_t vacation[9]; // each reindeer semaphore to keep track of when they are on vacation
sem_t multex;
sem_t christmas; // semaphore to keep track of when christmas is
mutex m;  


void Elf(){
    int tid; 
    sem_wait(&multex);
    tid = elf_id;
    elf_id++;
    sem_post(&multex);

    while (true){
        int work_time = (int) rand() % 31;
        printf("(Update) Elf %d is working for %d seconds\n",tid,work_time);
        sleep(work_time);
        printf("(Update) Elf %d has a problem\n",tid);
        sem_wait(&multex);
        problem_count++;
        if (problem_count == 3){
            printf("\n(Update) %d elves have a problem...\n", problem_count);
            printf("(Ready) Elf %d is waking up santa...\n", tid);
            problem_count = 0;
            // sem_post(&sleeping);
        }
        sem_post(&multex);
        
    }
}

void Reindeer(){
    int tid; 
    sem_wait(&multex);
    tid = reindeer_id;
    reindeer_id++;
    sem_post(&multex);

    while (true){
        int vacation_time = (int) rand() % 31;
        printf("(Update) Reindeer %d is on vacation for %d seconds\n",tid,vacation_time);
        sleep(vacation_time);
        sem_wait(&multex);
        if (warming_up_count < 9){  
            warming_up_count++;
        }
        if (warming_up_count == 9){
            printf("\n(Update) %d reindeers are back from vacation, warming up in the hut...\n", warming_up_count);
            printf("(Ready) Reindeer %d is waking up santa...\n", tid);
            warming_up_count = 0;
            sem_post(&sleeping);
        }
        sem_post(&multex);

        sem_wait(&christmas);
    }
}

void Santa(){
    while (true){
        printf("\n(Update) Santa is Sleeping...\n\n");
        sem_wait(&sleeping);
        printf("\n(Update) Santa is Awake...\n\n");
        int christmas_time = (int) rand() % 31;
        printf("\n(Update) It is christmas time, santa and the reindeer are going off to work for %d seconds...\n", christmas_time);
        sleep(christmas_time);
        printf("(Update) Christmas time is over, santa is going to sleep, and the reindeers are going on vacation...\n\n", christmas_time);
        for (int i = 0; i < 9; i++){
            sem_post(&christmas);
        }
    }
}

int main(int argc, char* argv[]) {

    sem_init(&multex, 0, 1);
    // initialize so that all reindeer are on vacation
    for (int i = 0; i < 9; i++){
        sem_init(&vacation[i], 0, 0);
    }

    // initialize so that santa is sleeping
    sem_init(&sleeping, 0, 0);

    // initialize so that chistmas has not yet started
    sem_init(&christmas, 0, 0);

    // start threads
    santa = thread(Santa);
    for (int i = 0; i < 9; i++){
        reindeer[i] = thread(Reindeer);
    }
    for (int i = 0; i < 9; i++){
        elves[i] = thread(Elf);
    }

   for (int i = 0; i < 9; i++){
        reindeer[i].join();
    }

    return 0;
}
