#ifndef PRINTU_H
#define PRINTU_H

void printu(const char *format_string, ...);

#define printf(...) do { printk(__VA_ARGS__); printu(__VA_ARGS__); } while(0)

#endif /* PRINTU_H */
