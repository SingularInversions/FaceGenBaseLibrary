
/*
  Typedef declarations.
*/
typedef struct _DIR
{
  int
    d_VRefNum;
 
  long int
    d_DirID;
 
  int
    d_index;
} DIR;
 
struct dirent
{
  char
     d_name[255];
 
  int
    d_namlen;
};

/*
  Dirent utilities routines.
*/
extern char
  *mktemp(char *);

extern DIR
  *opendir(char *);

extern struct dirent
  *readdir(DIR *);

extern void
  closedir(DIR *);

