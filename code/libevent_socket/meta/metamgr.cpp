#include "metamgr.h"

#include <list>
#include <map>
using namespace std;

#define PORT        8103
#define BACKLOG     5
#define MEM_SIZE    1024
#define IP_ADDR 100

struct Sock_ev
{  
    struct event* read_ev;  
    struct event* write_ev;
	char* buffer;
	int len;
    int sock;
};

char dm_ip[IP_ADDR];
int dm_port;
uint32_t g_rpg_id = 0;

struct event_base* base = NULL;
int metamgrSock;
int deploymgrSock;
struct event* listen_metamgr_ev = NULL;

list<Sock_ev*> metamgrLinkSockList;

void release_sock_event(struct Sock_ev* ev)  
{  
    event_del(ev->read_ev);  
    free(ev->read_ev);
	event_del(ev->write_ev); 
    free(ev->write_ev);  
    free(ev->buffer);  
    free(ev);  
}  
  
void on_write_to_metamgr(int sock, short event, void* arg)  
{
    Sock_ev* ev = (Sock_ev*)arg;  
    send(sock, ev->buffer, ev->len, 0);

    meta_rsp_para* rsp = (meta_rsp_para*)ev->buffer;
	printf("rsp: %ld, %d\n", rsp->msg_id, rsp->msg_type);
	
    free(ev->buffer);  
}

 
void on_read_from_metamgr(int sock, short event, void* arg)  
{
    Sock_ev* ev = (Sock_ev*)arg;
	
    ev->buffer = (char*)malloc(MEM_SIZE);
    bzero(ev->buffer, MEM_SIZE);
	
    int size = recv(sock, ev->buffer, MEM_SIZE, 0); 
    printf("receive data:%s, size:%d\n", ev->buffer, size);
	
    if (size <= 0)
    {
        metamgrLinkSockList.remove(ev);
        printf("remove sock fd = %d\n", sock);
        release_sock_event(ev);  
        close(sock);

        /*list<Sock_ev*>::iterator it = metamgrLinkSockList.begin();  
        for(; it != metamgrLinkSockList.end(); it++)  
        {
            char content[] = "thank you";
            (*it)->buffer = (char*)malloc(MEM_SIZE);  
            bzero((*it)->buffer, MEM_SIZE);
            strcpy((*it)->buffer, content);
            event_set((*it)->write_ev, (*it)->sock, EV_WRITE, on_write, (*it)->buffer);  
            event_base_set(base, (*it)->write_ev);  
            event_add((*it)->write_ev, NULL);
        } */

        return;  
    }

    meta_req_para* req = (meta_req_para*)ev->buffer;
    printf("req: %ld, %d\n", req->msg_id, req->msg_type);
	
    meta_msg_header* meta_msg_h = (meta_msg_header*)ev->buffer;
    switch (meta_msg_h->msg_type)
    {
        case MSG_REQ_META_CREATE:
        {
            meta_rsp_para rsp;
        	rsp.msg_id = req->msg_id;
        	rsp.msg_type = req->msg_type;

        	bzero(ev->buffer, MEM_SIZE);
        	memcpy(ev->buffer, &rsp, sizeof(meta_rsp_para));
        	ev->len = sizeof(meta_rsp_para);
    		
            event_set(ev->write_ev, sock, EV_WRITE, on_write_to_metamgr, ev);  
            event_base_set(base, ev->write_ev);  
            event_add(ev->write_ev, NULL);
    	}
            break;

        default:
            break;
    }
}

