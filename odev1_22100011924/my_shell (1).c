// Öğrenci No:22100011924
// Ad-Soyad:suphi kuse
// (Kodunuzu SADECE '>>> SORU n BASLANGIC' ve '<<< SORU n BITIS' satirlari arasina yazin.
//  Bu isaret satirlarini silmeyin/degistirmeyin!)
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

// Simplifed xv6 shell.

#define MAXARGS 10

// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd {
  int type;          //  ' ' (exec), '|' (pipe), 'A' (&&), 'O' (||), ';' (list), '<' or '>'
};

struct execcmd {
  int type;              // ' '
  char *argv[MAXARGS];   // arguments to the command to be exec-ed
};

struct redircmd {
  int type;          // < or >
  struct cmd *cmd;   // the command to be run (e.g., an execcmd)
  char *file;        // the input/output file
  int mode;          // the mode to open the file with
  int fd;            // the file descriptor number to use for the file
};

struct pipecmd {
  int type;          // '|'
  struct cmd *left;  // ciktisi boruya (pipe) yazilacak komut
  struct cmd *right; // girdisini borudan (pipe) okuyacak komut
};

struct condcmd {
  int type;          // 'A' (&&) veya 'O' (||)
  struct cmd *left;  // once calisacak komut
  struct cmd *right; // kosul saglanirsa calisacak komut
};

struct listcmd {
  int type;          // ;
  struct cmd *left;  // first command (run and wait)
  struct cmd *right; // second command (run after left finishes)
};

int fork1(void);  // Fork but exits on failure.
struct cmd *parsecmd(char*);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int r;
  struct execcmd *ecmd;
  struct redircmd *rcmd;
  struct pipecmd *pcmd;
  struct condcmd *ccmd;
  struct listcmd *lcmd;

  if(cmd == 0)
    exit(0);

  switch(cmd->type){
  default:
    fprintf(stderr, "unknown runcmd\n");
    exit(-1);

  case ' ':
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit(0);
    // Basit komutlar HAZIR verilmistir (bu bolumu degistirmeyin).
    execvp(ecmd->argv[0], ecmd->argv);
    fprintf(stderr, "komut bulunamadi: %s\n", ecmd->argv[0]);
    exit(1);
    break;

  case '>':
  case '<':
    rcmd = (struct redircmd*)cmd;
    // -------------------------------------------------------
    // >>> SORU 1 BASLANGIC: Girdi/cikti yonlendirme (< > ve ardisik yonlendirme)
    // -------------------------------------------------------

         int fd = open(rcmd->file, rcmd->mode, 0644);

    if(fd < 0) {
      fprintf(stderr, "dosya acilamadi: %s\n", rcmd->file);
      exit(1);
    }

    if(dup2(fd, rcmd->fd) < 0) {
      close(fd);
      exit(1);
    }

    close(fd);

    // -------------------------------------------------------
    // <<< SORU 1 BITIS
    // -------------------------------------------------------
    runcmd(rcmd->cmd);
    break;

  case '|':
    pcmd = (struct pipecmd*)cmd;
    // -------------------------------------------------------
    // >>> SORU 2 BASLANGIC: Pipe (|)
    // -------------------------------------------------------

        int p[2];
    if(pipe(p) < 0) {
      fprintf(stderr, "pipe failed\n");
      exit(1);
    }

    if(fork1() == 0) {
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }

    if(fork1() == 0) {
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }

    close(p[0]);
    close(p[1]);

    wait(&r);
    wait(&r);
    exit(0);

    // -------------------------------------------------------
    // <<< SORU 2 BITIS
    // -------------------------------------------------------
    break;

  case 'A':   // &&
  case 'O':   // ||
    ccmd = (struct condcmd*)cmd;
    // -------------------------------------------------------
    // >>> SORU 3 BASLANGIC: Kosullu yürütme (&& ||)
    // -------------------------------------------------------

        int status;
    int pid = fork1();

    if(pid == 0)
      runcmd(ccmd->left);

    waitpid(pid, &status, 0);

    int success = WIFEXITED(status) && WEXITSTATUS(status) == 0;

    if((cmd->type == 'A' && success) ||
       (cmd->type == 'O' && !success)) {
      runcmd(ccmd->right);
    }

    exit(success ? 0 : 1);

    // -------------------------------------------------------
    // <<< SORU 3 BITIS
    // -------------------------------------------------------
    break;

  case ';':
    lcmd = (struct listcmd*)cmd;
    // -------------------------------------------------------
    // >>> SORU 4 BASLANGIC: Komut listesi (;)
    // -------------------------------------------------------

    int status;
    int pid = fork1();

    if(pid == 0)
      runcmd(lcmd->left);

    waitpid(pid, &status, 0);

    runcmd(lcmd->right);
    // -------------------------------------------------------
    // <<< SORU 4 BITIS
    // -------------------------------------------------------
    break;
  }
  exit(0);
}

