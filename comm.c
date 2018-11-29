/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
*  Any remaining bugs are, of course, our work, not his.  :)              *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
***************************************************************************/

/*
* This file contains all of the OS-dependent stuff:
*   startup, signals, BSD sockets for tcp/ip, i/o, timing.
*
* The data flow for input is:
*    Game_loop ---> Read_from_descriptor ---> Read
*    Game_loop ---> Read_from_buffer
*
* The data flow for output is:
*    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
*
* The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
* -- Furey  26 Jan 1993
*/

#define _XOPEN_SOURCE

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include <unistd.h>

/*
* Malloc debugging stuff.
*/
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern int malloc_debug args((int));
extern int malloc_verify args((void));
#endif

/*
* Signal handling.
* Apollo has a problem with __attribute(atomic) in signal.h,
*   I dance around it.
*/
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif

/*
* Socket and TCP/IP stuff.
*/
#if defined(macintosh) || defined(MSDOS)
const char echo_off_str[] = {'\0'};
const char echo_on_str[] = {'\0'};
const char go_ahead_str[] = {'\0'};
#endif

#if defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const char echo_off_str[] = {IAC, WILL, TELOPT_ECHO, '\0'};
const char echo_on_str[] = {IAC, WONT, TELOPT_ECHO, '\0'};
const char go_ahead_str[] = {IAC, GA, '\0'};
#endif

/*
* OS-dependent declarations.
*/
#if defined(_AIX)
#include <sys/select.h>
int accept args((int s, struct sockaddr *addr, int *addrlen));
int bind args((int s, struct sockaddr *name, int namelen));
void bzero args((char *b, int length));
int getpeername args((int s, struct sockaddr *name, int *namelen));
int getsockname args((int s, struct sockaddr *name, int *namelen));
int gettimeofday args((struct timeval * tp, struct timezone *tzp));
int listen args((int s, int backlog));
int setsockopt args((int s, int level, int optname, void *optval,
					 int optlen));
int socket args((int domain, int type, int protocol));
#endif

#if defined(apollo)
#include <unistd.h>
void bzero args((char *b, int length));
#endif

#if defined(__hpux)
int accept args((int s, void *addr, int *addrlen));
int bind args((int s, const void *addr, int addrlen));
void bzero args((char *b, int length));
int getpeername args((int s, void *addr, int *addrlen));
int getsockname args((int s, void *name, int *addrlen));
int gettimeofday args((struct timeval * tp, struct timezone *tzp));
int listen args((int s, int backlog));
int setsockopt args((int s, int level, int optname,
					 const void *optval, int optlen));
int socket args((int domain, int type, int protocol));
#endif

#if defined(interactive)
#include <net/errno.h>
#include <sys/fcntl.h>
#endif

#if defined(linux)
/*
int	accept		args( ( int s, const struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, const struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
*/
#endif

#if defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct timeval
{
	time_t tv_sec;
	time_t tv_usec;
};
#if !defined(isascii)
#define isascii(c) ((c) < 0200)
#endif
static long theKeys[4];

int gettimeofday args((struct timeval * tp, void *tzp));
#endif

#if defined(MIPS_OS)
extern int errno;
#endif

#if defined(MSDOS)
int gettimeofday args((struct timeval * tp, void *tzp));
int kbhit args((void));
#endif

#if defined(NeXT)
int close args((int fd));
int fcntl args((int fd, int cmd, int arg));
#if !defined(htons)
u_short htons args((u_short hostshort));
#endif
#if !defined(ntohl)
u_long ntohl args((u_long hostlong));
#endif
int read args((int fd, char *buf, int nbyte));
int select args((int width, fd_set *readfds, fd_set *writefds,
				 fd_set *exceptfds, struct timeval *timeout));
int write args((int fd, char *buf, int nbyte));
#endif

#if defined(sequent)
int accept args((int s, struct sockaddr *addr, int *addrlen));
int bind args((int s, struct sockaddr *name, int namelen));
int close args((int fd));
int fcntl args((int fd, int cmd, int arg));
int getpeername args((int s, struct sockaddr *name, int *namelen));
int getsockname args((int s, struct sockaddr *name, int *namelen));
int gettimeofday args((struct timeval * tp, struct timezone *tzp));
#if !defined(htons)
u_short htons args((u_short hostshort));
#endif
int listen args((int s, int backlog));
#if !defined(ntohl)
u_long ntohl args((u_long hostlong));
#endif
int read args((int fd, char *buf, int nbyte));
int select args((int width, fd_set *readfds, fd_set *writefds,
				 fd_set *exceptfds, struct timeval *timeout));
int setsockopt args((int s, int level, int optname, caddr_t optval,
					 int optlen));
int socket args((int domain, int type, int protocol));
int write args((int fd, char *buf, int nbyte));
#endif

/*
* This includes Solaris SYSV as well.
*/
#if defined(sun)
int accept args((int s, struct sockaddr *addr, int *addrlen));
int bind args((int s, struct sockaddr *name, int namelen));
void bzero args((char *b, int length));
int close args((int fd));
int getpeername args((int s, struct sockaddr *name, int *namelen));
int getsockname args((int s, struct sockaddr *name, int *namelen));
int gettimeofday args((struct timeval * tp, struct timezone *tzp));
int listen args((int s, int backlog));
int read args((int fd, char *buf, int nbyte));
int select args((int width, fd_set *readfds, fd_set *writefds,
				 fd_set *exceptfds, struct timeval *timeout));

#if defined(SYSV)
int setsockopt args((int s, int level, int optname,
					 const char *optval, int optlen));
#else
int setsockopt args((int s, int level, int optname, void *optval,
					 int optlen));
#endif
int socket args((int domain, int type, int protocol));
int write args((int fd, char *buf, int nbyte));
#endif

#if defined(ultrix)
int accept args((int s, struct sockaddr *addr, int *addrlen));
int bind args((int s, struct sockaddr *name, int namelen));
void bzero args((char *b, int length));
int close args((int fd));
int getpeername args((int s, struct sockaddr *name, int *namelen));
int getsockname args((int s, struct sockaddr *name, int *namelen));
int gettimeofday args((struct timeval * tp, struct timezone *tzp));
int listen args((int s, int backlog));
int read args((int fd, char *buf, int nbyte));
int select args((int width, fd_set *readfds, fd_set *writefds,
				 fd_set *exceptfds, struct timeval *timeout));
int setsockopt args((int s, int level, int optname, void *optval,
					 int optlen));
int socket args((int domain, int type, int protocol));
int write args((int fd, char *buf, int nbyte));
#endif

/*
* Global variables.
*/
DESCRIPTOR_DATA *descriptor_free; /* Free list for descriptors	*/
DESCRIPTOR_DATA *descriptor_list; /* All open descriptors		*/
DESCRIPTOR_DATA *d_next;		  /* Next descriptor in loop	*/
FILE *fpReserve;				  /* Reserved file handle		*/
bool god;						  /* All new chars are gods!	*/
bool merc_down;					  /* Shutdown			*/
bool wizlock;					  /* Game is wizlocked		*/
char str_boot_time[MAX_INPUT_LENGTH];
time_t current_time; /* Time of this pulse		*/

/* Colour scale char list - Calamar */

char *scale[SCALE_COLS] = {
	LIGHTRED,
	LIGHTBLUE,
	LIGHTGREEN,
	YELLOW};

/*
* OS-dependent local functions.
*/
#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos args((void));
bool read_from_descriptor args((DESCRIPTOR_DATA * d));
bool write_to_descriptor args((int desc, char *txt, int length));
#endif

#if defined(unix)
void game_loop_unix args((int control));
int init_socket args((int port));
void new_descriptor args((int control));
bool read_from_descriptor args((DESCRIPTOR_DATA * d));
bool write_to_descriptor args((int desc, char *txt, int length));
#endif

/*
* Other local functions (OS-independent).
*/
bool check_parse_name args((char *name));
bool check_reconnect args((DESCRIPTOR_DATA * d, char *name,
						   bool fConn));
bool check_kickoff args((DESCRIPTOR_DATA * d, char *name,
						 bool fConn));
bool check_playing args((DESCRIPTOR_DATA * d, char *name));
int main args((int argc, char **argv));
void nanny args((DESCRIPTOR_DATA * d, char *argument));
bool process_output args((DESCRIPTOR_DATA * d, bool fPrompt));
void read_from_buffer args((DESCRIPTOR_DATA * d));
void stop_idling args((CHAR_DATA * ch));
int port;

void add_to_history args((CHANNEL_DATA * channel_history, char *information));

#if defined(unix)
int control;
#endif