void on_accept_metamgr(int sock, short event, void* arg)  
{  
    struct sockaddr_in cli_addr;

    socklen_t sin_size = sizeof(struct sockaddr_in);  
    int fd = accept(sock, (struct sockaddr*)&cli_addr, &sin_size);
	if (-1 == fd)
	{
	    printf("on_accept_metamgr accept fail !\r\n");
	    return;
	}

	/* read_ev must allocate from heap memory, otherwise the program would crash from segmant fault */
    Sock_ev* ev = (Sock_ev*)malloc(sizeof(Sock_ev));  
    ev->read_ev = (struct event*)malloc(sizeof(struct event));  
    ev->write_ev = (struct event*)malloc(sizeof(struct event));  

	ev->sock = fd;
    event_set(ev->read_ev, fd, EV_READ|EV_PERSIST, on_read_from_metamgr, ev);  
    event_base_set(base, ev->read_ev);  
    event_add(ev->read_ev, NULL);

    metamgrLinkSockList.push_back(ev);
    printf("add sock fd = %d\n", fd);
}   

void termination_handler(int signum)
{
    printf("Exit......\r\n");
	event_del(listen_metamgr_ev);
    free(listen_metamgr_ev);
    close(metamgrSock);
	close(deploymgrSock);
    exit(0);
}

void on_read_deploy_manager(int sock, short event, void* arg)
{
    struct event* listen_ev = (struct event*)arg;
    char buf[MEM_SIZE];
    int size = read(deploymgrSock, buf, MEM_SIZE);
    
    if (size <= 0)
    {  
        event_del(listen_ev);
        free(listen_ev);
        close(deploymgrSock);
        printf("deploy manager disconnect !\r\n");
        return;  
    } 
    printf("response from deploy manager:\n");
    printf("receive data:%s, size:%d\n", buf, size);

    dm_msg_header* dm_msg_h = (dm_msg_header*)buf;

    switch (dm_msg_h->msg_type)
    {
        case 0:
        {
            break;
        }
        default:
            break;
    }
}

int connect_deploy_manager()
{
    struct sockaddr_in servaddr;
	
    deploymgrSock = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, dm_ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(dm_port);
    
    if (-1 == connect(deploymgrSock, (struct sockaddr*)&servaddr, sizeof(servaddr)))
	{
	    printf("connect fail !\r\n");
	    return -1;
	}

    struct event* listen_ev = (struct event*)malloc(sizeof(struct event));
    event_set(listen_ev, deploymgrSock, EV_READ|EV_PERSIST, on_read_deploy_manager, listen_ev);  
    event_base_set(base, listen_ev);  
    event_add(listen_ev, NULL);

	printf("connect succeed !\r\n");
	
	return 0;
}

int listen_metamgr()
{
    struct sockaddr_in s_add;

    metamgrSock = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == metamgrSock)
    {
        printf("socket fail !\r\n");
        return -1;
    }

    int yes = 1;  
    setsockopt(metamgrSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));  

    bzero(&s_add, sizeof(struct sockaddr_in));
    s_add.sin_family = AF_INET;
    s_add.sin_addr.s_addr = htonl(INADDR_ANY);
    s_add.sin_port = htons(PORT);

    if(-1 == bind(metamgrSock, (struct sockaddr*)(&s_add), sizeof(struct sockaddr)))
    {
        printf("bind fail !\r\n");
        return -1;
    }

    if(-1 == listen(metamgrSock, BACKLOG))
    {
        printf("listen fail !\r\n");
        return -1;
    }

    listen_metamgr_ev = (struct event*)malloc(sizeof(struct event)); 
    event_set(listen_metamgr_ev, metamgrSock, EV_READ|EV_PERSIST, on_accept_metamgr, NULL);  
    event_base_set(base, listen_metamgr_ev);  
    event_add(listen_metamgr_ev, NULL);
	
	return 0;
}

int main(int argc, char **argv)
{
    if (argc == 3)
    {        
        strcpy(dm_ip, argv[1]);
        dm_port = atoi(argv[2]);
    }
    else
    {
        printf("Please input three parameters: dm ip, dm port !\n");
        return -1;
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

    base = event_base_new();

	if (-1 == listen_metamgr())
	{
	    printf("listen_metamgr fail !\r\n");
		return -1;
	}

	if (-1 == connect_deploy_manager())
	{
	    printf("connect_deploy_manager fail !\r\n");
		return -1;
	}

	event_base_dispatch(base);

    return 0;
}
