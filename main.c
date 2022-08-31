#include "ft_grep.h"

#define MAX_MATCH   100     // max matches in one line

#define F_MULTFILE  200
#define F_EMPTLINE  201


int exit_status = EXIT_SUCCESS;

typedef struct s_grep
{
    t_list      *patterns_list;
    t_list      *files;
    t_list      *patterns_path;
    char        *patterns;
    int         flags;  //eivclnhsfoE
    int         c_lines;
    int         c_matches;
}               t_grep;


void    exit_perror(char *msg, int status, int mask){
    if (msg && mask)
        perror(msg);
    exit(status);
}

void    *ft_allocate(size_t n){
    void    *out;

    if (n < 1)
        return (NULL);
    out = malloc(n);
    if (out == NULL)
        exit_perror("malloc: ", 2, 1);
    return (out);
}

void    exit_msg(char *msg, int status, int mask){
    if (msg && mask)
        ft_putstr_fd(msg, 2);
    exit(status);
}

void    print_error(char *msg, int status, int mask){
        if (msg && mask)
            perror(msg);
        exit_status = status;
}

void    init_grep(t_grep *arg)
{
    if (arg == NULL)
        return;
    arg->files = NULL;
    arg->patterns_path = NULL;
    arg->patterns_list = NULL;
    arg->patterns = NULL;
    arg->flags = 0;
    arg->c_lines = 1;
    arg->c_matches = 0;
}

int     check_flag(t_grep *arg, int flag)
{
    if (arg == NULL)
        return (0);
    switch (flag)
    {
    case 'e':
        return ( (arg->flags & 1) != 0);
    case 'i':
        return ( (arg->flags & 1<< 1) != 0);
    case 'v':
        return ( (arg->flags & 1<< 2) != 0);
    case 'c':
        return ( (arg->flags & 1<< 3) != 0);
    case 'l':
        return ( (arg->flags & 1<< 4) != 0);
    case 'n':
        return ( (arg->flags & 1<< 5) != 0);
    case 'h':
        return ( (arg->flags & 1<< 6) != 0);
    case 's':
        return ( (arg->flags & 1<< 7) != 0);
    case 'f':
        return ( (arg->flags & 1<< 8) != 0);
    case 'o':
        return ( (arg->flags & 1<< 9) != 0);
    case 'E':
        return ( (arg->flags & 1<< 10) != 0);
    case F_EMPTLINE:
        return ( (arg->flags & 1<< 11) != 0);
    case F_MULTFILE:
        return ( (arg->flags & 1<< 12) != 0);
    default:
        return (0);
    }
}

void    set_flag(t_grep *arg, int flag)
{
    if (arg == NULL)
        return ;
    switch (flag)
        {
        case 'e':
            arg->flags |= 1 << 0;
            break;
        case 'i':
            arg->flags |= 1 << 1;
            break;
        case 'v':
            arg->flags |= 1 << 2;
            break;
        case 'c':
            arg->flags |= 1 << 3;
            break;
        case 'l':
            arg->flags |= 1 << 4;
            break;
        case 'n':
            arg->flags |= 1 << 5;
            break;
        case 'h':
            arg->flags |= 1 << 6;
            break;
        case 's':
            arg->flags |= 1 << 7;
            break;
        case 'f':
            arg->flags |= 1 << 8;
            break;
        case 'o':
            arg->flags |= 1 << 9; 
            break;
        case 'E':
            arg->flags |= 1 << 10;
            break;
        case F_EMPTLINE:
            arg->flags |= 1 << 11;
            break;
        case F_MULTFILE:
            arg->flags |= 1 << 12;
            break;
        default:
            exit_msg("grep: invalid option\n", 2, 1);
    }
}

void    read_args(t_grep *arg, int argc, char **argv){
    int     i;

    if (!arg || !argv)
        return ;
    i = 1;
    // read the flags first
    while (i < argc && argv[i][0] == '-')
    {
        set_flag(arg, argv[i][1]);
        if (argv[i][2]){
            if (argv[i][1] == 'f')
                ft_lstadd_back(&(arg->patterns_path), ft_lstnew(ft_strdup(argv[i] + 2)));
            else if (argv[i][1] == 'e')
                ft_lstadd_back(&(arg->patterns_list), ft_lstnew(ft_strdup(argv[i] + 2)));
            else{
                arg->patterns = ft_strdup(argv[i] + 2);
                i++;
                break;
            }
        }else if (argv[i][1] == 'f'){
            if (++i == argc)
                exit_msg("grep: option requires an argument -- 'f'\n", 2, 1);
            ft_lstadd_back(&(arg->patterns_path), ft_lstnew(ft_strdup(argv[i])));
        }else if (argv[i][1] == 'e'){
            if (++i == argc)
                exit_msg("grep: option requires an argument -- 'e'\n", 2, 1);
           ft_lstadd_back(&(arg->patterns_list), ft_lstnew(ft_strdup(argv[i]))); 
        }
        i++;
    }

    // read patterns if NULL
    if (!arg->patterns && !arg->patterns_path && !arg->patterns_list && i < argc)
        arg->patterns = ft_strdup(argv[i++]);
    
    // save the rest as files
    while (i < argc)
        ft_lstadd_back(&(arg->files), ft_lstnew(ft_strdup(argv[i++])));
    
    if (!arg->patterns && !arg->patterns_path && !arg->patterns_list)
        exit_msg("Usage: grep [OPTION]... PATTERNS [FILE]...\n", 2, 1);
    
    // set flag X to 1 if there are multiple files and not flag -h
    if (arg->files && arg->files->next && !check_flag(arg, 'h'))
        set_flag(arg, F_MULTFILE);
}