int main(int argc, char **argv)
{
	struct timeval now_time;
	bool fCopyOver = FALSE;

#if defined(unix)
	signal(SIGPIPE, SIG_IGN);
#endif

	/*
    * Memory debugging if needed.
    */
#if defined(MALLOC_DEBUG)
	malloc_debug(2);
#endif

	/*
    * Init time.
    */
	gettimeofday(&now_time, NULL);
	current_time = (time_t)now_time.tv_sec;
	strncpy(str_boot_time, ctime(&current_time), MAX_INPUT_LENGTH);

	/*
    * Macintosh console initialization.
    */
#if defined(macintosh)
	console_options.nrows = 31;
	cshow(stdout);
	csetmode(C_RAW, stdin);
	cecho2file("log file", 1, stderr);
#endif

	/*
    * Reserve one channel for our use.
    */
	if ((fpReserve = fopen(NULL_FILE, "r")) == NULL)
	{
		perror(NULL_FILE);
		exit(1);
	}

	/*
    * Get the port number.
    */
	port = 4000;
	if (argc > 1)
	{
		if (argv[2] && argv[2][0])
		{
			fCopyOver = TRUE;
			control = atoi(argv[3]);
		}
		else
			fCopyOver = FALSE;

		if (!is_number(argv[1]))
		{
			fprintf(stderr, "Usage: %s [port #]\n", argv[0]);
			exit(1);
		}
		else if ((port = atoi(argv[1])) <= 1024)
		{
			fprintf(stderr, "Port number must be above 1024.\n");
			exit(1);
		}
	}

	/*
    * Run the game.
    */
#if defined(macintosh) || defined(MSDOS)
	boot_db(fCopyOver);
	log_string("Vampire Wars is ready to rock.");
	game_loop_mac_msdos();
#endif

#if defined(unix)
	if (!fCopyOver) /* We have already the port if copyover'ed */
		control = init_socket(port);
	boot_db(fCopyOver);
	snprintf(log_buf, MAX_INPUT_LENGTH * 2, "Vampire Wars is ready to rock on port %d.", port);
	log_string(log_buf);
	bug("Crash/Reboot.", 0);
	game_loop_unix(control);
	close(control);
#endif

	/*
    * That's all, folks.
    */
	log_string("Normal termination of game.");
	exit(0);
	return 0;
}

#if defined(unix)
int init_socket(int port)
{
	static struct sockaddr_in sa_zero;
	struct sockaddr_in sa;
	int x;
	int fd;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Init_socket: socket");
		exit(1);
	}

	x = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
				   (char *)&x, sizeof(x)) < 0)
	{
		perror("Init_socket: SO_REUSEADDR");
		close(fd);
		exit(1);
	}

#if defined(SO_DONTLINGER) && !defined(SYSV)
	{
		struct linger ld;

		ld.l_onoff = 1;
		ld.l_linger = 1000;

		if (setsockopt(fd, SOL_SOCKET, SO_DONTLINGER,
					   (char *)&ld, sizeof(ld)) < 0)
		{
			perror("Init_socket: SO_DONTLINGER");
			close(fd);
			exit(1);
		}
	}
#endif

	sa = sa_zero;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
	{
		perror("Init_socket: bind");
		close(fd);
		exit(1);
	}

	if (listen(fd, 3) < 0)
	{
		perror("Init_socket: listen");
		close(fd);
		exit(1);
	}

	return fd;
}
#endif

#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos(void)
{
	struct timeval last_time;
	struct timeval now_time;
	static DESCRIPTOR_DATA dcon;

	gettimeofday(&last_time, NULL);
	current_time = (time_t)last_time.tv_sec;

	/*
    * New_descriptor analogue.
    */
	dcon.descriptor = 0;
	dcon.connected = CON_GET_NAME;
	dcon.host = str_dup("localhost");
	dcon.outsize = 2000;
	dcon.outbuf = alloc_mem(dcon.outsize);
	dcon.next = descriptor_list;
	descriptor_list = &dcon;

	/*
    * Send the greeting.
    */
	{
		extern char *help_greeting;
		if (help_greeting[0] == '.')
			write_to_buffer(&dcon, help_greeting + 1, 0, 0);
		else
			write_to_buffer(&dcon, help_greeting, 0, 0);
	}

	/* Main loop */
	while (!merc_down)
	{
		DESCRIPTOR_DATA *d;

		/*
	   * Process input.
	   */
		for (d = descriptor_list; d != NULL; d = d_next)
		{
			d_next = d->next;
			d->fcommand = FALSE;

#if defined(MSDOS)
			if (kbhit())
#endif
			{
				if (d->character != NULL)
					d->character->timer = 0;
				if (!read_from_descriptor(d))
				{
					if (d->character != NULL)
						save_char_obj(d->character);
					d->outtop = 0;
					close_socket(d);
					continue;
				}
			}

			if (d->character != NULL && d->character->wait > 0)
			{
				--d->character->wait;
				continue;
			}

			read_from_buffer(d);
			if (d->incomm[0] != '\0')
			{
				d->fcommand = TRUE;
				stop_idling(d->character);

				if (d->connected == CON_PLAYING)
					interpret(d->character, d->incomm);
				else
					nanny(d, d->incomm);

				d->incomm[0] = '\0';
			}
		}

		/*
	   * Autonomous game motion.
	   */
		update_handler();

		/*
	   * Output.
	   */
		for (d = descriptor_list; d != NULL; d = d_next)
		{
			d_next = d->next;

			if ((d->fcommand || d->outtop > 0))
			{
				if (!process_output(d, TRUE))
				{
					if (d->character != NULL)
						save_char_obj(d->character);
					d->outtop = 0;
					close_socket(d);
				}
			}
		}

		/*
	   * Synchronize to a clock.
	   * Busy wait (blargh).
	   */
		now_time = last_time;
		for (;;)
		{
			int delta;

#if defined(MSDOS)
			if (kbhit())
#endif
			{
				if (dcon.character != NULL)
					dcon.character->timer = 0;
				if (!read_from_descriptor(&dcon))
				{
					if (dcon.character != NULL)
						save_char_obj(d->character);
					dcon.outtop = 0;
					close_socket(&dcon);
				}
#if defined(MSDOS)
				break;
#endif
			}

			gettimeofday(&now_time, NULL);
			delta = (now_time.tv_sec - last_time.tv_sec) * 1000 * 1000 + (now_time.tv_usec - last_time.tv_usec);
			if (delta >= 1000000 / PULSE_PER_SECOND)
				break;
		}
		last_time = now_time;
		current_time = (time_t)last_time.tv_sec;
	}

	return;
}
#endif

#if defined(unix)

void excessive_cpu(int blx)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for (vch = char_list; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next;

		if (!IS_NPC(vch))
		{
			send_to_char("Mud frozen: Autosave and quit.  The mud will reboot in 2 seconds.\n\r", vch);
			interpret(vch, "quit");
		}
	}
	exit(1);
}

