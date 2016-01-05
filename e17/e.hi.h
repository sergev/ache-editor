#define  LPARAM 20
#define  MAXHISTORY  22
typedef struct {
    char *p;
    Short l;
    Short len;
} S_cmd;
extern S_cmd *hcmd;
extern S_cmd ccmd;
extern Short nhistory;