void    str_tolower(char *str){
    if (str == NULL)
        return ;
    while (*str){
        *str = ft_tolower(*str);
        str++;
    }
}

// TODO: check empty lines in patterns file

void    split_patterns(t_grep *arg){
    char    **spt;
    char    **p;
    char    *line;
    int     fd, empty = 0;
    t_list  *tmp;

    if (arg == NULL)
        return ;
    if (arg->patterns)
    {
        if (check_flag(arg, 'i'))
            str_tolower(arg->patterns);
        spt = ft_split(arg->patterns, '\n');
        p = spt;
        while(*p)
            ft_lstadd_back(&(arg->patterns_list), ft_lstnew(*p++));
        free(spt);
    }else if (arg->patterns_path)
    {
        tmp = arg->patterns_path;
        while (tmp){
            if ((fd = open(tmp->content, O_RDONLY)) < 0)
                exit_perror("error reading patterns file:", 2, check_flag(arg, 's') == 0);
            while ((line = ft_get_next_line(fd))){
                if (empty == 1 && empty++)
                    set_flag(arg, F_EMPTLINE);
                if (*line == 0)
                {
                    free(line);
                    empty++;
                    continue;
                }
                ft_lstadd_back(&(arg->patterns_list), ft_lstnew(line));
            }
            close(fd);
            tmp = tmp->next;
        }
    }
}

void    compile_patterns(t_grep *arg){

    regex_t *re;
    t_list  *p;
    int     f;

    if (arg == NULL)
        return ;
    f =  check_flag(arg, 'i') ? REG_ICASE : 0;
    f |= check_flag(arg, 'E') ? REG_EXTENDED : 0;
    p = arg->patterns_list;
    while (p)
    {
        re = malloc(sizeof(regex_t));
        if (! re)
            exit_perror("allocation t_regex: ", 2, 1);
        if (regcomp(re, p->content, f) != 0)
            exit_perror("regcomp error: ", 2, 1);
        free(p->content);
        p->content = re;
        p = p->next;
    }
}

int     match(t_grep *arg, char *str, regmatch_t pmatch[], t_list **matches){
    t_list      *p;
    regmatch_t  min, *tmp;
    int         out;

    if (!arg || !arg->patterns_list || !str)
        return (0);
    out = 0;
    while (*str)
    {
        p = arg->patterns_list;
        min.rm_so = __INT32_MAX__;
        while (p)
        {
            
            if (regexec(p->content, str, 1, pmatch, 0) == 0)
            {
                out = 1;
                if (check_flag(arg, 'l'))
                    return (1);
                if (pmatch[0].rm_so != -1 && pmatch[0].rm_so < min.rm_so)
                    ft_memcpy(&min, &pmatch[0], sizeof(regmatch_t));
                else if (pmatch[0].rm_so != -1 && pmatch[0].rm_so == min.rm_so && pmatch[0].rm_eo > min.rm_eo)
                    ft_memcpy(&min, &pmatch[0], sizeof(regmatch_t));
            }
            p = p->next;
        }
        if (min.rm_so != __INT32_MAX__){
            tmp = ft_allocate(sizeof(regmatch_t));
            ft_memcpy(tmp, &min, sizeof(regmatch_t));
            ft_lstadd_back(matches, ft_lstnew(tmp));
            str += min.rm_eo;
        }else
            break;
        
    }
    return (out);
}

void   print_matches(t_grep *arg, char *str, t_list *matches, char *file_path){

    (void) arg;
    (void) file_path;
    if (str == NULL)
        return ;

    t_list *p = matches;
    while (p)
    {
        regmatch_t *rm = p->content;
        for (int i = 0; i < rm->rm_so && *str; i++)
            putc(*str++, stdout);
        fputs(S21_RED, stdout);
        for (int i = rm->rm_so; i < rm->rm_eo && *str; i++)
            putc(*str++, stdout);
        fputs(S21_WHITE, stdout);
        p = p->next;
    }
    if (*str)
        printf("%s", str);
    putc('\n', stdout);
}

