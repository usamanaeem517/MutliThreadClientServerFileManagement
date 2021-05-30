#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <math.h>

//some constants defined to be used by the functions
#define BLOCK_SIZE 128
#define MAX_FILE_NAME 16
#define filemaxblocks (BLOCK_SIZE * 8 /2)
#define numberofblocks BLOCK_SIZE * 8


//struct to hold the details of the disk to be used by the FAT system to handle the data 
typedef struct disk_details disk_details; 
struct disk_details{
   int numberofcylinders;
   int numberofsectors;
   char * disk;
   int fd;
   size_t disksize; 
};

disk_details * diskdet; //disk struct pointer to be used by the FAT system
//struct to hold content needed by the file structure
typedef struct fileinfo fileinfo; 
struct fileinfo{
    char filename[MAX_FILE_NAME];
    int fileblocks[filemaxblocks];
    size_t filesize;
};

//struct to hold content needed by the directory structure
typedef struct directories directories;
struct directories{
     char dirname[MAX_FILE_NAME];
     struct fileinfo files[filemaxblocks];
     int numberoffiles;
};

//struct to hold content needed by the filesystem
typedef struct fatsystem fatsystem; 
struct fatsystem{
    struct directories dirs[filemaxblocks];  
    unsigned char bitmap[numberofblocks];
    int currdir;
    int numberofdirs;
};

//struct pointer to hold all the FAT system
fatsystem * fatsys;

//function to create file in the directory
int createfile(char * filename){
	int flag = 1;
        fatsys->dirs[fatsys->currdir].files[0].filename;
        for(int i = 0; i< fatsys->dirs[fatsys->currdir].numberoffiles; i++){
	   if(strcmp(fatsys->dirs[fatsys->currdir].files[i].filename, filename) == 0){
                flag = 0;
                return 1;
           }
	}
        if(flag == 1){
                strcpy(fatsys->dirs[fatsys->currdir].files[fatsys->dirs[fatsys->currdir].numberoffiles].filename, filename);
                fatsys->dirs[fatsys->currdir].files[fatsys->dirs[fatsys->currdir].numberoffiles].filesize = 0;
                fatsys->dirs[fatsys->currdir].files[fatsys->dirs[fatsys->currdir].numberoffiles].fileblocks[0] = -1;
                fatsys->dirs[fatsys->currdir].numberoffiles += 1;
                return 0;
        }
        return 2;
}
//to mark the block free after the deletion of the file
void makeblockfree(int blocknum){
        int byte = blocknum /8;
        int bit = blocknum%8;
        unsigned char m = ~(1 << bit);
        fatsys->bitmap[byte] &= m;
}

//function to delete the file from directory
int deletefile(char * filename){
	int flag = 1;
	for(int i = 0; i< fatsys->dirs[fatsys->currdir].numberoffiles; i++){
	   if(strcmp(fatsys->dirs[fatsys->currdir].files[i].filename, filename) == 0){
                fatsys->dirs[fatsys->currdir].files[i].filename[0] = '\0';
                size_t size = fatsys->dirs[fatsys->currdir].files[i].filesize;
                for(int j = 0; j < floor(size/BLOCK_SIZE); j++){
                   int blocknum = fatsys->dirs[fatsys->currdir].files->fileblocks[j];
                   makeblockfree(blocknum);
                }
                fatsys->dirs[fatsys->currdir].files[i].filesize = 0;
                flag = 0;
                fatsys->dirs[fatsys->currdir].numberoffiles -= 1;
                return 0;
           }
	}
        if(flag == 1){
                return 1;
        }
        return 2;
}

//function to list all the files in a the current working directory
void directorylisting(int booldir, char * dirdata){
        for(int i = 0; i< fatsys->dirs[fatsys->currdir].numberoffiles; i++){
           if(booldir == 0){
           int len = strlen(dirdata);
           strcpy(dirdata + len, fatsys->dirs[fatsys->currdir].files[i].filename);
	 }
         else if(booldir == 1){
           int len = strlen(dirdata);
           strcpy(dirdata + len, fatsys->dirs[fatsys->currdir].files[i].filename);
           len = strlen(dirdata);
           strcpy(dirdata + len, (char *)fatsys->dirs[fatsys->currdir].files->filesize);
         }
        }
}

