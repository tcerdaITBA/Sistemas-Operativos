
#include "include/IPC.h"
#include "include/client_marshalling.h"
#include "include/marshalling.h"

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

struct session {
  t_connectionADT con;
  t_requestADT req;
};

static int send_op(sessionADT se, char * op_bytes, char res_bytes[BUFSIZE]);

static int process_tweets(char res[], t_tweet * tws);

sessionADT start_session(char * path) {
  sessionADT se = malloc(sizeof(struct session));
  t_addressADT sv_addr = create_address(path);
  se->req = create_request();
  se->con = connect_peer(sv_addr);
  return se;
}

void end_session(sessionADT se) {
  disconnect(se->con);
  free_request(se->req);
  free(se);
}

// TODO: capaz se pueden modularizar

int send_tweet(sessionADT se, char * user, char * msg) {
  char req_bytes[BUFSIZE], res[BUFSIZE];

  sprintf(req_bytes, "%s%s%s%s%s", OPCODE_TWEET, SEPARATOR, user, SEPARATOR, msg);

  if (send_op(se, req_bytes, res) == 0) {
    return -1;
  }

  return atoi(res);
}

int send_like(sessionADT se, int tweet_id) {
  char req_bytes[BUFSIZE], res[BUFSIZE];

  sprintf(req_bytes, "%s%s%d", OPCODE_LIKE, SEPARATOR, tweet_id);

  if (send_op(se, req_bytes, res) == 0) {
    return -1;
  }

  return atoi(res);
}

t_tweet * send_refresh(sessionADT se, int *size) {
  char req_bytes[BUFSIZE], res[BUFSIZE];
  int count, from_id;
  t_tweet * tws = malloc(MAX_TW_REFRESH);

  count = from_id = *size = 0;

  do {
    tws = realloc(tws, (MAX_TW_REFRESH + *size) * sizeof(*tws));
    sprintf(req_bytes, "%s%s%d%s", OPCODE_REFRESH, SEPARATOR, from_id, SEPARATOR);

    if (send_op(se, req_bytes, res) == 0)
      return NULL;

    count = process_tweets(res, tws + *size);
    *size += count;
    from_id += MAX_TW_REFRESH;
  } while (count == MAX_TW_REFRESH);

  return tws;
}

static int process_tweets(char res[], t_tweet * tws) {
  char str[BUFSIZE]; // para proteger a res de las modificaciones que le hace strtok()
  int i = 0;
  strcpy(str, res);

  char * token = strtok(str, SEPARATOR);
  while (token != NULL) {

    // Saca los atributos del tweet.
    tws[i].id = atoi(token);
    strcpy(tws[i].user, strtok(NULL, SEPARATOR));
    strcpy(tws[i].msg, strtok(NULL, SEPARATOR));
    tws[i].likes = atoi(strtok(NULL, SEPARATOR));

    token = strtok(NULL, SEPARATOR);
    i++;
  }

  return i;
}

static int send_op(sessionADT se, char * op_bytes, char res_bytes[BUFSIZE]) {
  set_request_msg(se->req, op_bytes);
  t_responseADT res = send_request(se->con, se->req);
  if (res != NULL)
    get_response_msg(res, res_bytes);
  return res != NULL;
}