void game_loop_unix(int control)
{
	static struct timeval null_time;
	struct timeval last_time;
	signal(SIGPIPE, SIG_IGN);
	signal(SIGXCPU, excessive_cpu);
	gettimeofday(&last_time, NULL);
	current_time = (time_t)last_time.tv_sec;

	/* Main loop */
	while (!merc_down)
	{
		fd_set in_set;
		fd_set out_set;
		fd_set exc_set;
		DESCRIPTOR_DATA *d;
		int maxdesc;

#if defined(MALLOC_DEBUG)
		if (malloc_verify() != 1)
			abort();
#endif

		/*
		  * Poll all active descriptors.
	   */
		FD_ZERO(&in_set);
		FD_ZERO(&out_set);
		FD_ZERO(&exc_set);
		FD_SET(control, &in_set);
		maxdesc = control;
		for (d = descriptor_list; d; d = d->next)
		{
			maxdesc = UMAX(maxdesc, d->descriptor);
			FD_SET(d->descriptor, &in_set);
			FD_SET(d->descriptor, &out_set);
			FD_SET(d->descriptor, &exc_set);
		}

		if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0)
		{
			perror("Game_loop: select: poll");
			exit(1);
		}

		/*
	   * New connection?
	   */
		if (FD_ISSET(control, &in_set))
			new_descriptor(control);

		/*
		  * Kick out the freaky folks.
	   */
		for (d = descriptor_list; d != NULL; d = d_next)
		{
			d_next = d->next;
			if (FD_ISSET(d->descriptor, &exc_set))
			{
				FD_CLR(d->descriptor, &in_set);
				FD_CLR(d->descriptor, &out_set);
				if (d->character)
					save_char_obj(d->character);
				d->outtop = 0;
				close_socket(d);
			}
		}

		/*
	   * Process input.
	   */
		for (d = descriptor_list; d != NULL; d = d_next)
		{
			d_next = d->next;
			d->fcommand = FALSE;

			if (FD_ISSET(d->descriptor, &in_set))
			{
				if (d->character != NULL)
					d->character->timer = 0;
				if (!read_from_descriptor(d))
				{
					FD_CLR(d->descriptor, &out_set);
					if (d->character != NULL)
						save_char_obj(d->character);
					d->outtop = 0;
					close_socket(d);
					continue;
				}
			}

			if (current_time % 300 == 0)
			{
				save_claninfo();
			}

			if (d->character != NULL && d->character->wait > 0)
			{
				--d->character->wait;
				continue;
			}

			read_from_buffer(d);
			if (d->incomm[0] != '\0')
			{
				d->fcommand = TRUE;
				stop_idling(d->character);

				if (d->connected == CON_PLAYING)
				{
					interpret(d->character, d->incomm);
				}
				else
				{
					nanny(d, d->incomm);
				}

				d->incomm[0] = '\0';
			}

			if ((d->connected == CON_GET_NAME || d->connected == CON_GET_OLD_PASSWORD) && (current_time - d->connect_time) > 60)
			{
				close_socket(d);
			}
		}

		/*
	   * Autonomous game motion.
	   */
		update_handler();

		/*
	   * Output.
	   */
		for (d = descriptor_list; d != NULL; d = d_next)
		{
			d_next = d->next;

			if ((d->fcommand || d->outtop > 0) && FD_ISSET(d->descriptor, &out_set))
			{
				if (!process_output(d, TRUE))
				{
					if (d->character != NULL)
						save_char_obj(d->character);
					d->outtop = 0;
					close_socket(d);
				}
			}
		}

		/*
	   * Synchronize to a clock.
	   * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	   * Careful here of signed versus unsigned arithmetic.
	   */
		{
			struct timeval now_time;
			long secDelta;
			long usecDelta;

			gettimeofday(&now_time, NULL);
			usecDelta = ((int)last_time.tv_usec) - ((int)now_time.tv_usec) + 1000000 / PULSE_PER_SECOND;
			secDelta = ((int)last_time.tv_sec) - ((int)now_time.tv_sec);
			while (usecDelta < 0)
			{
				usecDelta += 1000000;
				secDelta -= 1;
			}

			while (usecDelta >= 1000000)
			{
				usecDelta -= 1000000;
				secDelta += 1;
			}

			if (secDelta > 0 || (secDelta == 0 && usecDelta > 0))
			{
				struct timeval stall_time;

				stall_time.tv_usec = usecDelta;
				stall_time.tv_sec = secDelta;
				if (select(0, NULL, NULL, NULL, &stall_time) < 0)
				{
					perror("Game_loop: select: stall");
					exit(1);
				}
			}
		}

		gettimeofday(&last_time, NULL);
		current_time = (time_t)last_time.tv_sec;
	}

	return;
}
#endif

void init_descriptor(DESCRIPTOR_DATA *dnew, int desc)
{
	static DESCRIPTOR_DATA d_zero;

	*dnew = d_zero;
	dnew->descriptor = desc;
	dnew->connected = CON_GET_NAME;
	dnew->outsize = 2000;
	dnew->outbuf = alloc_mem(dnew->outsize);
	dnew->connect_time = current_time;
}

#if defined(unix)
void new_descriptor(int control)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *dnew;
	BAN_DATA *pban;
	struct sockaddr_in sock;
	struct hostent *from;
	int desc;
	socklen_t size;

	size = sizeof(sock);
	getsockname(control, (struct sockaddr *)&sock, &size);
	if ((desc = accept(control, (struct sockaddr *)&sock, &size)) < 0)
	{
		perror("New_descriptor: accept");
		return;
	}

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

	if (fcntl(desc, F_SETFL, FNDELAY) == -1)
	{
		perror("New_descriptor: fcntl: FNDELAY");
		return;
	}

	/*
    * Cons a new descriptor.
    */
	if (descriptor_free == NULL)
	{
		dnew = alloc_perm(sizeof(*dnew));
	}
	else
	{
		dnew = descriptor_free;
		descriptor_free = descriptor_free->next;
	}

	init_descriptor(dnew, desc);

	size = sizeof(sock);
	if (getpeername(desc, (struct sockaddr *)&sock, &size) < 0)
	{
		perror("New_descriptor: getpeername");
		dnew->host = str_dup("(unknown)");
	}
	else
	{
		/*
    * Would be nice to use inet_ntoa here but it takes a struct arg,
    * which ain't very compatible between gcc and system libraries.
	   */
		int addr;

		addr = ntohl(sock.sin_addr.s_addr);
		dnew->host_ip = addr;
		snprintf(buf, MAX_STRING_LENGTH, "%d.%d.%d.%d",
				 (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
				 (addr >> 8) & 0xFF, (addr)&0xFF);
		snprintf(log_buf, MAX_INPUT_LENGTH * 2, "Sock.sinaddr:  %s", buf);
		log_string(log_buf);
		from = gethostbyaddr((char *)&sock.sin_addr,
							 sizeof(sock.sin_addr), AF_INET);
		dnew->host = str_dup(from ? from->h_name : buf);
	}

	/*
    * Swiftest: I added the following to ban sites.  I don't
    * endorse banning of sites, but Copper has few descriptors now
    * and some people from certain sites keep abusing access by
    * using automated 'autodialers' and leaving connections hanging.
    *
    * Archon : added wild card check to handle IP logins.
    */
	for (pban = ban_list; pban != NULL; pban = pban->next)
	{
		if (strstr(dnew->host, pban->name))
		{
			write_to_descriptor(desc,
								"Your site has been banned from this Mud.\n\r", 0);
			close(desc);
			free_string(dnew->host);
			free_mem(dnew->outbuf, dnew->outsize);
			dnew->next = descriptor_free;
			descriptor_free = dnew;
			return;
		}
	}

	/*
    * Init descriptor data.
    */
	dnew->next = descriptor_list;
	descriptor_list = dnew;

	/*
    * Send the greeting.
    */
	{
		extern char *help_greeting;
		if (help_greeting[0] == '.')
			write_to_buffer(dnew, help_greeting + 1, 0, 0);
		else
			write_to_buffer(dnew, help_greeting, 0, 0);
	}
	return;
}
#endif

void close_socket(DESCRIPTOR_DATA *dclose)
{
	CHAR_DATA *ch;

	if (dclose->outtop > 0)
		process_output(dclose, FALSE);

	if (dclose->snoop_by != NULL)
	{
		write_to_buffer(dclose->snoop_by,
						"Your victim has left the game.\n\r", 0, 1);
	}

	if (dclose->character != NULL && dclose->connected == CON_PLAYING &&
		IS_NPC(dclose->character))
		do_return(dclose->character, "");
	/*
	   if ( dclose->character != NULL
	   && dclose->connected == CON_PLAYING
	   && !IS_NPC(dclose->character)
	   && dclose->character->pcdata != NULL
	   && dclose->character->pcdata->obj_vnum != 0
	   && dclose->character->pcdata->chobj != NULL)
	   extract_obj(dclose->character->pcdata->chobj);
    */
	{
		DESCRIPTOR_DATA *d;

		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->snoop_by == dclose)
				d->snoop_by = NULL;
		}
	}

	if ((ch = dclose->character) != NULL)
	{
		snprintf(log_buf, MAX_INPUT_LENGTH * 2, "Closing link to %s.", ch->name);
		log_string(log_buf);

		/*    sprintf(buf, "%s has left the Vampire Wars.", ch->name);
	   do_info(ch, buf); */

		if (dclose->connected == CON_PLAYING)
		{
			if (IS_NPC(ch) || ch->pcdata->obj_vnum == 0)
				act("$n has lost $s link.", ch, NULL, NULL, TO_ROOM);
			ch->desc = NULL;
		}
		else
		{
			free_char(dclose->character);
		}
	}

	if (d_next == dclose)
		d_next = d_next->next;

	if (dclose == descriptor_list)
	{
		descriptor_list = descriptor_list->next;
	}
	else
	{
		DESCRIPTOR_DATA *d;

		for (d = descriptor_list; d && d->next != dclose; d = d->next)
			;
		if (d != NULL)
			d->next = dclose->next;
		else
			bug("Close_socket: dclose not found.", 0);
	}

	/*	dclose->descriptor = CON_NOT_PLAYING; */

	close(dclose->descriptor);
	/* Palmer testing here */
	/*   extract_char( ch, TRUE); */

	free_string(dclose->host);
	free_mem(dclose->outbuf, dclose->outsize);
	dclose->next = descriptor_free;
	descriptor_free = dclose;
