#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char *argv[]) {
  int server_fd = -1, client_fd;

  struct sockaddr_in address;
  socklen_t addrlen = sizeof(address);

  char buffer[1024];

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return 1;
  }

  int opt = 1;

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
      -1) {
    perror("setsockopt");
    close(server_fd);
    return 1;
  }

  memset(&address, 0, sizeof(address));

  address.sin_port = htons(PORT);
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
    perror("bind");
    close(server_fd);
    return 1;
  }

  if (listen(server_fd, SOMAXCONN) == -1) {
    perror("listen");
    close(server_fd);
    return 1;
  }

  while (1) {
    if ((client_fd =
             accept(server_fd, (struct sockaddr *)&address, &addrlen)) == -1) {
      perror("accept");
      continue;
    }

    ssize_t buff_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (buff_read > 0) {
      buffer[buff_read] = '\0';

      char *response_body = "Hello from server";
      int response_len = strlen(response_body);

      char response_buffer[1024];

      int buffer_len = snprintf(response_buffer, sizeof(response_buffer),
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: %d\r\n"
                                "\r\n"
                                "%s",
                                response_len, response_body);

      send(client_fd, response_buffer, buffer_len, 0);
    } else if (buff_read == -1) {
      fprintf(stderr, "recv error\n");
    }
    if (buff_read == 0) {
      fprintf(stderr, "recv: user closed connection\n");
    }
    close(client_fd);
  }
}
