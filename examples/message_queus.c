#include <sys/types.h> // For pid_t
#include <sys/ipc.h>   // For IPC_CREAT, etc.
#include <sys/msg.h>   // For message queue functions
#include <stdio.h>     // For printf, perror
#include <string.h>    // For strcpy
#include <stdlib.h>    // For exit
#include <unistd.h>    // For fork

struct msgbuf {
    long mtype;       // Message type (must be > 0)
    char mtext[70];   // Message data
};

int main() {
    // Generate a unique key for the message queue
    key_t key = ftok("keyfile", 65); // Generates a key using "keyfile" and 65.
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }
     // Create or get the message queue
    int msqid = msgget(key, IPC_CREAT | 0666); // Creates a message queue if it doesn't exist.
    if (msqid == -1) {
        perror("msgget failed");
        exit(1);
    }

    // Create a child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) { // Child: Sender
        struct msgbuf msg;
        msg.mtype = 1;
        strcpy(msg.mtext, "Hello from child!");
        // Send the message to the queue
        if (msgsnd(msqid, &msg, sizeof(msg.mtext), !IPC_NOWAIT) == -1) {
            perror("msgsnd failed");
            exit(1);
        }
        printf("Child sent: %s\n", msg.mtext);
    } else { // Parent: Receiver
        struct msgbuf msg;
        // Receive a message of type 1 from the queue
        if (msgrcv(msqid, &msg, sizeof(msg.mtext), 1, !IPC_NOWAIT) == -1) {
            perror("msgrcv failed");
            exit(1);
        }
        printf("Parent received: %s\n", msg.mtext);
        // Remove the message queue (cleanup)
        if (msgctl(msqid, IPC_RMID, NULL) == -1) {
            perror("Cleaning up (msgctl) failed");
            exit(1);
        }
    }
    return 0;
}