#include "client.h"
#include "threadpool.h"

/* ******************************************************* */
/* main()                                                  */
/* ******************************************************* */
struct sockaddr_in client_address;  // client socket naming struct
pthread_mutex_t lock;
int end = 0;

int main() {

    // task counter, also serves as argument to 3A+1 algorithm
    int task_counter;

    // create threadpool with THREADS_IN_POOL threads
    threadpool pool = threadpool_create();
    pthread_mutex_init(&lock, NULL);

    // create addr struct
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    client_address.sin_port = htons(PORT);


    for (task_counter=1; task_counter <= NUMBER_TASKS; task_counter++)
    {
        // in each loop, execute three_a_plus_one_wrapper in a thread from the pool
        threadpool_add_task(pool, task_copy_arguments, client_wrapper, (void*)&task_counter);
    }

    // lame way to wait for everybody to get done
    // in a network server, this is not needed as the main thread keeps accepting connections
    sleep(15);

    exit(EXIT_SUCCESS);
}


/* ******************************************************* */
/* three_a_plus_one_wrapper()                              */
/* ******************************************************* */
void client_wrapper(void *number_ptr)
{
    int number = *((int*)number_ptr);
    int input = 0;
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1)
    {
      perror("Error creating socket");
      exit(EXIT_FAILURE);
    }

    // connect to server socket
    if (connect(client_socket, (struct sockaddr *)&client_address, sizeof(client_address)) == -1) {
      perror("Error connecting to server!\n");
      exit(EXIT_FAILURE);
    }

    // write to server number to calcuate
    write(client_socket, &number, sizeof(int));


    // read the input from 3A + 1
    read(client_socket, &input, sizeof(int));

    sleep(.5);

    printf("Closing client_socket %d ----> %d: %d\n", client_socket, number, input);
    close(client_socket);
}

/* ******************************************************* */
/* prepare arguments for thread function                   */
/* ******************************************************* */
void *task_copy_arguments(void *args_in)
{
    void *args_out;

    args_out = malloc(sizeof(int));
    *((int*)args_out) = *((int*)args_in);

    return args_out;
}