#if defined(MSDOS) || defined(macintosh)
	exit(1);
#endif
	return;
}

/* For a better kickoff message :) KaVir */
void close_socket2(DESCRIPTOR_DATA *dclose)
{
	CHAR_DATA *ch;

	if (dclose->outtop > 0)
		process_output(dclose, FALSE);

	if (dclose->snoop_by != NULL)
	{
		write_to_buffer(dclose->snoop_by,
						"Your victim has left the game.\n\r", 0, 1);
	}

	if (dclose->character != NULL && dclose->connected == CON_PLAYING &&
		IS_NPC(dclose->character))
		do_return(dclose->character, "");
	/*
	   if ( dclose->character != NULL
	   && dclose->connected == CON_PLAYING
	   && !IS_NPC(dclose->character)
	   && dclose->character->pcdata != NULL
	   && dclose->character->pcdata->obj_vnum != 0
	   && dclose->character->pcdata->chobj != NULL)
	   extract_obj(dclose->character->pcdata->chobj);
    */
	{
		DESCRIPTOR_DATA *d;

		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->snoop_by == dclose)
				d->snoop_by = NULL;
		}
	}

	if ((ch = dclose->character) != NULL)
	{
		snprintf(log_buf, MAX_INPUT_LENGTH * 2, "%s kicked by another connection.", ch->name);
		log_string(log_buf);
		if (dclose->connected == CON_PLAYING)
		{
			act("$n doubles over in agony and $s eyes roll up into $s head.", ch, NULL, NULL, TO_ROOM);
			ch->desc = NULL;
		}
		else
		{
			free_char(dclose->character);
		}
	}

	if (d_next == dclose)
		d_next = d_next->next;

	if (dclose == descriptor_list)
	{
		descriptor_list = descriptor_list->next;
	}
	else
	{
		DESCRIPTOR_DATA *d;

		for (d = descriptor_list; d && d->next != dclose; d = d->next)
			;
		if (d != NULL)
			d->next = dclose->next;
		else
			bug("Close_socket: dclose not found.", 0);
	}

	close(dclose->descriptor);
	free_string(dclose->host);
	free_mem(dclose->outbuf, dclose->outsize);
	dclose->next = descriptor_free;
	descriptor_free = dclose;
#if defined(MSDOS) || defined(macintosh)
	exit(1);
#endif
	return;
}

