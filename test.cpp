#include <cstdio>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <semaphore.h>
#include <unistd.h>
#include <mutex>
using namespace std;

/* -- The Dining Savages Problem --
* We have 10 savages (threads).
* We have a pot with M servings
* The savages eat from the pot whenever they want to
* If the pot is not empty, a savage can eat
* If the pot is empty, the savage wakes up the cook and waits while they refill the pot
*/

/* -- SETUP VARIABLES --
* M : this is the numebr of servings in the pot (i.e. the number of sephamores)
* pot : a semaphore initialized to M servings. The pot is either full (value of M), empty (value of 0), or partially full (value > 0 and < M)
* sleeping : a semaphore whose value is 0 when the cook is sleeping, and 1 when the cook is awake
* fullpot : a semaphore to notify the savages when the pot is full
* savages : an array of 10 threads / "savages".
* cook : a single thread / "cook"
* multex : multex : our mutex lock for atomic actions (i.e. refill pot as atomic action)
* savage_id : keep track of the number of savages so we can assign each one an id
* savage_id : keep track of the number of cooks so we can assign each one an id (there is only 1 cook)
* servings : keep track of the number of servings in the pot at any given time
*/
int M;
sem_t pot;  
sem_t sleeping;
sem_t fullpot;
thread savages[10];  
thread cook;
mutex multex;  
int savage_id = 1;
int cook_id = 1;
int servings;

/* -- COOK / REFILL POT FUNCTION --
* (0) Set id for cook thread, then increment the id counter for the next cook thread (if there were to be one)
* (1) Allow the cook to wait/sleep until they are woken up by a savage
* (2) Reset the pot to hold M servings
* (3) Set the servings count to M
* (4) Notify the savage that the pot is full
* NOTE : we must use a mutex lock when we reset the value of the global variable "servings"
*/
void working(){
    int tid;
    multex.lock();
    tid = cook_id;
    cook_id++;
    multex.unlock();
    printf("(Starting Thread) Cook %d is sleeping.\n",tid);
    
    while (true){
        sem_wait(&sleeping);
        printf("The cook is awake!\n");
        //sem_init(&pot, 0, M);
        for (int i = 0; i < 5; i++){
            sem_post(&pot);
        }
        multex.lock(); 
        servings = M;
        multex.unlock();
        printf("The cook has refilled the pot.\n\n");
        printf("The pot is full! The cook is going back to sleep.\n");
        sem_post(&fullpot);
    }
}

/* -- EAT / GET SERVING FUNCTION --
* (1) If the pot is not empty...
* (2) Allow savage to eat a serving (decrement the pot/servings by 1)
* (3) Decrement the serving count by 1
* (4) If the pot is empty
* (5) Wake up the cook (unlock sleeping semaphore to make the cook work)
* (6) Wait until the cook has refilled the pot and all the savage to eat a serving
* (6) Decrement the number of servings in the pot
* NOTE: we must use a mutex lock when checking or decrementing the value of the global variable "servings"
*/
void eat(int tid){
    sem_wait(&pot);
    /* -- if pot is not empty -- */
    multex.lock(); 
    if (servings > 0){
        multex.unlock();
        //sem_wait(&pot);
        printf("+++ Savage %d just ate serving %d.\n",tid, servings);
        multex.lock();
        servings--;
        multex.unlock();
    }
    else {
        multex.unlock();
        printf("!!! SAVAGE %d IS WAKING UP THE COOK...\n",tid);
        /* -- wakeup the cook -- */
        sem_post(&sleeping);
        /* -- wait until the cook is finished -- */
        sem_wait(&fullpot);
        /* -- give calling thread / savage a chance to eat first -- */
        //sem_wait(&pot);
        printf("+++ Savage %d just ate serving %d.\n",tid, servings);
        multex.lock();
        servings--;
        multex.unlock();
    }
}


/* -- SAVAGE FUNCTION --
* (0) Set id for savage thread, then increment the id counter for the next savage thread
* (1) Get a random amount of time between 0 and 30 seconds that the savage thread will "not eat"/sleep for
* (2) Savage thread sleeps/"doesnt eat" for the random amount of time 
* (3) Call eat, so savage thread can "eat a serving". 
*/
void savage(){
    int tid;
    multex.lock();
    tid = savage_id;
    savage_id++;
    multex.unlock();
    printf("(Starting Thread) Savage %d is getting hungry.\n",tid);
    
    while(true){
        int not_hungry_time = (int) rand() % 31;  
        sleep(not_hungry_time);
        printf("!!! SAVAGE %d WANTS TO EAT...\n",tid);
        eat(tid);
    }

}


int main(int argc, char* argv[]) {
    /* -- get the number of servings in the pot (default is 5) -- */
    if (argc == 2) {
        M = atoi(argv[1]);
    } 
    else{
        M = 5;
    }
    servings = M;
    printf("\nThere are %d servings in a single pot.\n\n", M);
    
    /* -- Initialize pot to hold 5 servings (none have been "eaten" / used yet) -- */
    sem_init(&pot, 0, M);
    /* -- Initialize full pot to 1, i.e. it is full -- */
    sem_init(&fullpot, 0, 0); 
     /* -- Initialize sleeping to 0, i.e. it the cook is sleeping -- */
    sem_init(&sleeping, 0, 0); 
    
    /* -- Let the savages eat... (create the savage threads) -- */
    for (int i = 0; i < 10; i++){
        savages[i] = thread(savage);
    }
    
    printf("\n-----> Savage Threads Created <-----\n\n");
    
    /* -- Let the cook sleep... (create the cook thread) -- */
    cook = thread(working);
    
    printf("\n-----> Cook Thread Created <-----\n\n");


    /* -- wait for savages and the cook to finish -- */
    for (int i = 0; i < 10; i++){
        savages[i].join();
    }
    cook.join();

    return 0;
}



