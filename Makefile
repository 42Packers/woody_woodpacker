NAME	= woody_woodpacker

CC		=	gcc


SRCS_PATH	= ./sources/
SRCS	=	main.c \
		    woody.c

OBJS_PATH	= ./objects/
OBJS	=	$(addprefix $(OBJS_PATH), $(SRCS:.c=.o))
DEPS	= 	$(OBJS:.o=.d)


CFLAGS = -Wall -Wextra -Werror
CPPFLAGS = -MMD -c -g3

all: $(NAME)

$(NAME):	$(OBJS)
			$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJS_PATH)%.o:	$(SRCS_PATH)%.c
			@mkdir -p $(OBJS_PATH)
			$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

clean:
			rm -rf $(OBJS_PATH)

fclean:		clean
			rm -rf $(NAME)

re:			fclean
			$(MAKE) all

.PHONY: all, build, clean, fclean, re, libft

-include $(DEPS)
