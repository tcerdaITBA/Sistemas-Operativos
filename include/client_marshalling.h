#ifndef _CLIENT_MARSHALLING_
#define _CLIENT_MARSHALLING_

#include "marshalling.h"

typedef struct session * sessionADT;

sessionADT start_session(char * a);

void end_session(sessionADT se);

int send_tweet(sessionADT se, char * user, char * msg);

int send_like(sessionADT se, int tweet_id);

t_tweet * send_refresh(sessionADT se, int tw_count, int * received_count);

#endif