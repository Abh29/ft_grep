#include "../ft_grep.h"

int	ft_tolower(int c)
{
	if (c > 64 && c < 91)
		return ((unsigned char)(c + 32));
	return (c);
}
