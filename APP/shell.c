#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "serial.h"
#include "shell.h"

//command max length
#define MAXARGS 32

static int cmd_seq;
void pr_result(pr p, int result)
{
  if(result != 0)
    p("CMD%04d: ERROR %d\n", (cmd_seq > 0 && cmd_seq <= 9999) ? cmd_seq : 0, result);
  else
    p("CMD%04d: OK\n", (cmd_seq > 0 && cmd_seq <= 9999) ? cmd_seq : 0);
}

int do_V (int argc, char *argv[], pr p)
{
	p("hello world!!!!! \n");
	return 0;
} 

struct cmd_tbl command_table[] = {
  // put the commands used more frequently first to save the matching time
  {"V", do_V },
};
int table_length = sizeof(command_table) / sizeof(command_table[0]);

int parse_line(char *buf, char *argv[])
{
  int nargs = 0;
  while (nargs < MAXARGS) {
    /* skip leading space */
    while (isblank(*buf))
      ++buf;
    /* end of line */
    if (*buf == '\0') {
      argv[nargs] = NULL;
      return nargs;
    }
    /* save arg ptr */
    argv[nargs++] = buf;
    /* find end of arg */
    while (*buf && !isblank(*buf))
      ++buf;
    /* end of line */
    if (*buf == '\0') {
      argv[nargs] = NULL;
      return nargs;
    }
    /* terminate */
    *buf++ = '\0';
  }
  return nargs;
}

static void parse_and_process(char *buf, pr p)
{
  int argc;
  char *argv[MAXARGS];
  struct cmd_tbl *p_tbl = command_table;
  int found = 0;
  int result;
  char seq_str[5];
  int seq;

  while (isblank(*buf))
    ++buf;
  // CMDxxxx
  if ((strncmp(buf, "CMD", 3) == 0) && (strlen(buf) >= 7)) {
    memcpy(seq_str, buf + 3, 4);
    seq_str[4] = 0;
    seq = atoi(seq_str);
    if (seq > 0 && seq <= 9999)
      cmd_seq = seq;
    buf += 7;
  }
  if (strlen(buf) == 0)
    return;

  if ((argc = parse_line(buf, argv)) == 0) {
    p("Unknown command '%s'\n", argv[0]);
    return;
  }

  for(; p_tbl != &command_table[table_length]; p_tbl++) {
    if (strncmp(argv[0], p_tbl->name, strlen(p_tbl->name)) == 0) {
      found = 1;
      break;
    }
  }
  if (!found) {
    p("Unknown command '%s'\n", argv[0]);
    result = -1;
  } else {
    result = p_tbl->cmd(argc, argv, p);
  }

  pr_result(p, result);
}

#define ITM_PROMPT "DBG> "

char ser_buf[1024];
void serRX(void const *argument) {
  int dat;
  int col = 0;
  while (1) {
    dat = SER_getchar();
    if (dat == '\r' || dat == '#' || dat == '\n') {
      ser_buf[col] = '\0';
      col = 0;
      if (strlen(ser_buf) > 0) parse_and_process(ser_buf, SER_printf);
    } else {
      if (col < 1023)
        ser_buf[col++] = dat;
    }
  }
}