int    match_file(t_grep *arg, char *file_path){
    int         fd;
    char        *line;
    regmatch_t  pmatch[2];
    t_list      *matches;

    if( arg == NULL)
        return (0);
    if (file_path == NULL || ft_strncmp(file_path, "-", ft_strlen(file_path)) == 0 || \
        ft_strncmp(file_path, "--", ft_strlen(file_path)) == 0)
    {
        fd = 0;
        file_path = "(standard input)";
    }
    else if ((fd = open(file_path, O_RDONLY)) < 0)
    {
        print_error("open file_path error: ", 2, check_flag(arg, 's') == 0);
        return (0);
    }

    matches = NULL;
    while ((line = ft_get_next_line(fd)) != NULL){
        if (match(arg, line, pmatch, &matches) != check_flag(arg, 'v')){ // logical XOR
            arg->c_matches++;
            if (check_flag(arg, F_MULTFILE) && !check_flag(arg, 'l'))
                printf("%s%s%s:%s", S21_MAGENTA, file_path, S21_CYAN, S21_WHITE);
            if (check_flag(arg, 'n') && !check_flag(arg, 'l'))
                printf("%s%d%s:%s", S21_GREEN, arg->c_lines, S21_CYAN, S21_WHITE);
            if (!check_flag(arg, 'c') && !check_flag(arg, 'l'))
                print_matches(arg, line, matches, file_path);
            if (check_flag(arg, 'l')){
                printf("%s%s%s\n", S21_MAGENTA ,file_path, S21_WHITE);
                free(line);
                break;
            }
        }
        free(line);
        ft_lstclear(&matches, free);
        arg->c_lines++;
    }
    if (check_flag(arg, 'c') && arg->c_lines){
        if (check_flag(arg, F_MULTFILE))
            printf("%s%s%s:%s", S21_MAGENTA, file_path, S21_CYAN, S21_WHITE);
        printf("%d\n", arg->c_matches);
    }

    if (fd)
        close(fd);
    return (arg->c_matches);
}

void    match_files_list(t_grep *arg){
    t_list  *p;

    if (arg == NULL)
        return ;
    p = arg->files;
    if (p == NULL)
        match_file(arg, NULL);
    while (p)
    {
        arg->c_lines = 1;
        arg->c_matches = 0;
        match_file(arg, p->content);
        p = p->next;
    }
} 

void    free_split(char ***arr){
    char **p;

    if (!arr || !*arr)
        return ;
    p = *arr;
    while (p)
        if (*p)
            free(*p++);
    free(*arr);
}

void    free_reg(void *re){
    if (re){
        regfree(re);
        free(re);
    }
}

void    free_args(t_grep *arg){
    if (arg == NULL)
        return ;
    if (arg->patterns)
        free(arg->patterns);
    if (arg->patterns_path)
        ft_lstclear(&(arg->patterns_path), free);
    if (arg->files)
        ft_lstclear(&(arg->files), free);
    if (arg->patterns_list)
        ft_lstclear(&(arg->patterns_list), free_reg);
}

void    print_args(t_grep *arg){

    t_list  *p;

    ft_putnbr_base_fd(arg->flags, "01", 1);

    printf("\npatterns : %s\n", arg->patterns);

    printf("patterns_path :\n");
    p = arg->patterns_path;
    while (p)
    {
        printf("\t%s\n", (char *) p->content);
        p = p->next;
    }

    printf("files : \n");
    p = arg->files;
    while (p)
    {
        printf("\t%s\n", (char *) p->content);
        p = p->next;
    }

    printf("patterns_list :\n");
    p = arg->patterns_list;
    while (p)
    {
        printf("\t%s\n", (char *) p->content);
        p = p->next;
    }
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    t_grep data;

    init_grep(&data);

    read_args(&data, argc, argv);

    split_patterns(&data);

    //print_args(&data);

    compile_patterns(&data);

    match_files_list(&data);

    free_args(&data);

    return exit_status;
}

int main22()
{
   int i = 0;
   int res;
   int len;
   char result[BUFSIZ];
   char err_buf[BUFSIZ];
   char* src = "hello world";  

   const char* pattern = "(\\w+) (\\w+)";
   regex_t preg;

   regmatch_t pmatch[10];

   if( (res = regcomp(&preg, pattern, REG_EXTENDED)) != 0)
   {
      regerror(res, &preg, err_buf, BUFSIZ);
      printf("regcomp: %s\n", err_buf);
      exit(res);
   }

   res = regexec(&preg, src, 10, pmatch, REG_NOTBOL);
   //~ res = regexec(&preg, src, 10, pmatch, 0);
   //~ res = regexec(&preg, src, 10, pmatch, REG_NOTEOL);
   if(res == REG_NOMATCH)
   {
      printf("NO match\n");
      exit(0);
   }
   for (i = 0; pmatch[i].rm_so != -1; i++)
   {
      len = pmatch[i].rm_eo - pmatch[i].rm_so;
      memcpy(result, src + pmatch[i].rm_so, len);
      result[len] = 0;
      printf("num %d: '%s'\n", i, result);
   }
   regfree(&preg);
   return 0;
}



// main testing main again test main main mmnmainnnm