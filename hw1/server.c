#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<stdbool.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[512];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    // you don't need to change this.
    int id;
    int wait_for_write;  // used by handle_read to know if the header is read or not.
} request;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* accept_read_header = "ACCEPT_FROM_READ";
const char* accept_write_header = "ACCEPT_FROM_WRITE";

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance 
int lock_reg(int fd,int cmd,int type,off_t offset, int whence, off_t len);
typedef struct {
    int id;          //902001-902020
    int AZ;          
    int BNT;         
    int Moderna;     
}registerRecord;

int handle_read(request* reqP){
    int r;
    char buf[512];

    // Read in request from client
    r = read(reqP->conn_fd, buf, sizeof(buf));
    if (r < 0) return -1;
    if (r == 0) return 0;
    char* p1 = strstr(buf, "\015\012");
    int newline_len = 2;
    if (p1 == NULL) {
       p1 = strstr(buf, "\012");
        if (p1 == NULL) {
	    //fprintf(stderr,"this really should not happen ...");
	    //return -1;
            ERR_EXIT("this really should not happen...");
        }
    }
    size_t len = p1 - buf + 1;
    memmove(reqP->buf, buf, len);
    reqP->buf[len - 1] = '\0';
    reqP->buf_len = len-1;
    return 1;
} 
int lock_reg(int fd,int cmd,int type,off_t offset, int whence, off_t len){
    struct flock lock;
    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;
    return(fcntl(fd,cmd,&lock));
}
int main(int argc, char** argv) {

    // Parse args.
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(1);
    }

    struct sockaddr_in cliaddr;  // used by accept()
    int clilen;

    int conn_fd;  // fd for a new connection with client
    int file_fd;  // fd for file that we open for reading
    char buf[512];
    int buf_len;
	
    int fRecord = open("./registerRecord",O_RDWR);
    
    registerRecord input;
    init_server((unsigned short) atoi(argv[1]));
    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);
    fd_set set,set_bak;
    int socket_ready;
    int mfd,new_mfd;
    bool canread[20] = {false};
    bool canwrite[20] = {false};
    mfd = svr.listen_fd;
    new_mfd = mfd;
    int tmp_mfd = mfd;	    
    FD_SET(svr.listen_fd,&set);
    set_bak = set;
    char *lockstring = "Locked.\n";
    while (1) {
	/*    
	for(int j = 0;j < 20;j ++){
	fprintf(stderr,"%d\n",Record[j].AZ);
	}
	*/
        // TODO: Add IO multiplexing        
	FD_ZERO(&set);
	FD_SET(svr.listen_fd,&set);
	fprintf(stderr,"%d\n",mfd);
	for(int i = 0;i < mfd+1;i ++){
	    if(requestP[i].wait_for_write != 0){
	        FD_SET(i,&set);
	    }
	}
	socket_ready = select(mfd+1,&set,NULL,NULL,NULL);
	//fprintf(stderr,"%d\n",socket_ready);	
       	if(socket_ready > 0){
	    for(int i = 0;i < mfd+1;i ++){
		//fprintf(stderr,"%d\n",i);
		if(!FD_ISSET(i,&set)){
		    continue;
		}
		if(i == svr.listen_fd){
		    clilen = sizeof(cliaddr);
		    conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
		    //fprintf(stderr,"%d\n",conn_fd);
		    if (conn_fd < 0) {
			if(errno == EINTR || errno == EAGAIN) continue;  // try again
			if (errno == ENFILE) {
			    (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
			    continue;
			}
		        ERR_EXIT("accept");
		    }
		    fprintf(stderr,"get the connetion\n");
		    clilen = sizeof(cliaddr);
		    requestP[conn_fd].conn_fd = conn_fd;
		    if(conn_fd > mfd){
			mfd = conn_fd;
		    }
		    requestP[conn_fd].wait_for_write = 1; 
		    strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
		    fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
	            char *enter = "Please enter your id (to check your preference order):\n";
		    write(requestP[conn_fd].conn_fd,enter,strlen(enter));
		    FD_SET(conn_fd,&set_bak);
		}
#ifdef READ_SERVER      
		else{
		    if(requestP[i].wait_for_write == 1 && FD_ISSET(i,&set)){
		        //read(requestP[i].conn_fd,requestP[i].buf,sizeof(requestP[i].buf));
			int ret = handle_read(&requestP[i]); // parse data from client to requestP[conn_fd].buf
		        fprintf(stderr, "ret = %d\n", ret);
			if (ret < 0) {
			    fprintf(stderr, "bad request from %s\n", requestP[i].host);
			    FD_CLR(requestP[i].conn_fd,&set);
			    close(requestP[i].conn_fd);
			    free_request(&requestP[i]);
			    continue;
			}
			fprintf(stderr, "%s", requestP[i].buf);
			int len = strlen(requestP[i].buf);
			int id = atoi(requestP[i].buf);
			if(len != 6 || id < 902001 || id > 902020){
			    char *invalid_id = "[Error] Operation failed. Please try again.\n";
			    write(requestP[i].conn_fd,invalid_id,strlen(invalid_id));
			    FD_CLR(requestP[i].conn_fd,&set);
			    close(requestP[i].conn_fd);
			    free_request(&requestP[i]);
			    continue;
			}			
			int can_read = lock_reg(fRecord,F_SETLK,F_RDLCK,sizeof(registerRecord)*(id-902001),SEEK_SET,sizeof(registerRecord));
			//sleep(5);
			
			//fprintf(stderr,"%d %d %d\n",sizeof(Record[0]),sizeof(Record),sizeof(Record[0])*(id-902001));
			char *e = strerror(errno);
			//fprintf(stderr,"%s\n",e);
			if(can_read == -1 || canwrite[id-902001]){
			    write(requestP[i].conn_fd,lockstring,strlen(lockstring));
			    FD_CLR(requestP[i].conn_fd,&set);
			    close(requestP[i].conn_fd);
			    free_request(&requestP[i]);
			    continue;
			}
			pread(fRecord,&input,sizeof(registerRecord),sizeof(registerRecord)*(id-902001));
			if(!(len != 6 || id < 902001 || id > 902020)){
		 	    canread[id-902001] = true;
			    /*
				for(int i = 0;i < 20;i ++){
			        fprintf(stderr,"%d %d %d %d\n",Record[i].AZ,Record[i].BNT,Record[i].Moderna,Record[i].id);
			    }
			    */
			    int num = id-902001;
			    char *name[3];
			    //fprintf(stderr,"test\n");
			   // fprintf(stderr,"%d %d %d\n",Record[num].AZ-1,Record[num].BNT-1,Record[num].Moderna-1);
			    name[input.AZ-1] = "AZ"; 
			    name[input.BNT-1] = "BNT";
			    name[input.Moderna-1] = "Moderna";
			    char readout[100];
			    
			    snprintf(readout,100,"Your preference order is %s > %s > %s.\n",name[0],name[1],name[2]);
			    //fprintf(stderr,"test\n");
			    write(requestP[i].conn_fd,readout,strlen(readout));
			    fprintf(stderr,"test\n");
		    
			}
			canread[id-902001] = false;
			lock_reg(fRecord,F_SETLK,F_UNLCK,sizeof(registerRecord)*(id-902001),SEEK_SET,sizeof(registerRecord));
			FD_CLR(requestP[i].conn_fd,&set);
			close(requestP[i].conn_fd);
			free_request(&requestP[i]);
		    }
		}
	    }
#elif defined WRITE_SERVER
			    //sprintf(buf,"%s : %s",accept_write_header,requestP[conn_fd].buf);
			    //write(requestP[conn_fd].conn_fd, buf, strlen(buf));    
		else{
		    if(requestP[i].wait_for_write == 1 && FD_ISSET(i,&set)){
		        int ret = handle_read(&requestP[i]); // parse data from client to requestP[conn_fd].buf
		        fprintf(stderr, "ret = %d\n", ret);
			if (ret < 0) {
			    fprintf(stderr, "bad request from %s\n", requestP[i].host);
			    FD_CLR(requestP[i].conn_fd,&set);
			    close(requestP[i].conn_fd);
			    free_request(&requestP[i]);
			    continue;
			}
			int len = strlen(requestP[i].buf);
			int id = atoi(requestP[i].buf);
			requestP[i].id = id;
			if(len != 6 || id < 902001 || id > 902020){
			    char *invalid_id = "[Error] Operation failed. Please try again.\n";
			    write(requestP[i].conn_fd,invalid_id,strlen(invalid_id));
			    FD_CLR(requestP[i].conn_fd,&set);
			    close(requestP[i].conn_fd);
			    free_request(&requestP[i]);
			    continue;
			}

			int can_write = lock_reg(fRecord,F_SETLK,F_WRLCK,sizeof(registerRecord)*(id-902001),SEEK_SET,sizeof(registerRecord));
			if(can_write == -1 || canwrite[id-902001] || canread[id-902001]){
			    write(requestP[i].conn_fd,lockstring,strlen(lockstring));
			    FD_CLR(requestP[i].conn_fd,&set);
			    close(requestP[i].conn_fd);
			    free_request(&requestP[i]);
			    continue;
			}
			pread(fRecord,&input,sizeof(registerRecord),sizeof(registerRecord)*(id-902001));
			if(!(len != 6 || id < 902001 || id > 902020)){
			    canwrite[id-902001] = true;
			    int num = id-902001;
			    char *name[3];
			    name[input.AZ-1] = "AZ"; 
			    name[input.BNT-1] = "BNT";
			    name[input.Moderna-1] = "Moderna";
			    char readout[100];
			    snprintf(readout,100,"Your preference order is %s > %s > %s.\n",name[0],name[1],name[2]);
			    write(requestP[i].conn_fd,readout,strlen(readout));
			    char *please = "Please input your preference order respectively(AZ,BNT,Moderna):\n";
			    write(requestP[i].conn_fd,please,strlen(please));
			    requestP[i].wait_for_write = 2;
			}
		    }
		    else if(requestP[i].wait_for_write == 2 && FD_ISSET(i,&set)){
			int id = requestP[i].id;
			int num = id-902001;
			char arr[10];
			char name[3];
			memset(arr,' ',sizeof(arr));
		        int ret = handle_read(&requestP[i]); // parse data from client to requestP[conn_fd].buf
		        fprintf(stderr, "ret = %d\n", ret);
			if (ret < 0) {
			    fprintf(stderr, "bad request from %s\n", requestP[i].host);
			    FD_CLR(requestP[i].conn_fd,&set);
			    close(requestP[i].conn_fd);
			    free_request(&requestP[i]);
			    canwrite[num] = false;
			    lock_reg(fRecord,F_SETLK,F_UNLCK,sizeof(registerRecord)*(id-902001),SEEK_SET,sizeof(registerRecord));
			    continue;
			}
			int n[3];
			int count = 0;
			for(int j = 0;j < 5;j ++){
			    if(requestP[i].buf[j] != ' '){
				n[count] = (int)requestP[i].buf[j]-48;
				count ++;
			    }
			}
			fprintf(stderr,"%d %d %d %d\n",strlen(requestP[i].buf),n[0],n[1],n[2]);
			if(strlen(requestP[i].buf) != 5 || (n[0] != 1 && n[0] != 2 && n[0] != 3) || (n[2] != 1 && n[2] != 2 && n[2] != 3) || (n[1] != 1 && n[1] != 2 && n[1] != 3) || n[0]+n[2]+n[1] != 6){
			    char *invalid_id = "[Error] Operation failed. Please try again.\n";
			    write(requestP[i].conn_fd,invalid_id,strlen(invalid_id));
			    canwrite[num] = false;
			    lock_reg(fRecord,F_SETLK,F_UNLCK,sizeof(registerRecord)*(id-902001),SEEK_SET,sizeof(registerRecord));
			    FD_CLR(requestP[i].conn_fd,&set);
			    close(requestP[i].conn_fd);
			    free_request(&requestP[i]);
			    continue;
			}
			
			//pread(fRecord,&input,sizeof(registerRecord),sizeof(registerRecord)*(id-902001));
			fprintf(stderr,"%d %d %d\n",n[0],n[1],n[2]);
			char *name2[3]; 
			name2[n[0]-1] = "AZ";
			name2[n[1]-1] = "BNT";
			name2[n[2]-1] = "Moderna";
			input.AZ = n[0];
			input.BNT = n[1];
			input.Moderna = n[2];
			char readout[100];
			pwrite(fRecord,&input,sizeof(registerRecord),sizeof(registerRecord)*(id-902001));	
			snprintf(readout,100,"Preference order for %d modified successed, new preference order is %s > %s > %s.\n",id,name2[0],name2[1],name2[2]);
			//fprintf(stderr,"end\n");
			//Record[num].AZ = n[0];
			//Record[num].BNT = n[1];
			//Record[num].Moderna = n[2];
			//lseek(fRecord,SEEK_SET,sizeof(Record[0])*num);
			//write(fRecord,&Record[num],sizeof(Record[0]));
			canwrite[num] = false;
			lock_reg(fRecord,F_SETLK,F_UNLCK,sizeof(registerRecord)*(id-902001),SEEK_SET,sizeof(registerRecord));
			write(requestP[i].conn_fd,readout,strlen(readout));
			FD_CLR(requestP[i].conn_fd,&set);
		        close(requestP[i].conn_fd);
		        free_request(&requestP[i]);
			fprintf(stderr,"end\n");
		    }
		}
    	    }
#endif

		//close(requestP[conn_fd].conn_fd);
		//free_request(&requestP[conn_fd]);
	}
    }
    free(requestP);
    return 0;
}

// ======================================================================================================
// You don't need to know how the following codes are working
#include <fcntl.h>

static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->id = 0;
    reqP->wait_for_write = 0;
}

static void free_request(request* reqP) {
    /*if (reqP->filename != NULL) {
        free(reqP->filename);
        reqP->filename = NULL;
    }*/
    init_request(reqP);
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }

    // Get file descripter table size and initialize request table
    maxfd = getdtablesize();
    requestP = (request*) malloc(sizeof(request) * maxfd);
    if (requestP == NULL) {
        ERR_EXIT("out of memory allocating all requests");
    }
    for (int i = 0; i < maxfd; i++) {
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    return;
}