bool read_from_descriptor(DESCRIPTOR_DATA *d)
{
	int iStart;

	/* Hold horses if pending command already. */
	if (d->incomm[0] != '\0')
		return TRUE;

	/* Check for overflow. */
	iStart = strlen(d->inbuf);
	if (iStart >= sizeof(d->inbuf) - 10)
	{
		snprintf(log_buf, MAX_INPUT_LENGTH * 2, "%s input overflow!", d->host);
		log_string(log_buf);
		write_to_descriptor(d->descriptor,
							"\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
		return FALSE;
	}

	/* Snarf input. */
#if defined(macintosh)
	for (;;)
	{
		int c;
		c = getc(stdin);
		if (c == '\0' || c == EOF)
			break;
		putc(c, stdout);
		if (c == '\r')
			putc('\n', stdout);
		d->inbuf[iStart++] = c;
		if (iStart > sizeof(d->inbuf) - 10)
			break;
	}
#endif

#if defined(MSDOS) || defined(unix)
	for (;;)
	{
		int nRead;

		nRead = read(d->descriptor, d->inbuf + iStart,
					 sizeof(d->inbuf) - 10 - iStart);
		if (nRead > 0)
		{
			iStart += nRead;
			if (d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r')
				break;
		}
		else if (nRead == 0)
		{
			log_string("EOF encountered on read.");
			return FALSE;
		}
		else if (errno == EWOULDBLOCK)
			break;
		else
		{
			perror("Read_from_descriptor");
			return FALSE;
		}
	}
#endif

	d->inbuf[iStart] = '\0';
	return TRUE;
}

/*
* Transfer one line from input buffer to input line.
*/
void read_from_buffer(DESCRIPTOR_DATA *d)
{
	int i, j, k;

	/*
    * Hold horses if pending command already.
    */
	if (d->incomm[0] != '\0')
		return;

	/*
	   * Look for at least one new line.
    */
	for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
	{
		if (d->inbuf[i] == '\0')
			return;
	}

	/*
    * Canonical input processing.
    */
	for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
	{
		if (k >= MAX_INPUT_LENGTH - 2)
		{
			write_to_descriptor(d->descriptor, "Line too long.\n\r", 0);

			/* skip the rest of the line */
			for (; d->inbuf[i] != '\0'; i++)
			{
				if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
					break;
			}
			d->inbuf[i] = '\n';
			d->inbuf[i + 1] = '\0';
			break;
		}

		if (d->inbuf[i] == '\b' && k > 0)
			--k;
		else if (isascii(d->inbuf[i]) && isprint(d->inbuf[i]))
			d->incomm[k++] = d->inbuf[i];
	}

	/*
    * Finish off the line.
    */
	if (k == 0)
		d->incomm[k++] = ' ';
	d->incomm[k] = '\0';

	/*
    * Deal with bozos with #repeat 1000 ...
    */
	if (k > 1 || d->incomm[0] == '!')
	{
		if (d->incomm[0] != '!' && strcmp(d->incomm, d->inlast))
		{
			d->repeat = 0;
		}
		else
		{
			if (++d->repeat >= 40)
			{
				snprintf(log_buf, MAX_STRING_LENGTH * 2, "%s input spamming!", d->host);
				log_string(log_buf);
				write_to_descriptor(d->descriptor,
									"\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
				strncpy(d->incomm, "quit", 5);
			}
		}
	}

	/*
    * Do '!' substitution.
    */
	if (d->incomm[0] == '!')
		strncpy(d->incomm, d->inlast, MAX_INPUT_LENGTH);
	else
		strncpy(d->inlast, d->incomm, MAX_INPUT_LENGTH);

	/*
	   * Shift the input buffer.
    */
	while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
		i++;
	for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++)
		;
	return;
}

/*
* Low level output function.
*/
bool process_output(DESCRIPTOR_DATA *d, bool fPrompt)
{
	extern bool merc_down;

	/*
    * Bust a prompt.
    */
	if (fPrompt && !merc_down && d->connected == CON_PLAYING)
	{
		CHAR_DATA *ch;
		CHAR_DATA *victim;

		ch = d->original ? d->original : d->character;
		if (IS_SET(ch->act, PLR_BLANK))
			write_to_buffer(d, "\n\r", 2, 0);

		if (IS_SET(ch->act, PLR_PROMPT))
		{
			char buf[MAX_INPUT_LENGTH];
			char cond[MAX_INPUT_LENGTH];
			char hit_str[MAX_INPUT_LENGTH];
			char mana_str[MAX_INPUT_LENGTH];
			char move_str[MAX_INPUT_LENGTH];
			char exp_str[MAX_INPUT_LENGTH];
			int jok = 1;

			ch = d->character;
			if (IS_HEAD(ch, LOST_HEAD) || IS_EXTRA(ch, EXTRA_OSWITCH))
			{
				snprintf(exp_str, MAX_INPUT_LENGTH, "%ld", ch->exp);
				COL_SCALE(exp_str, ch, ch->exp, 1000, MAX_INPUT_LENGTH);
				snprintf(buf, MAX_INPUT_LENGTH, "[%s exp] <?hp ?m ?mv> ", exp_str);
			}
			else if (ch->position == POS_FIGHTING)
			{
				victim = ch->fighting;
				jok = (victim->hit * 100 / victim->max_hit);
				snprintf(cond, MAX_INPUT_LENGTH, "%d/100", jok);

				if ((victim->hit * 100 / victim->max_hit) < 25)
				{
					ADD_COLOUR(ch, cond, LIGHTRED, MAX_INPUT_LENGTH);
				}
				else if ((victim->hit * 100 / victim->max_hit) < 50)
				{
					ADD_COLOUR(ch, cond, LIGHTBLUE, MAX_INPUT_LENGTH);
				}
				else if ((victim->hit * 100 / victim->max_hit) < 75)
				{
					ADD_COLOUR(ch, cond, LIGHTGREEN, MAX_INPUT_LENGTH);
				}
				else if ((victim->hit * 100 / victim->max_hit) < 100)
				{
					ADD_COLOUR(ch, cond, YELLOW, MAX_INPUT_LENGTH);
				}
				else if ((victim->hit * 100 / victim->max_hit) >= 100)
				{
					ADD_COLOUR(ch, cond, LIGHTCYAN, MAX_INPUT_LENGTH);
				}
				snprintf(hit_str, MAX_INPUT_LENGTH, "%d", ch->hit);
				COL_SCALE(hit_str, ch, ch->hit, ch->max_hit, MAX_INPUT_LENGTH);
				snprintf(mana_str, MAX_INPUT_LENGTH, "%d", ch->mana);
				COL_SCALE(mana_str, ch, ch->mana, ch->max_mana, MAX_INPUT_LENGTH);
				snprintf(move_str, MAX_INPUT_LENGTH, "%d", ch->move);
				COL_SCALE(move_str, ch, ch->move, ch->max_move, MAX_INPUT_LENGTH);
				snprintf(buf, MAX_INPUT_LENGTH, "[ %s ] <%shp %sm %smv> ", cond, hit_str, mana_str, move_str);
			}
			else
			{
				snprintf(hit_str, MAX_INPUT_LENGTH, "%d", ch->hit);
				COL_SCALE(hit_str, ch, ch->hit, ch->max_hit, MAX_INPUT_LENGTH);
				snprintf(mana_str, MAX_INPUT_LENGTH, "%d", ch->mana);
				COL_SCALE(mana_str, ch, ch->mana, ch->max_mana, MAX_INPUT_LENGTH);
				snprintf(move_str, MAX_INPUT_LENGTH, "%d", ch->move);
				COL_SCALE(move_str, ch, ch->move, ch->max_move, MAX_INPUT_LENGTH);
				snprintf(exp_str, MAX_INPUT_LENGTH, "%ld", ch->exp);
				COL_SCALE(exp_str, ch, ch->exp, 1000, MAX_INPUT_LENGTH);
				snprintf(buf, MAX_INPUT_LENGTH, "[%s exp] <%shp %sm %smv> ", exp_str, hit_str, mana_str, move_str);
			}
			write_to_buffer(d, buf, 0, 0);
		}

		if (IS_SET(ch->act, PLR_TELNET_GA))
			write_to_buffer(d, go_ahead_str, 0, 0);
	}

	/*
    * Short-circuit if nothing to write.
    */
	if (d->outtop == 0)
		return TRUE;

	/*
	   * Snoop-o-rama.
    */
	if (d->snoop_by != NULL)
	{
		write_to_buffer(d->snoop_by, "% ", 2, 0);
		write_to_buffer(d->snoop_by, d->outbuf, d->outtop, 0);
	}

	/*
    * OS-dependent output.
    */
	if (!write_to_descriptor(d->descriptor, d->outbuf, d->outtop))
	{
		d->outtop = 0;
		return FALSE;
	}
	else
	{
		d->outtop = 0;
		return TRUE;
	}
}

void write_to_buffer(DESCRIPTOR_DATA *d, const char *txt, int length, int anti_trig)
{
	char *outp;
	int triggered;
	int size;
	bool ansi;

	// added by Oberon

	bool newline = FALSE, returnline = FALSE;

	char options[MAX_COLORS + 2] = {'e', 'i', 'u', 'k', 'w', 'r', 'R', 'g', 'G', 'n', 'l', 'b', 'B', 'm', 'M', 'c', 'C', 'y', 'W', '0',
									'1', '2', '3', 'N', 'L', '4', '5', '6', '7', '8', '9', 'Y', 's', 'S'};

	char *theColors[MAX_COLORS] = {NORMAL, BLINKING, UNDERLINE, D_BLACK, L_WHITE, D_RED, L_RED, D_GREEN, L_GREEN,
								   D_BROWN, D_YELLOW, D_BLUE, L_BLUE, D_MAGENTA, L_MAGENTA, D_CYAN, L_CYAN, D_GREY, L_WHITE_BG, D_RED_BG,
								   L_RED_BG, D_GREEN_BG, L_GREEN_BG, D_BROWN_BG, D_YELLOW_BG, D_BLUE_BG, L_BLUE_BG, D_MAGENTA_BG, L_MAGENTA_BG,
								   D_CYAN_BG, L_CYAN_BG, D_GREY_BG};

	char buffer[MAX_STRING_LENGTH];

	int i = 0, j = 0, h = 0;

	// end of added

	/*
     * Find length in case caller didn't.
     */
	if (length <= 0)
		length = strlen(txt);

	if (length >= MAX_STRING_LENGTH)
	{
		bug("Write_to_buffer: Way too big. Closing.", 0);
		return;
	}

	if (d->character == NULL ||
		(d->connected != CON_PLAYING))
		ansi = FALSE;
	else
		ansi = (IS_SET(d->character->act, PLR_ANSI)) ? TRUE : FALSE;

	/*
       * Initial \n\r if needed.
       */
	if (d->outtop == 0 && !d->fcommand)
	{
		d->outbuf[0] = '\n';
		d->outbuf[1] = '\r';
		d->outtop = 2;
	}

	/*
     * Expand d->outbuf for ansi info
     */

	// added by Oberon

	for (i = 0; i < length; i++)
	{
		if (txt[i] == '#')
		{
			i++;
			switch (txt[i])
			{
			case '#':
				buffer[j++] = '#';
				break;

			case '+':
				buffer[j++] = '%';
				break;

			case '-':
				buffer[j++] = '~';
				break;

			default:
				if (ansi)
				{
					for (h = 0; h < (MAX_COLORS + 2); h++)
					{
						if (txt[i] == options[h])
						{
							buffer[j++] = '\e';
							if (h == MAX_COLORS)
								h = number_range(4, 17);
							if (h == (MAX_COLORS + 1))
								h = number_range(18, 31);
							strcpy(buffer + j, theColors[h]);
							j += strlen(theColors[h]);
							break;
						}
					}
				}
				break;
			}
		}
		else
		{
			if ((txt[i] == '\n') && (i > length - 3))
				newline = TRUE;
			if ((txt[i] == '\r') && (i > length - 3))
				returnline = TRUE;
			buffer[j++] = txt[i];
		}
	}
	if (newline)
		--j;
	if (returnline)
		--j;
	buffer[j++] = '\e';
	strcpy(buffer + j, theColors[0]);
	j += strlen(theColors[0]);
	if (newline)
		buffer[j++] = '\n';
	if (returnline)
		buffer[j++] = '\r';
	buffer[j] = '\0';
	// end of added

	length = j;

	/* grow or shrink the buffer if need be */
	for (size = d->outsize / 2; size < length + d->outtop; size += 1000)
	{
	}
	if (size != d->outsize)
	{
		outp = alloc_mem(size);
		strncpy(outp, d->outbuf, d->outtop);
		free_mem(d->outbuf, d->outsize);
		d->outbuf = outp;
		d->outsize = size;
	}
	/*
    * Cap the max buffer length at 25k
    */
	if (d->outtop + length > 25000)
	{
		length = 25000 - d->outtop;
	}
	/*
    * Copy to the buffer.
    */
	anti_trig = 0; /* disable for now */

	if (!anti_trig)
	{
		strncpy(d->outbuf + d->outtop, buffer, length);
		d->outtop += length;
	}
	else
	{
		triggered = 0;
		outp = d->outbuf + d->outtop;
		while (length > 0)
		{
			/* copy a char from txt */
			*(outp++) = *(txt++);
			++(d->outtop);
			--length;

			if ((!triggered) && (*(txt - 1) == ' ') && (1 == number_range(1, 5)))
			{
				/* anti-trigger code - Genghis
              * there's a 1/5 chance of a space beingremoved */
				--outp;
				--(d->outtop);
				triggered = 1;
			}
			else if (*txt != ' ')
			{
				triggered = 0;
			}
		}
		*(outp++) = '\0';
	}

	return;
}

/*
* Lowest level output function.
* Write a block of text to the file descriptor.
* If this gives errors on very long blocks (like 'ofind all'),
*   try lowering the max block size.
*/
bool write_to_descriptor(int desc, char *txt, int length)
{
	int iStart;
	int nWrite;
	int nBlock;

#if defined(macintosh) || defined(MSDOS)
	if (desc == 0)
		desc = 1;
#endif

	if (length <= 0)
		length = strlen(txt);

	for (iStart = 0; iStart < length; iStart += nWrite)
	{
		nBlock = UMIN(length - iStart, 4096);
		if ((nWrite = write(desc, txt + iStart, nBlock)) < 0)
		{
			perror("Write_to_descriptor");
			return FALSE;
		}
	}

	return TRUE;
}

/*
* Deal with sockets that haven't logged in yet.
*/
void nanny(DESCRIPTOR_DATA *d, char *argument)
{
	char jok[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char kav[MAX_STRING_LENGTH];
	char stat[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	char *pwdnew;
	char *p;
	bool fOld;
	DESCRIPTOR_DATA *df;

	while (isspace(*argument))
		argument++;

	ch = d->character;

	switch (d->connected)
	{

	default:
		bug("Nanny: bad d->connected %d.", d->connected);
		close_socket(d);
		return;

	case CON_GET_NAME:
		if (argument[0] == '\0')
		{
			close_socket(d);
			return;
		}

		argument[0] = UPPER(argument[0]);
		if (!check_parse_name(argument))
		{
			write_to_buffer(d, "Illegal name, try another.\n\rName: ", 0, 0);
			return;
		}

		snprintf(jok, MAX_STRING_LENGTH, "%s is trying to log in.", argument);
		log_string(jok);
		fOld = load_char_obj(d, argument);
		ch = d->character;
		if (fOld && ch->lasthost != NULL && strlen(ch->lasthost) > 1 &&
			ch->lasttime != NULL && strlen(ch->lasttime) > 1)
		{
			snprintf(kav, MAX_STRING_LENGTH, "Last connected at %s", ch->lasttime);
			write_to_buffer(d, kav, 0, 0);
		}
		else if (fOld && ch->lasthost != NULL && strlen(ch->lasthost) > 1)
		{
			snprintf(kav, MAX_STRING_LENGTH, "Last connected from %s.\n\r", ch->lasthost);
			write_to_buffer(d, kav, 0, 0);
		}

		if (IS_SET(ch->act, PLR_DENY))
		{
			snprintf(log_buf, MAX_INPUT_LENGTH * 2, "Denying access to %s @ %s.", argument, d->host);
			log_string(log_buf);
			write_to_buffer(d, "You are denied access.\n\r", 0, 0);
			close_socket(d);
			return;
		}

		if (check_reconnect(d, argument, FALSE))
		{
			fOld = TRUE;
		}
		else
		{
			if (wizlock && !IS_IMMORTAL(ch))
			{
				write_to_buffer(d, "The game is wizlocked.\n\r", 0, 0);
				close_socket(d);
				return;
			}
		}

		if (fOld)
		{
			/* Old player */
			write_to_buffer(d, "Please enter password: ", 0, 0);
			write_to_buffer(d, echo_off_str, 0, 0);
			d->connected = CON_GET_OLD_PASSWORD;
			return;
		}
		else
		{
			/* New player */
			snprintf(buf, MAX_STRING_LENGTH, "You want %s engraved on your tombstone (Y/N)? ", argument);
			write_to_buffer(d, buf, 0, 0);
			d->connected = CON_CONFIRM_NEW_NAME;
			return;
		}
		break;

	case CON_GET_OLD_PASSWORD:
#if defined(unix)
		write_to_buffer(d, "\n\r", 2, 0);
#endif

		if (ch == NULL || (!IS_EXTRA(ch, EXTRA_NEWPASS) &&
						   strcmp(argument, ch->pcdata->pwd) &&
						   strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd)))
		{
			if (strcmp(argument, ch->pcdata->pwd))
			{
				write_to_buffer(d, "Wrong password.\n\r", 0, 0);
				close_socket(d);
				return;
			}
		}
		else if (IS_EXTRA(ch, EXTRA_NEWPASS) &&
				 strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd))
		{
			if (strcmp(argument, ch->pcdata->pwd))
			{
				write_to_buffer(d, "Wrong password.\n\r", 0, 0);
				close_socket(d);
				return;
			}
		}

		if (!IS_EXTRA(ch, EXTRA_NEWPASS))
		{
			snprintf(kav, MAX_STRING_LENGTH, "%s %s", argument, argument);
			do_password(ch, kav);
		}

		write_to_buffer(d, echo_on_str, 0, 0);

		if (check_reconnect(d, ch->name, TRUE))
			return;

		if (check_playing(d, ch->name))
			return;

		if (check_kickoff(d, ch->name, TRUE))
			return;

		/* Avoid nasty duplication bug - KaVir */
		snprintf(kav, MAX_STRING_LENGTH, "%s", ch->name);
		free_char(d->character);
		d->character = NULL;
		fOld = load_char_obj(d, kav);
		ch = d->character;

		if (ch->lasthost != NULL)
			free_string(ch->lasthost);
		ch->lasthost = str_dup(ch->desc->host);
		if (ch->lasttime != NULL)
			free_string(ch->lasttime);
		ch->lasttime = str_dup(ctime(&current_time));

		snprintf(log_buf, MAX_INPUT_LENGTH * 2, "%s@%s has connected.", ch->name, d->host);
		log_string(log_buf);

		/* In case we have level 4+ players from another merc mud, or
	   * players who have somehow got file access and changed their pfiles.
	   */
		if (ch->level > 3 && ch->trust == 0)
			ch->level = 3;
		else
		{
			if (ch->level > MAX_LEVEL)
				ch->level = 1;
			if (ch->trust > MAX_LEVEL)
				ch->trust = 1;
			/* To temporarily grant higher powers... */
			if (ch->trust > ch->level)
				ch->trust = ch->level;
		}

		if (IS_IMMORTAL(ch))
			do_help(ch, "imotd");
		do_help(ch, "motd");
		d->connected = CON_READ_MOTD;
		break;

	case CON_CONFIRM_NEW_NAME:
		switch (*argument)
		{
		case 'y':
		case 'Y':
			for (df = descriptor_list; df != NULL; df = df->next)
			{
				if ((d != df) && (df->character != NULL) && (df->character->name != NULL) && !strncasecmp(df->character->name, d->character->name, 16))
				{
					write_to_buffer(d, "That name is already in use.\n\r", 0, 0);
					close_socket(d);
					return;
				}
			}

			snprintf(buf, MAX_STRING_LENGTH, "New character.\n\rGive me a password for %s: %s",
					 ch->name, echo_off_str);
			write_to_buffer(d, buf, 0, 0);
			d->connected = CON_GET_NEW_PASSWORD;
			break;

		case 'n':
		case 'N':
			write_to_buffer(d, "Ok, what IS it, then? ", 0, 0);
			free_char(d->character);
			d->character = NULL;
			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer(d, "Please type Yes or No? ", 0, 0);
			break;
		}
		break;

	case CON_GET_NEW_PASSWORD:
#if defined(unix)
		write_to_buffer(d, "\n\r", 2, 0);
#endif

		if (strlen(argument) < 5)
		{
			write_to_buffer(d,
							"Password must be at least five characters long.\n\rPassword: ",
							0, 0);
			return;
		}

		pwdnew = crypt(argument, ch->name);

		for (p = pwdnew; *p != '\0'; p++)
		{
			if (*p == '~')
			{
				write_to_buffer(d,
								"New password not acceptable, try again.\n\rPassword: ",
								0, 0);
				return;
			}
		}

		free_string(ch->pcdata->pwd);
		ch->pcdata->pwd = str_dup(pwdnew);

		write_to_buffer(d, "Please retype password: ", 0, 0);
		d->connected = CON_CONFIRM_NEW_PASSWORD;
		break;

	case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
		write_to_buffer(d, "\n\r", 2, 0);
#endif

		if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd))
		{
			write_to_buffer(d, "Passwords don't match.\n\rRetype password: ",
							0, 0);
			d->connected = CON_GET_NEW_PASSWORD;
			return;
		}

		write_to_buffer(d, echo_on_str, 0, 0);
		write_to_buffer(d, "What is your sex (M/F/N)? ", 0, 0);
		d->connected = CON_GET_NEW_SEX;
		break;

	case CON_GET_NEW_SEX:
		switch (argument[0])
		{
		case 'm':
		case 'M':
			ch->sex = SEX_MALE;
			break;
		case 'f':
		case 'F':
			ch->sex = SEX_FEMALE;
			break;
		case 'n':
		case 'N':
			ch->sex = SEX_NEUTRAL;
			break;
		default:
			write_to_buffer(d, "That's not a sex.\n\rWhat IS your sex? ", 0, 0);
			return;
		}

		ch->pcdata->perm_str = 18;
		ch->pcdata->perm_int = 18;
		ch->pcdata->perm_wis = 18;
		ch->pcdata->perm_dex = 18;
		ch->pcdata->perm_con = 18;
		strncpy(buf, "Your stats are: [", MAX_STRING_LENGTH);
		snprintf(stat, MAX_STRING_LENGTH, "Str %d, ", ch->pcdata->perm_str);
		strncat(buf, stat, MAX_STRING_LENGTH - strlen(buf));
		snprintf(stat, MAX_STRING_LENGTH, "Int %d, ", ch->pcdata->perm_int);
		strncat(buf, stat, MAX_STRING_LENGTH - strlen(buf));
		snprintf(stat, MAX_STRING_LENGTH, "Wis %d, ", ch->pcdata->perm_wis);
		strncat(buf, stat, MAX_STRING_LENGTH - strlen(buf));
		snprintf(stat, MAX_STRING_LENGTH, "Dex %d, ", ch->pcdata->perm_dex);
		strncat(buf, stat, MAX_STRING_LENGTH - strlen(buf));
		snprintf(stat, MAX_STRING_LENGTH, "Con %d.", ch->pcdata->perm_con);
		strncat(buf, stat, MAX_STRING_LENGTH - strlen(buf));
		strncat(buf, "]", MAX_STRING_LENGTH - strlen(buf));
		write_to_buffer(d, buf, 0, 0);
		ch->class = 0;
		snprintf(log_buf, MAX_INPUT_LENGTH * 2, "%s@%s new player.", ch->name, d->host);
		log_string(log_buf);
		write_to_buffer(d, "\n\r", 2, 0);
		do_help(ch, "motd");
		d->connected = CON_READ_MOTD;
		break;
	case CON_READ_MOTD:
		write_to_buffer(d, "\n\rWelcome to Vampire Wars Mud.  May thy blade stay ever sharp, thy soul ever dark.\n\r",
						0, 0);
		ch->next = char_list;
		char_list = ch;
		d->connected = CON_PLAYING;

		if (ch->level == 0)
		{
			ch->level = 1;
			ch->exp = 0;
			ch->hit = ch->max_hit;
			ch->mana = ch->max_mana;
			ch->move = ch->max_move;
			set_title(ch, "the mortal");

			char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));
			do_look(ch, "auto");
		}
		else if (!IS_NPC(ch) && ch->pcdata->obj_vnum != 0)
		{
			if (ch->in_room != NULL)
				char_to_room(ch, ch->in_room);
			else
				char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));
			bind_char(ch);
			break;
		}
		else if (ch->in_room != NULL)
		{
			char_to_room(ch, ch->in_room);
			do_look(ch, "auto");
		}
		else if (IS_IMMORTAL(ch))
		{
			char_to_room(ch, get_room_index(ROOM_VNUM_CHAT));
			do_look(ch, "auto");
		}
		else
		{
			char_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
			do_look(ch, "auto");
		}

		/* Code to mess up them damn age botters - Archon */
		ch->logon = current_time;

		snprintf(buf, MAX_STRING_LENGTH, "%s has entered the Vampire Wars.", ch->name);
		do_info(ch, buf);
		act("$n has entered the game.", ch, NULL, NULL, TO_ROOM);
		room_text(ch, ">ENTER<");
		break;
	}

	return;
}

