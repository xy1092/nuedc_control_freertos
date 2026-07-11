#ifndef RTT_LOG_H
#define RTT_LOG_H

#include <stdint.h>

void RTT_Log_Write(const char *data, uint32_t len);
void RTT_Log_WriteString(const char *s);

#endif
