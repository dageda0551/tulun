#include "metamgr.h"


#define MAXLINE 1024
#define PORT 8103


struct event_base* base;
int sockfd;

void termination_handler(int signum)
{
    printf("Exit......\n");
    close(sockfd);
    exit(0);
}

void on_read(int sock, short event, void* arg)
{
    struct event* listen_ev = (struct event*)arg;
    char buf[MAXLINE];
    int n = read(sockfd, buf, MAXLINE);
    
    if (n <= 0) {  
        event_del(listen_ev);
        free(listen_ev);
        close(sockfd);
        printf("server disconnect !\n");
        return;  
    } 
    printf("Response from server:\n");

	meta_rsp_para* rsp = (meta_rsp_para*)buf;
    printf("rsp: %ld, %d\n", rsp->msg_id, rsp->msg_type);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fputs("Please input two para !\n", stderr);
        exit(1);
    }

    struct sigaction new_action, old_action;
    /* Configure SIGINT */
    new_action.sa_handler = termination_handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction (SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN){
        sigaction (SIGINT, &new_action, NULL);
    }

    struct sockaddr_in servaddr;
    char *str = argv[1];
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(PORT);
    
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	meta_req_para req;
	req.msg_id = 1;
	req.msg_type = MSG_REQ_META_CREATE;
	write(sockfd, &req, sizeof(meta_req_para));
	
    /*  */
    struct event* listen_ev = (struct event*)malloc(sizeof(struct event));  
    base = event_base_new();  
    event_set(listen_ev, sockfd, EV_READ|EV_PERSIST, on_read, listen_ev);  
    event_base_set(base, listen_ev);  
    event_add(listen_ev, NULL);  
    event_base_dispatch(base);  

    return 0;
}

