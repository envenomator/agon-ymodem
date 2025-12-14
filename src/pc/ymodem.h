#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void ymodem_send(int port, int filecount, char **filenames);
void ymodem_receive(int port);

#ifdef __cplusplus
}
#endif
