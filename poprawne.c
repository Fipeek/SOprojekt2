#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>



pthread_mutex_t readerMutex,library,writerQueueMutex ,readerQueueMutex ,writerCond,readerCond;			
int W = 0, R = 0,readerInLibrary = 0,writerInLibrary = 0,end = 0,queueW=0,queueR=0;					


void *reader(void *arg) {
    int id = *(int *) arg; // przekonwertowanie i zapisanie id(void*) watku na i(int)
    free(arg); //zwolnienie miejsca parametru arg zalokowaniego przez main
    while (end == 0) {
        pthread_mutex_lock(&readerQueueMutex);
	queueR++;	//zwiekszenie koleji readerow
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, readerInLibrary, writerInLibrary, id);
        pthread_mutex_unlock(&readerQueueMutex);
        pthread_mutex_lock(&library); // oczekiwanie na brak writerow w bibliotece
	while(writerInLibrary>0&&end==0)
		pthread_cond_wait(&writerCond,&library);  // oczekiwanie na zmiane ilosci writerow w kolejce do biblioteki
	pthread_mutex_unlock(&library);
        pthread_mutex_lock(&readerMutex);
	readerInLibrary++; //zwiekszenie ilosci readerow w bibliotece
        pthread_mutex_unlock(&readerMutex);
	pthread_mutex_lock(&readerQueueMutex);
	queueR--; //zmniejszenie kolejki readerow
        pthread_mutex_unlock(&readerQueueMutex);
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, readerInLibrary, writerInLibrary, id);
      
	pthread_mutex_lock(&readerMutex);
	readerInLibrary--;	//zmniejszenie ilosci readerow w biliotece
        pthread_mutex_unlock(&readerMutex);
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, readerInLibrary, writerInLibrary, id);
        pthread_cond_signal(&writerCond); //wyslanie sygnalu o wyjsciu z biblioteki
        sleep(1.5); //odczekiwanie na ponowne wejsce do kolejki
    }
}


void *writer(void *arg) {
    int id = *(int *) arg;
    free(arg);
    while (end == 0) {
        pthread_mutex_lock(&writerQueueMutex);
	queueW++;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, readerInLibrary, writerInLibrary, id);
        pthread_mutex_unlock(&writerQueueMutex);
        pthread_mutex_lock(&library);
	while(readerInLibrary > 0  &&  end == 0)
		pthread_cond_wait(&writerCond,&library); //oczekiwanie na sygnal od zmianie iloci readerow w biliotece
	writerInLibrary++;
	pthread_mutex_lock(&writerQueueMutex);
	queueW--;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, readerInLibrary, writerInLibrary, id);
        pthread_mutex_unlock(&writerQueueMutex);
       
		writerInLibrary--;
	printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, readerInLibrary, writerInLibrary, id);
        pthread_mutex_unlock(&library);
	pthread_cond_broadcast(&readerCond); // wyslanie sygnalu do wszystkich oczekujacych readerow po zmianie ilosci writerow w bibliotece
        sleep(1.5);
    }
}



int main(int argc, char *argv[]) {
    int i = 0;
    W = atoi(argv[1]);	
    R = atoi(argv[2]);	
    check = (int *) malloc(sizeof(int) * (W + R));
    pthread_t *tab = (pthread_t *) malloc(sizeof(pthread_t) * (W + R));		//tablica wszystkich watkow
    for (i = 0; i < W + R; i++) { // zainicjowanie tablic
        check[i] = 0;  //inicjalizacja tablicy sprawdzajacej ilosc wejsc
        tab[i] = 0;		//inicjalizajca tablicy watkow
    }
    pthread_cond_init(&writerCond, NULL); // inicjalizacje mutexow i zmiennych warunkowych
    pthread_cond_init(&writerCond, NULL);
    pthread_mutex_init(&readerMutex, NULL);
    pthread_mutex_init(&library, NULL);
    pthread_mutex_init(&writerQueueMutex, NULL);
    pthread_mutex_init(&readerQueueMutex, NULL);

    for (i = 0; i < W; i++) { //tworzenie watkow writerow
        int *id = (int *) malloc(sizeof(int)); // rezerwacja pamieci na id watku
        *id = i; // przypisanie id dla wadku
        if (pthread_create(&tab[i], NULL, &writer, id) != 0) {
            perror("Failed to create thread");
        }
    }
    for (i = W; i < W + R; i++) { //tworzenie watkow readerow
        int *id = (int *) malloc(sizeof(int));
        *id = i;
        if (pthread_create(&tab[i], NULL, &reader, id) != 0) {
            perror("Failed to create thread");
        }
    }

    for (i = 0; i < W + R; i++) { //rozpoczecie watkow
        if (pthread_join(tab[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }


    pthread_cond_destroy(&writerCond); // zniszczenie zmiennych warunkowych i mutexow
    pthread_cond_destroy(&readerCond);
    pthread_mutex_destroy(&library);
    pthread_mutex_destroy(&readerMutex);
    pthread_mutex_destroy(&writerQueueMutex);
    pthread_mutex_destroy(&readerQueueMutex);
    free(tab);			//wyczyszczenie tablicy watkow
    return 0;
}
