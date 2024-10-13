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

thread reindeer[9]; // 9 reindeers
int reindeer_id = 1;
int warming_up_count = 0; // count of reindeers warming up in the hut
thread santa; // 1 santa thread
sem_t sleeping; // santa has a semaphore to keep track of when he is sleeping
sem_t vacation[9]; // each reindeer semaphore to keep track of when they are on vacation
sem_t multex;
sem_t christmas; // semaphore to keep track of when christmas is
mutex m;  

void Reindeer(){
    int tid; 
    m.lock();
    tid = reindeer_id;
    reindeer_id++;
    m.unlock();

    while (true){
        int vacation_time = (int) rand() % 31;
        printf("(Update) Reindeer %d is on vacation for %d seconds\n",tid,vacation_time);
        sleep(vacation_time);
        m.lock();
        if (warming_up_count < 9){
            warming_up_count++;
        }
        if (warming_up_count == 9){
            printf("(Ready) Reindeer %d arrived to the hut. %d reindeers are warming up...\n",tid, warming_up_count);
        }
        m.unlock();

        sem_wait(&christmas);
    }
}

// void Santa(){
// }

int main(int argc, char* argv[]) {

    
    // initialize so that all reindeer are on vacation
    for (int i = 0; i < 9; i++){
        sem_init(&vacation[i], 0, 0);
    }

    // initialize so that santa is sleeping
    sem_init(&sleeping, 0, 0);

    // initialize so that chistmas has not yet started
    sem_init(&christmas, 0, 0);

    // start threads
    // santa = thread(Santa);
    for (int i = 0; i < 9; i++){
        reindeer[i] = thread(Reindeer);
    }

    return 0;
}
