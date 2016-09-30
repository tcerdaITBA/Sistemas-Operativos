//server_marshalling.c
#include "include/server_marshalling.h"
#include "include/IPC.h"
#include "include/marshalling.h"
#include "include/server.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define CMDS_SIZE (sizeof(commands)/sizeof(commands[0]))

struct t_session {
  t_connectionADT con;
  t_master_sessionADT master_se;
  void * data;
};

struct t_master_session {
  t_addressADT addr;
};

t_tweet create_tweet(char * usr, char * msg);
char * deploy_tweet (t_tweet twe);

int execute(char *args, t_responseADT res, void * data);

int tweet( char * msg , t_responseADT res, void * data);
int like(char * msg, t_responseADT res, void * data);
int refresh(char * msg, t_responseADT res, void * data);
int login(char * msg, t_responseADT res, void * data);
int logout(char * msg, t_responseADT res, void * data);

/*
** Cada función decodifica msg, llama a la función del servidor y settea en
** el response la respuesta. NO LA ENVÍA. attend se encarga de enviarla.
*/
typedef int (*command) (char* msg, t_responseADT res, void * data);

static command commands[]= {tweet, like, refresh, login, logout};

t_master_sessionADT setup_master_session(char *sv_path) {

  t_addressADT addr = create_address(sv_path);
  t_master_sessionADT se;

  if (addr == NULL) {
    printf("failed to create address\n");
    exit(0);
  }

  printf("Opening channel...\n");

  printf("Server listening\n");

  if (listen_peer(addr) < 0) {
    fprintf(stderr, "Cannot listen\n");
    return NULL;
  }

  se = malloc(sizeof(struct t_session));
  se->addr = addr;
  return se;
}

t_sessionADT accept_client(t_master_sessionADT master_session) {
  t_connectionADT con = accept_peer(master_session->addr);
  t_sessionADT se;

  if (con == NULL) {
    printf("Accept failed.\n");
    return NULL;
  }

  printf("Accepted!\n");

  se = malloc(sizeof(struct t_session));

  se->con = con;
  se->master_se = master_session;

  return se;
}

void end_session(t_sessionADT se) {
  if (se != NULL) {
    unaccept(se->con);
    free(se);
  }
}

void end_master_session(t_master_sessionADT master_se) {
  if (master_se != NULL) {
    unlisten_peer(master_se->addr);
    free_address(master_se->addr);
    free(master_se);
  }
}

void set_session_data(t_sessionADT session, void * data) {
  session->data = data;
}

int attend(t_sessionADT se) {
  char buffer[BUFSIZE];
  int valid;
  t_connectionADT con = se->con;
  t_responseADT res = create_response();
  t_requestADT req;

  while(1) {
    req = read_request(con);
    if (req == NULL)
      return 0;
    get_request_msg(req, buffer);
    valid = execute(buffer, res, se->data);
    if (!valid)
      set_response_msg(res, INVALID_MSG);
    if (send_response(req, res) < 0)
      return -1;
  }
}

int execute(char *request, t_responseADT res, void * data) {
    int i;
    int opcode = atoi(request);
    char * start = strstr(request, SEPARATOR) + strlen(SEPARATOR);

    if (opcode < 0 || opcode >= CMDS_SIZE)
      return 0;

    return (*commands[opcode]) (start, res, data);
}

int tweet(char * str, t_responseADT res, void * data) {
  char msg[MSG_SIZE];
  char response[SHORTBUF];
  int posted_id; //id del tweet enviado

  strcpy(msg, str);

  posted_id = sv_tweet(data, msg);

  //get last id from database
  sprintf(response, "%d", posted_id);

  set_response_msg(res, response);
  return 1;
}

int like(char * msg, t_responseADT res, void * data) {
  int id = atoi(msg);
  char str[SHORTBUF];

  int likes = sv_like(data, id);

  if (likes == -1)
    return 0;

  sprintf(str, "%d", likes);
  set_response_msg(res, str);
  return 1;
}

// TODO: agregar return 0 si arreglo de tweets es NULl
int refresh(char * msg, t_responseADT res, void * data) {
  int from_id = atoi(msg);
  char str[BUFSIZE];
  t_tweet * tws = sv_refresh(data, from_id, str);

  printf("[SV M]: Received from server: %s\n", str);
  set_response_msg(res, str); //lo que viene de DB lo manda directo por el tubo porque tiene bien los separadores.

  return 1;
}

int login(char * msg, t_responseADT res, void * data) {
  int valid = sv_login(data, msg);
  if (valid)
    set_response_msg(res, "");
  return valid;
}

int logout(char * msg, t_responseADT res, void * data) {
  int valid = sv_logout(data);
  if (valid)
    set_response_msg(res, "");
  return valid;
}
