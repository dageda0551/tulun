#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <event.h>
#include <signal.h>

#include <unistd.h>
#include <arpa/inet.h>


#define RPG_DC_ID_MAX_NUM 4
#define DC_MAX_NUM 4

enum meta_msg_type {
    MSG_REQ_RSP_META_BEGIN = 0,
    MSG_REQ_META_CREATE,
};

struct meta_msg_header
{
    uint64_t msg_id;
    uint32_t msg_type;
};

struct meta_req_para
{
    uint64_t msg_id;
    uint32_t msg_type;
};

struct meta_rsp_para
{
    uint64_t msg_id;
    uint32_t msg_type;
};


struct dm_msg_header
{
    uint32_t msg_len;
    uint32_t msg_type;
};


