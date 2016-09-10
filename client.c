#include "fifo_data.h"

void send_request(char * fifo, t_request req);
t_response read_response(char * fifo);

int main()
{
  int server_fd, client_fd, cid_fd, cid;

  // El fifo del cliente lo tiene que dar el servidor para asegurarse que cada
  // cliente tenga un nombre distinto.

  t_request req = { .msg_len = sizeof(ID_SIGNAL), .msg = ID_SIGNAL, \
    .fifo_len = sizeof(CID_FIFO), .res_fifo = CID_FIFO};

  send_request(SERVER_FIFO_PATH, req);
  printf("Request sent by client\n");

  t_response res = read_response(CID_FIFO);

  printf("Response read by client\n");
  printf("msg: %d\n", res.msg_len);
  printf("str: %s\n", res.msg);
}

void send_request(char * fifo, t_request req) {
  int fd = open(fifo, O_RDWR);
  write(fd, &req.msg_len, sizeof(int));
  write(fd, req.msg, req.msg_len);
  write(fd, &req.fifo_len, sizeof(int));
  write(fd, req.res_fifo, req.fifo_len);
  close(fd);
}

t_response read_response(char * fifo) {
  t_response res;
  int fd = open(fifo, O_RDWR);
  read(fd, &res.msg_len, sizeof(int));
  res.msg = malloc(res.msg_len);
  read(fd, res.msg, res.msg_len);
  close(fd);
  return res;
}