//function to read content of a particular file and it returns data to be sent to the client
int readfile(char * filename, char * readdata){
        int blocktoread = 1;
        for(int i = 0; i< fatsys->dirs[fatsys->currdir].numberoffiles; i++){
          if(strcmp(fatsys->dirs[fatsys->currdir].files[i].filename, filename) == 0){
           for(int j = 0; j < blocktoread; j++){
                int blocknum = fatsys->dirs[fatsys->currdir].files[i].fileblocks[j];
                if (blocknum != -1){
                 size_t offset = (blocknum * BLOCK_SIZE) - BLOCK_SIZE;
                 int len = strlen(readdata);
                 memcpy(readdata + len, diskdet->disk + offset, BLOCK_SIZE);
                }
                else{
                  return 1;
                }
           }
           return 0;
         }
        }
        return 1;
}

//to get the free block available to store the data
int getfreeblocknum(){
        for(int i = 0; i < BLOCK_SIZE; i++){
           if(fatsys->bitmap[i] != 0xFF){
             for(int j = 0; j < 8; j++){
                int m = 1 << j;
                if(j & ~fatsys->bitmap[i]){
                    fatsys->bitmap[i] |= m;
                    return (i * 8) + j;    
                }
             }
           }
        }
}

//fucntion to write content of the file
int writefile(char * filename, char * writedata, size_t datasize){
        int blocksreq = 1;
        int flag = 1;
        size_t lenwritten = 0;
        for(int i = 0; i< fatsys->dirs[fatsys->currdir].numberoffiles; i++){
          if(strcmp(fatsys->dirs[fatsys->currdir].files[i].filename, filename) == 0){ //if file exists in the system
                for(int k = 0; k < blocksreq; k++){
                   if(flag == 1){ //if only one block is needed
                    int blocknum = getfreeblocknum();
                    fatsys->dirs[fatsys->currdir].files[i].fileblocks[k] = blocknum;
                    fatsys->dirs[fatsys->currdir].files[i].fileblocks[k + 1] = -1; 
                    fatsys->dirs[fatsys->currdir].files[i].filesize = datasize;
                    lenwritten = BLOCK_SIZE;
                    size_t offset = (blocknum * BLOCK_SIZE) - BLOCK_SIZE;
                    memcpy(diskdet->disk + offset, writedata, BLOCK_SIZE);
                   }
                   if(flag == 0){  //if more than one block is needed by the file
                    int blocknum = getfreeblocknum();
                    fatsys->dirs[fatsys->currdir].files[i].fileblocks[k] = blocknum;
                    fatsys->dirs[fatsys->currdir].files[i].fileblocks[k + 1] = -1;  
                    size_t offset = (blocknum * BLOCK_SIZE) - BLOCK_SIZE;
                    memcpy(diskdet->disk + offset, writedata + lenwritten, BLOCK_SIZE);
                    lenwritten += BLOCK_SIZE;
                   }
                   flag = 0;
                }
            return 0;
           }
         
        }
        return 1;
}


//function to make a directory in the file system
int mkdir_fun(char * dirname){  
        for(int i = 0; i < fatsys->numberofdirs; i++){
           if(strcmp(fatsys->dirs[i].dirname, dirname) == 0){
                   return 1;
           }
        }
        strcpy(fatsys->dirs[fatsys->numberofdirs].dirname, dirname);
        fatsys->dirs[fatsys->numberofdirs].numberoffiles = 0;
        fatsys->numberofdirs += 1;
        return 0;
}

//function to get the path name of the current working directory
int pwd_fun(char * workdir){
    strcpy(workdir, fatsys->dirs[fatsys->currdir].dirname);
    return 0;
}

int cd_fun(char * dirname){
     for(int i = 0; i < fatsys->numberofdirs; i++){
        if(strcmp(fatsys->dirs[i].dirname, dirname) == 0){
           fatsys->currdir = i;
           return 0;
        }
     }   
     return 1;
}
//function to remove directory from the FAT systems and it also delete all the files in that directory from the disk
int rmdir_fun(char * dirname){
    for(int i = 0; i < fatsys->numberofdirs; i++){
        if(strcmp(fatsys->dirs[i].dirname, dirname) == 0){
                int temp = fatsys->currdir;
                cd_fun(dirname);
           for(int j = 0; j < fatsys->dirs[i].numberoffiles; i++){
                deletefile(fatsys->dirs->files[j].filename);
           }
           fatsys->dirs[i].dirname[0] = '\0';
           fatsys->dirs[i].numberoffiles = 0;
           fatsys->numberofdirs -= 1;
           fatsys->currdir = temp;
           return 0;
        }
     }   
     return 1;
}

int count = 0;