/*
* Parse a name for acceptability.
*/
bool check_parse_name(char *name)
{
	/*
* Reserved words.
    */
	if (is_name(name, "all auto immortal self you someone warrior avatar fighter adventurer champion hero fuck twat cunt dickhead fuckov fuckof fuckoff bastard wanker penis fucker anyone something somebody shit shithead spam spammer arse brujah malkavian ventrue tremere gangrel nosferatu toreador assamite cappadocian"))
		return FALSE;

	/*
	   * Length restrictions.
    */
	if (strlen(name) < 3)
		return FALSE;

#if defined(MSDOS)
	if (strlen(name) > 8)
		return FALSE;
#endif

#if defined(macintosh) || defined(unix)
	if (strlen(name) > 12)
		return FALSE;
#endif

	/*
	   * Alphanumerics only.
	   * Lock out IllIll twits.
    */
	{
		char *pc;
		bool fIll;

		fIll = TRUE;
		for (pc = name; *pc != '\0'; pc++)
		{
			if (!isalpha(*pc))
				return FALSE;
			if (LOWER(*pc) != 'i' && LOWER(*pc) != 'l')
				fIll = FALSE;
		}

		if (fIll)
			return FALSE;
	}

	/*
    * Prevent players from naming themselves after mobs.
    */
	{
		extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
		MOB_INDEX_DATA *pMobIndex;
		int iHash;

		for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
		{
			for (pMobIndex = mob_index_hash[iHash];
				 pMobIndex != NULL;
				 pMobIndex = pMobIndex->next)
			{
				if (is_name(name, pMobIndex->player_name))
					return FALSE;
			}
		}
	}

	return TRUE;
}

