#ifndef PARSER_H
#define PARSER_H

#include <Arduino.h>

char** parse(char* input, const char* delim, int maxBuffer = 5)
{
  if (input == NULL || delim == NULL) return NULL;

  char** buffer = (char**)calloc(maxBuffer, sizeof(char*));
  if (!buffer) {
    Serial.println("❌ Gagal mengalokasikan memori untuk buffer parse");
    return NULL;
  }

  int index = 0;
  char* token = strtok(input, delim);
  while (token != NULL && index < maxBuffer - 1) {
    buffer[index] = strdup(token); 
    if (!buffer[index]) {
      Serial.printf("❌ Gagal alokasi untuk token ke-%d\n", index);
      for (int j = 0; j < index; j++) free(buffer[j]);
      free(buffer);
      return NULL;
    }
    index++;
    token = strtok(NULL, delim);
  }

  return buffer;
}

void freeParse(char** buffer, int maxBuffer)
{
  if (!buffer) return;
  for (int i = 0; i < maxBuffer; i++) {
    if (buffer[i]) free(buffer[i]);
  }
  free(buffer);
}

#endif