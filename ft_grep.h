#ifndef __FT_GREP_H__
#define __FT_GREP_H__


#ifndef __S_LIST__
#define __S_LIST__
typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}				t_list;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include "gnl/get_next_line.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>



#define S21_WHITE   "\x1b[0m"
#define S21_RED     "\x1b[31m"
#define S21_GREEN   "\x1b[32m"
#define S21_YELLOW  "\x1b[33m"
#define S21_BLUE    "\x1b[34m"
#define S21_MAGENTA "\x1b[35m"
#define S21_CYAN    "\x1b[36m"



int		ft_lstsize(t_list *lst);
t_list	*ft_lstnew(void *content);
void	ft_lstclear(t_list **lst, void (*del)(void*));
void	ft_lstadd_back(t_list **lst, t_list *new);
char	**ft_split(char const *s, char c);
size_t	ft_strlen(const char *s);
char	*ft_strdup(const char *s1);
int		ft_strncmp(const char *s1, const char *s2, size_t n);
char	*ft_strnstr(const char *haystack, const char *needle, size_t len);
int		ft_tolower(int c);
void	*ft_memcpy(void *dst, const void *src, size_t n);
char	*ft_strchr(const char *s, int c);
void	ft_putstr_fd(char *s, int fd);
void	ft_putnbr_base_fd(int nb, const char *base, int fd);
void	*ft_memset(void *b, int c, size_t len);




#endif //__FT_GREP_H__