/*
* Look for link-dead player to reconnect.
*/
bool check_reconnect(DESCRIPTOR_DATA *d, char *name, bool fConn)
{
	CHAR_DATA *ch;

	for (ch = char_list; ch != NULL; ch = ch->next)
	{
		if (!IS_NPC(ch) && !IS_EXTRA(ch, EXTRA_SWITCH) && (!fConn || ch->desc == NULL) && !str_cmp(d->character->name, ch->name))
		{
			if (fConn == FALSE)
			{
				free_string(d->character->pcdata->pwd);
				d->character->pcdata->pwd = str_dup(ch->pcdata->pwd);
			}
			else
			{
				free_char(d->character);
				d->character = ch;
				ch->desc = d;
				ch->timer = 0;
				send_to_char("Reconnecting.\n\r", ch);
				if (IS_NPC(ch) || ch->pcdata->obj_vnum == 0)
					act("$n has reconnected.", ch, NULL, NULL, TO_ROOM);
				snprintf(log_buf, MAX_INPUT_LENGTH * 2, "%s @ %s reconnected.", ch->name, d->host);
				log_string(log_buf);
				d->connected = CON_PLAYING;
			}
			return TRUE;
		}
	}

	return FALSE;
}

/*
* Kick off old connection.  KaVir.
*/
bool check_kickoff(DESCRIPTOR_DATA *d, char *name, bool fConn)
{
	CHAR_DATA *ch;

	for (ch = char_list; ch != NULL; ch = ch->next)
	{
		if (!IS_NPC(ch) && (!fConn || ch->desc == NULL) && !str_cmp(d->character->name, ch->name))
		{
			if (fConn == FALSE)
			{
				free_string(d->character->pcdata->pwd);
				d->character->pcdata->pwd = str_dup(ch->pcdata->pwd);
			}
			else
			{
				free_char(d->character);
				d->character = ch;
				ch->desc = d;
				ch->timer = 0;
				send_to_char("You take over your body, which was already in use.\n\r", ch);
				act("...$n's body has been taken over by another spirit!", ch, NULL, NULL, TO_ROOM);
				snprintf(log_buf, MAX_INPUT_LENGTH * 2, "%s @ %s kicking off old link.", ch->name, d->host);
				log_string(log_buf);
				d->connected = CON_PLAYING;
			}
			return TRUE;
		}
	}

	return FALSE;
}

/*
* Check if already playing - KaVir.
*/
bool check_playing(DESCRIPTOR_DATA *d, char *name)
{
	DESCRIPTOR_DATA *dold;

	for (dold = descriptor_list; dold; dold = dold->next)
	{
		if (dold != d && dold->character != NULL && dold->connected != CON_GET_NAME && dold->connected != CON_GET_OLD_PASSWORD && !str_cmp(name, dold->original ? dold->original->name : dold->character->name))
		{
			write_to_buffer(dold, "This body has been taken over!\n\r", 0, 0);
			close_socket2(dold);
			return FALSE;
		}
	}

	return FALSE;
}

