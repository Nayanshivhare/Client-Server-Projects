#include "genericServer.h"

/************************************************************************
 * MAIN
 ************************************************************************/
int main() {
    int input;                    // buffer for user input
    int client_socket;                  // client side socket
    struct sockaddr_in client_address;  // client socket naming struct
    int c;

    // client name
    printf("3A + 1 client\n");

    // create an unnamed socket, and then name it
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    // create addr struct
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    client_address.sin_port = htons(PORT);

    // connect to server socket
    if (connect(client_socket, (struct sockaddr *)&client_address, sizeof(client_address)) == -1) {
        perror("Error connecting to server!\n");
        exit(EXIT_FAILURE);
    }
      // simple message
        printf("\nInteger input for 3A + 1 algorithm: ");

        // read an integer
        scanf("%d", &input);

        // transmit the nunmber to compute
        write(client_socket, &input, sizeof(int));

        // get the result
        read(client_socket, &c, sizeof(int));

        // print steps
        printf("\nThe number of steps are: %d\n", c);

    return EXIT_SUCCESS;
}

