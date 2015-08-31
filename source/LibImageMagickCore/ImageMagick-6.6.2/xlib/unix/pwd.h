#ifdef __MWERKS__
#   include <types.h>
#else
#   include <sys/types.h>
#endif

struct passwd
{
  char  *pw_name;
  char  *pw_passwd;
  short int  pw_uid;
  short int  pw_gid;
  char  *pw_age;
  char  *pw_comment;
  char  *pw_gecos;
  char  *pw_dir;
  char  *pw_shell;
};

extern struct passwd
  *getpwnam(char *name);
