# /*************************************************************************
#  *                            Makefile                               *
#  * Authors:                                                              *
#  * Edwar Yepez              12-10855                                     *
#  * Maria Fernanda Machado   13-10780                                     *
#  * Veronica Mazutiel        13-10853                                     *
#  *                                                                       *
#  *************************************************************************/

CFLAGS	= -pthread
CC	= gcc
OBJS= svr_s.o svr_c.o

svr_s.o: $(OBJS)
		$(CC) $(CFLAGS) svr_s.c -o svr_s

svr_c.o: $(OBJS)
		$(CC) $(CFLAGS) svr_c.c -o svr_c

clean:
		/bin/rm svr_s svr_c