//thread function to synchronize different clients, it accomodates 3 clients at a time and if 1 client disconnects then other
//4th client is allowed to join otherwise client gets server full error 
void* receive_Client(void * cli_sock){

 int client_sock = *(int *)cli_sock; 
 char * sign_out_msg = "DISCONNECT";  //if client sends DISCONNECT then client is disconnected from the server
 //Receive the message from the client
 int loop = 1;
  while(loop == 1){
        char server_message[2000], client_message[2000];
        memset(server_message,'\0',sizeof(server_message));
        memset(client_message,'\0',sizeof(client_message));     
        if (recv(client_sock, client_message, sizeof(client_message),0) < 0)
        {
                printf("Receive Failed. Error!!!!!\n");
                return -1;
        }
        //checking if client has sent a message to disconnect
        printf("Client Message: %s\n",client_message);
        if(strcmp(client_message,sign_out_msg) == 0){
        	count -= 1;
         	loop = 0;
        }
       
        //to check if client has requested to create a file in directory
        else if(client_message[0] == 'C'){
         char filename[MAX_FILE_NAME];
         strcpy(filename, client_message + 2);
       	 int ret = createfile(filename);
         if(ret == 0){
           server_message[0] = '0';
         }
         else if(ret == 1){
          server_message[0] = '1';
         }
        }
      //to check if client has requested to delete a file in a directory
        else if(client_message[0] == 'D'){
         char filename[MAX_FILE_NAME];
         strcpy(filename, client_message + 2);
         int ret = deletefile(filename);
         if(ret == 0){
           server_message[0] = '0';
         }
         else if(ret == 1){
          server_message[0] = '1';
         }
        }
	//to check if client has requested to list the files in a directory
        else if(client_message[0] == 'L'){
         int booldir = client_message[2] - '0';
         char * dirdata = (char *)malloc(MAX_FILE_NAME * 32);
         directorylisting(booldir, dirdata);
         strcpy(server_message, dirdata);
        }
	//to check if client has requested to read content of the file
        else if(client_message[0] == 'R'){
         char filename[MAX_FILE_NAME];
         strcpy(filename, client_message + 2);
         char * data = (char *)malloc(MAX_FILE_NAME * 2024);
         readfile(filename, data);
         strcpy(server_message, data);
        }
	//to check if client has requested to write data into the file
        else if(client_message[0] == 'W'){
         char filename[MAX_FILE_NAME];
         int j = 2, k = 0;
         while(client_message[j] != ' '){
           filename[k] = client_message[j];
           filename[k + 1] = '\0';
           k += 1;
           j += 1;
         }
         j+=1;
         while(client_message[j] != ' '){
           k += 1;
           j += 1;
         }
         j+=1;
         k = 0;
         char data[strlen(client_message + j) + 1];
         while(client_message[j] != '\0'){
           data[k] = client_message[j];
           data[k + 1] = '\0';
           k += 1;
           j += 1;
         }
         size_t size = strlen(data);
         int ret = writefile(filename, data, size);
         if(ret == 0){
            server_message[0] = '0';
         }
         else if(ret == 1){
           server_message[0] = '1';
         }
        }
	//to check if client has requested to format file system
        else if(client_message[0] == 'F'){
         char * msg = "System Formated!";
         strcpy(server_message, msg);
        }
	//to check if client has requested to make a directory in the FAT system
        else if(client_message[0] == 'm'){
         char dirname[MAX_FILE_NAME];
         strcpy(dirname, client_message + 6);
       	 int ret = mkdir_fun(dirname);
         if(ret == 0){
           server_message[0] = '0';
         }
         else if(ret == 1){
          server_message[0] = '1';
         }
        }
        //to check if client has requested to remove a directory from the FAT system
        else if(client_message[0] == 'r'){
         char dirname[MAX_FILE_NAME];
         strcpy(dirname, client_message + 6);
       	 int ret = rmdir_fun(dirname);
         if(ret == 0){
           server_message[0] = '0';
         }
         else if(ret == 1){
          server_message[0] = '1';
         }
        }
	//to check if client has requested to change current working directory in the FAT system
        else if(client_message[0] == 'c' && client_message[1] == 'd'){
         char dirname[MAX_FILE_NAME];
         strcpy(dirname, client_message + 3);
       	 int ret = cd_fun(dirname);
         if(ret == 0){
           server_message[0] = '0';
         }
         else if(ret == 1){
          server_message[0] = '1';
         }
        }
        //to check if client has requested to know the current working directory
        else if(client_message[0] == 'p'){
       	 char * workdir = (char *)malloc(MAX_FILE_NAME);
         pwd_fun(workdir);
         strcpy(server_message, workdir);
        }
        //if clients sends an invalid option or command
        else{
          char * msg = "Invalid Input, Please Enter Correct Option!!";
          strcpy(server_message, msg);
        }

        //Send the message back to client
        
        if (send(client_sock, server_message, strlen(server_message),0)<0)
       {
                printf("Send Failed. Error!!!!!\n");
                return -1;
       }
        	 
        memset(server_message,'\0',sizeof(server_message));
        memset(client_message,'\0',sizeof(client_message));
 }
 close(client_sock);
}

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

