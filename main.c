#include "ft_grep.h"


typedef struct s_grep
{
    t_list      *patterns_list;
    t_list      *files;
    t_list      *patterns_path;
    char        *patterns;
    int         flags;  //eivclnhsfoX    // X for multiple files
}               t_grep;

void    exit_perror(char *msg, int status){
    if (msg)
        perror(msg);
    exit(status);
}

void    exit_msg(char *msg, int status){
    if (msg)
        ft_putstr_fd(msg, 2);
    exit(status);
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
}

int     check_flag(t_grep *arg, char flag)
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
    case 'X':
        return ( (arg->flags & 1<< 10) != 0);
    default:
        return (0);
    }
}

void    set_flag(t_grep *arg, char flag)
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
        case 'X':
            arg->flags |= 1 << 10;
            break;
        default:
            exit_msg("grep: invalid option\n", 2);
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
                exit_msg("grep: option requires an argument -- 'f'\n", 2);
            ft_lstadd_back(&(arg->patterns_path), ft_lstnew(ft_strdup(argv[i])));
        }else if (argv[i][1] == 'e'){
            if (++i == argc)
                exit_msg("grep: option requires an argument -- 'e'\n", 2);
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
        exit_msg("Usage: grep [OPTION]... PATTERNS [FILE]...\n", 2);
    
    // set flag X to 1 if there are multiple files and not flag -h
    if (arg->files && arg->files->next && !check_flag(arg, 'h'))
        set_flag(arg, 'X');
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
    int     fd;
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
                exit_perror("error reading patterns file !\n", 2);
            while ((line = ft_get_next_line(fd))){
                // if (*line == 0)
                // {
                //     free(line);
                //     continue;
                // }
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
    f = check_flag(arg, 'i') == 1 ? REG_ICASE : 0;
    p = arg->patterns_list;
    while (p)
    {
        re = malloc(sizeof(regex_t));
        if (! re)
            exit_perror("allocation t_regex: ", 2);
        if (regcomp(re, p->content, f | REG_EXTENDED) != 0)
            exit_perror("regcomp error: ", 2);
        free(p->content);
        p->content = re;
        p = p->next;
    }
}

int     match(t_grep *arg, char *str){
    t_list  *p;

    if (!arg || !arg->patterns_list || !str)
        return (0);

    p = arg->patterns_list;
    while (p)
    {
        if (regexec(p->content, str, 0, NULL, 0) == 0)
            return (1);
        p = p->next;
    }
    return (0);
}

int    match_file(t_grep *arg, char *file_path){
    int     fd;
    char    *line;
    int     count;
    int     l;

    if( arg == NULL)
        return 0;
    if (file_path){
        if ((fd = open(file_path, O_RDONLY)) < 0)
            exit_perror("open file_path error: ", 2);
    }
    else
        fd = 0;
    
    count = 0;
    l = 1;
    while ((line = ft_get_next_line(fd)) != NULL){
        if (match(arg, line) != check_flag(arg, 'v')){ // logical XOR
            // put the outputing logic
            if (check_flag(arg, 'X') && !check_flag(arg, 'l'))
                printf("%s:", file_path);
            if (check_flag(arg, 'n') && !check_flag(arg, 'l'))
                printf("%d:", l);
            if (!check_flag(arg, 'c') && !check_flag(arg, 'l'))
                printf("%s\n", line);
            count++;
        }
        free(line);
        l++;
    }
    if (check_flag(arg, 'l') && count){
        printf("%s\n", file_path);
    }else if (check_flag(arg, 'c') && count){
        if (check_flag(arg, 'X'))
            printf("%s:", file_path);
        printf("%d\n", count);
    }

    if (fd)
        close(fd);
    return (count);
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

    print_args(&data);

    compile_patterns(&data);

    match_files_list(&data);

    free_args(&data);

    return 0;
}
