#ifndef LOG_H
#define LOG_H

void log_init();
void log_save(char* input);
void log_show();
void log_purge();
void log_execute(int index, char** prev_dir, char* shell_home);

#endif