void stop_idling(CHAR_DATA *ch)
{
	if (ch == NULL || ch->desc == NULL || ch->desc->connected != CON_PLAYING || ch->was_in_room == NULL || ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
		return;

	ch->timer = 0;
	char_from_room(ch);
	char_to_room(ch, ch->was_in_room);
	ch->was_in_room = NULL;
	act("$n has returned from the void.", ch, NULL, NULL, TO_ROOM);
	return;
}

/*
* Write to one char.
*/
void send_to_char(const char *txt, CHAR_DATA *ch)
{
	if (txt != NULL && ch->desc != NULL)
		write_to_buffer(ch->desc, txt, strlen(txt), 1);
	return;
}
/* this one's for formatted text, such as help files and room descs */
void send_to_char_formatted(const char *txt, CHAR_DATA *ch)
{
	if (txt != NULL && ch->desc != NULL)
		write_to_buffer(ch->desc, txt, strlen(txt), 0);
	return;
}

/*
* The primary output interface for formatted output.
*/
void act(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type)
{
	static char *const he_she[] = {"it", "he", "she"};
	static char *const him_her[] = {"it", "him", "her"};
	static char *const his_her[] = {"its", "his", "her"};

	char buf[MAX_STRING_LENGTH];
	char fname[MAX_INPUT_LENGTH];
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *)arg2;
	OBJ_DATA *obj1 = (OBJ_DATA *)arg1;
	OBJ_DATA *obj2 = (OBJ_DATA *)arg2;
	const char *str;
	const char *i;
	char *point;
	bool is_ok;
	CHANNEL_DATA *channel = NULL;

	/*
    * Discard null and zero-length messages.
    */
	if (format == NULL || format[0] == '\0')
		return;

	to = ch->in_room->people;
	if (type == TO_VICT)
	{
		if (vch == NULL)
		{
			bug("Act: null vch with TO_VICT.", 0);
			return;
		}
		to = vch->in_room->people;
	}

	for (; to != NULL; to = to->next_in_room)
	{
		if (to->desc == NULL || !IS_AWAKE(to))
			continue;

		if (ch->in_room->vnum == ROOM_VNUM_IN_OBJECT)
		{
			is_ok = FALSE;

			if (!IS_NPC(ch) && ch->pcdata->chobj != NULL &&
				ch->pcdata->chobj->in_room != NULL &&
				!IS_NPC(to) && to->pcdata->chobj != NULL &&
				to->pcdata->chobj->in_room != NULL &&
				ch->in_room == to->in_room)
				is_ok = TRUE;
			else
				is_ok = FALSE;

			if (!IS_NPC(ch) && ch->pcdata->chobj != NULL &&
				ch->pcdata->chobj->in_obj != NULL &&
				!IS_NPC(to) && to->pcdata->chobj != NULL &&
				to->pcdata->chobj->in_obj != NULL &&
				ch->pcdata->chobj->in_obj == to->pcdata->chobj->in_obj)
				is_ok = TRUE;
			else
				is_ok = FALSE;

			if (!is_ok)
				continue;
		}
		if (type == TO_CHAR && to != ch)
			continue;
		if (type == TO_VICT && (to != vch || to == ch))
			continue;
		if (type == TO_ROOM && to == ch)
			continue;
		if (type == TO_NOTVICT && (to == ch || to == vch))
			continue;

		point = buf;
		str = format;
		while (*str != '\0')
		{
			if (*str != '$' )
			{
				*point++ = *str++;
				continue;
			}
			++str;

			if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
			{
				bug("Act: missing arg2 for code %d.", *str);
				i = " <@@@> ";
			}
			else
			{
				--str;
				if (*str == '`')
				{
					switch (*str)
					{
					case 'a':
						channel = to->pcdata->chat_history;
						break;
					case 't':
						channel = to->pcdata->tell_history;
						break;
					case 'n':
						channel = to->pcdata->newbie_history;
						break;
					case 'c':
						channel = to->pcdata->clan_history;
						break;
					}
					i = "";
				}
				else
				{
					++str;
					switch (*str)
					{
					default:
						bug("Act: bad code %d.", *str);
						i = " <@@@> ";
						break;
						/* Thx alex for 't' idea */
					case 't':
						i = (char *)arg1;
						break;
					case 'T':
						i = (char *)arg2;
						break;
					case 'n':
						i = PERS(ch, to);
						break;
					case 'N':
						i = PERS(vch, to);
						break;
					case 'e':
						i = he_she[URANGE(0, ch->sex, 2)];
						break;
					case 'E':
						i = he_she[URANGE(0, vch->sex, 2)];
						break;
					case 'm':
						i = him_her[URANGE(0, ch->sex, 2)];
						break;
					case 'M':
						i = him_her[URANGE(0, vch->sex, 2)];
						break;
					case 's':
						i = his_her[URANGE(0, ch->sex, 2)];
						break;
					case 'S':
						i = his_her[URANGE(0, vch->sex, 2)];
						break;

					case 'p':
						i = can_see_obj(to, obj1)
								? ((obj1->chobj != NULL && obj1->chobj == to)
									   ? "you"
									   : obj1->short_descr)
								: "something";
						break;

					case 'P':
						i = can_see_obj(to, obj2)
								? ((obj2->chobj != NULL && obj2->chobj == to)
									   ? "you"
									   : obj2->short_descr)
								: "something";
						break;

					case 'd':
						if (arg2 == NULL || ((char *)arg2)[0] == '\0')
						{
							i = "door";
						}
						else
						{
							one_argument((char *)arg2, fname, MAX_INPUT_LENGTH);
							i = fname;
						}
						break;
					}
				}
			}

			++str;
			while ((*point = *i) != '\0')
				++point, ++i;
		}

		*point++ = '\n';
		*point++ = '\r';
		buf[0] = UPPER(buf[0]);
		
		if(channel != NULL)
		{
			add_to_history(channel, buf);
		}
		
		write_to_buffer(to->desc, buf, point - buf, 1);
	}

	return;
}

void kavitem(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type)
{
	static char *const he_she[] = {"it", "he", "she"};
	static char *const him_her[] = {"it", "him", "her"};
	static char *const his_her[] = {"its", "his", "her"};

	char buf[MAX_STRING_LENGTH];
	char kav[MAX_INPUT_LENGTH];
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *)arg2;
	OBJ_DATA *obj1 = (OBJ_DATA *)arg1;
	const char *str;
	const char *i;
	char *point;
	bool is_ok;

	/*
    * Discard null and zero-length messages.
    */
	if (format == NULL || format[0] == '\0')
		return;

	to = ch->in_room->people;
	if (type == TO_VICT)
	{
		if (vch == NULL)
		{
			bug("Act: null vch with TO_VICT.", 0);
			return;
		}
		to = vch->in_room->people;
	}

	for (; to != NULL; to = to->next_in_room)
	{
		if (to->desc == NULL || !IS_AWAKE(to))
			continue;

		if (ch->in_room->vnum == ROOM_VNUM_IN_OBJECT)
		{
			is_ok = FALSE;

			if (!IS_NPC(ch) && ch->pcdata->chobj != NULL &&
				ch->pcdata->chobj->in_room != NULL &&
				!IS_NPC(to) && to->pcdata->chobj != NULL &&
				to->pcdata->chobj->in_room != NULL &&
				ch->in_room == to->in_room)
				is_ok = TRUE;
			else
				is_ok = FALSE;

			if (!IS_NPC(ch) && ch->pcdata->chobj != NULL &&
				ch->pcdata->chobj->in_obj != NULL &&
				!IS_NPC(to) && to->pcdata->chobj != NULL &&
				to->pcdata->chobj->in_obj != NULL &&
				ch->pcdata->chobj->in_obj == to->pcdata->chobj->in_obj)
				is_ok = TRUE;
			else
				is_ok = FALSE;

			if (!is_ok)
				continue;
		}
		if (type == TO_CHAR && to != ch)
			continue;
		if (type == TO_VICT && (to != vch || to == ch))
			continue;
		if (type == TO_ROOM && to == ch)
			continue;
		if (type == TO_NOTVICT && (to == ch || to == vch))
			continue;

		point = buf;
		str = format;
		while (*str != '\0')
		{
			if (*str != '$')
			{
				*point++ = *str++;
				continue;
			}
			++str;

			if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
				i = "";
			else
			{
				switch (*str)
				{
				default:
					i = "";
					break;
				case 'n':
					i = PERS(ch, to);
					break;
				case 'e':
					i = he_she[URANGE(0, ch->sex, 2)];
					break;
				case 'm':
					i = him_her[URANGE(0, ch->sex, 2)];
					break;
				case 's':
					i = his_her[URANGE(0, ch->sex, 2)];
					break;
				case 'p':
					i = can_see_obj(to, obj1)
							? ((obj1->chobj != NULL && obj1->chobj == to)
								   ? "you"
								   : obj1->short_descr)
							: "something";
					break;

				case 'o':
					if (obj1 != NULL)
						snprintf(kav, MAX_INPUT_LENGTH, "%s's", obj1->short_descr);
					i = can_see_obj(to, obj1)
							? ((obj1->chobj != NULL && obj1->chobj == to)
								   ? "your"
								   : kav)
							: "something's";
					break;
				}
			}

			++str;
			while ((*point = *i) != '\0')
				++point, ++i;
		}

		*point++ = '\n';
		*point++ = '\r';
		buf[0] = UPPER(buf[0]);
		write_to_buffer(to->desc, buf, point - buf, 1);
	}

	return;
}

/*
* Macintosh support functions.
*/
#if defined(macintosh)
int gettimeofday(struct timeval *tp, void *tzp)
{
	tp->tv_sec = time(NULL);
	tp->tv_usec = 0;
}
#endif