//function to initialize the disk to store data of the files of FAT system
//function uses mmap to map the file disk.txt to store the file system
disk_details * initdiskstorage(char * filename, int numberofcylinders, int numberofsectors){
	
	disk_details * dd = malloc(sizeof(disk_details));
	dd->disksize = numberofcylinders * numberofsectors * BLOCK_SIZE;
	dd->fd = open(filename, O_CREAT | O_RDWR, 0666);
	dd->numberofcylinders = numberofcylinders;
	dd->numberofsectors = numberofsectors;
	if(dd->fd < 0){
		printf("File Not Open\n");
	}
	ftruncate(dd->fd, dd->disksize);
	dd->disk = mmap(NULL, dd->disksize, PROT_READ | PROT_WRITE, MAP_SHARED, dd->fd, 0);

	if(dd->disk == MAP_FAILED){  //to handle any error given by mmap
  		handle_error("mmap");
        	return NULL;
  	}
  	memset(dd->disk, '\0', dd->disksize);
  	return dd;

}

int main(int argc, char *argv[])
{
	int numberofcylinders = atoi(argv[1]);
	int numberofsectors = atoi(argv[2]);
	char filename[sizeof(argv[3]) + 1];
	strcpy(filename, argv[3]); //getting filename to map disk on from the commandline argument entered by the user
	diskdet = initdiskstorage(filename, numberofcylinders, numberofsectors);  //iniatilizing the disk 
        fatsys = malloc(sizeof(fatsystem));  //initializing the file system
        int socket_desc, client_sock, client_size; 
        struct sockaddr_in server_addr, client_addr;         //SERVER ADDR will have all the server address
	pthread_t threads[3]; //threads to accomdate 3 clients at a time, otherwise for more than 3 there will be denial of service
        //Cleaning the Buffers
        
        // Set all bits of the padding field//
        
        //Creating Socket
        
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);
        
        if(socket_desc < 0)
        {
                printf("Could Not Create Socket. Error!!!!!\n");
                return -1;
        }
        
        printf("Socket Created\n");
        
        //Binding IP and Port to socket
        
        server_addr.sin_family = AF_INET;               /* Address family = Internet */
        server_addr.sin_port = htons(2003);               // Set port number, using htons function to use proper byte order */
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");    /* Set IP address to localhost */
		
		// BINDING FUNCTION
        
        if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0)    // Bind the address struct to the socket.  /
	                            	//bind() passes file descriptor, the address structure,and the length of the address structure
        {
                printf("Bind Failed. Error!!!!!\n");
                return -1;
        }        
        
        printf("Bind Done\n");
        int i = 0;
        //Put the socket into Listening State
       while(1){
        if(listen(socket_desc, 1) < 0)                               //This listen() call tells the socket to listen to the incoming connections.
     // The listen() function places all incoming connection into a "backlog queue" until accept() call accepts the connection.
        {
                printf("Listening Failed. Error!!!!!\n");
                return -1;
        }
        
        printf("Listening for Incoming Connections.....\n");
        
        //Accept the incoming Connections
        
        client_size = sizeof(client_addr);
		
        client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);          // heree particular client k liye new socket create kr rhaa ha
        
        if (client_sock < 0)
        {
                printf("Accept Failed. Error!!!!!!\n");
                return -1;
        }
        
   if(count == 3){
        char * msg = "Server Full";     //if the server is currently accomodating 3 clients then this message is sent as denial of service
     if (send(client_sock, msg, strlen(msg),0)<0)
        {
                printf("Send Failed. Error!!!!!\n");
                return -1;
        }
           
        }
     else
        {
        count++;
        printf("Client Connected with IP: %s and Port No: %i\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		       //inet_ntoa() function converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
	
	void * cli_sock = &client_sock;  //sending client socket to the thread function
        
        int ret1 = pthread_create(&threads[i],NULL,receive_Client,(void *)cli_sock);
        if(ret1!=0)
        {
                printf("Thread 1 Creation Failed\n");
        }
        i++;       
 }           
     }
        //Closing the Socket
        close(socket_desc);
        int n = 0;
        for (n = 0; n<3;n++){
 	       pthread_join(threads[n],NULL);
        }
        
        return 0;       
}
