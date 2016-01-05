extern int HelpItems;

typedef struct {
    char *item_name;
    ANlines def_line;
    ANcols def_col;
    short item_len;
} S_help;

typedef struct hpage {
    struct hpage *item_next;
    short item_num;
    ANlines item_line;
    ANcols item_col;
} S_hpage;

extern S_hpage *HelpPage;

extern S_help *Item;

#define HELPIDLEN 4