int
getcmd(char *buf, int nbuf)
{

  if (isatty(fileno(stdin)))
    fprintf(stdout, "$ ");
  memset(buf, 0, nbuf);
  fgets(buf, nbuf, stdin);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(void)
{
  static char buf[100];
  int r;

  // stdin tamponsuz: cocuk prosesler exit() ile cikarken dosyadan okunan
  // komut satirlarinin tekrar okunmasini engeller (./a.out < input.sh icin).
  setvbuf(stdin, NULL, _IONBF, 0);

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Clumsy but will have to do for now.
      // Chdir has no effect on the parent if run in the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(stderr, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait(&r);
  }
  exit(0);
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    perror("fork");
  return pid;
}

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ' ';
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, int type)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->mode = (type == '<') ?  O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC;
  cmd->fd = (type == '<') ? 0 : 1;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = '|';
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
condcmd(struct cmd *left, struct cmd *right, int type)
{
  struct condcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ';';
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>;&";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '&':   // "&&" -> 'A'  (tek '&' desteklenmez)
    if(s+1 < es && s[1] == '&'){
      ret = 'A';
      s += 2;
    } else {
      s++;
    }
    break;
  case '|':   // "||" -> 'O' ,  tek "|" -> '|' (pipe)
    if(s+1 < es && s[1] == '|'){
      ret = 'O';
      s += 2;
    } else {
      s++;
    }
    break;
  case ';':
  case '<':
    s++;
    break;
  case '>':
    s++;
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;

  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsecond(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char
*mkcopy(char *s, char *es)
{
  int n = es - s;
  char *c = malloc(n+1);
  assert(c);
  strncpy(c, s, n);
  c[n] = 0;
  return c;
}

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    fprintf(stderr, "leftovers: %s\n", s);
    exit(-1);
  }
  return cmd;
}

// Oncelik (dusukten yuksege):  ;   <   && ||   <   |   <   < >
struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;
  cmd = parsecond(ps, es);
  if(peek(ps, es, ";")){
    gettoken(ps, es, 0, 0);
    cmd = listcmd(cmd, parseline(ps, es));
  }
  return cmd;
}

// a && b || c  ->  ((a && b) || c)   (bash gibi soldan saga)
struct cmd*
parsecond(char **ps, char *es)
{
  struct cmd *cmd;
  int tok;

  cmd = parsepipe(ps, es);
  while(peek(ps, es, "&|")){
    tok = gettoken(ps, es, 0, 0);
    if(tok != 'A' && tok != 'O'){
      fprintf(stderr, "syntax error: only && and || are supported\n\n");
      exit(-1);
    }
    cmd = condcmd(cmd, parsepipe(ps, es), tok);
  }
  return cmd;
}

// a | b | c  ->  a | (b | c)
struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|") && !((*ps)+1 < es && (*ps)[1] == '|')){   // "||" degil, tek "|"
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a') {
      fprintf(stderr, "missing file for redirection\n");
      exit(-1);
    }
    switch(tok){
    case '<':
      cmd = redircmd(cmd, mkcopy(q, eq), '<');
      break;
    case '>':
      cmd = redircmd(cmd, mkcopy(q, eq), '>');
      break;
    }
  }
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "&|;")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a') {
      fprintf(stderr, "syntax error\n");
      exit(-1);
    }
    cmd->argv[argc] = mkcopy(q, eq);
    argc++;
    if(argc >= MAXARGS) {
      fprintf(stderr, "too many args\n");
      exit(-1);
    }
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  return ret;
}
