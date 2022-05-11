#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <libconfig.h>
#define CMDLENGTH		50

typedef struct {
	char *cmd;
	unsigned int interval;
	unsigned int signal;
} Block;
static inline void free_blocks(void);
void sig_handler(int num);
void button_handler(int sig, siginfo_t *si, void *ucontext);
void replace(char *str, char old, char new);
void remove_all(char *str, char to_remove);
void get_cmds(int time);
#ifndef __OpenBSD__
static void get_sig_cmds(int signal);
static FILE *open_config_file_at(const char *base, char **out_path);
static char *parse_libconfig(void);
void setup_signals();
void sig_handler(int signum);
#endif
int get_status(char *str, char *last);
void set_root();
static void status_loop();
void term_handler(int signum);
static const char *xdg_config_home(void);


static Display *dpy;
static int screen;
static Window root;
static char (*statusbar)[CMDLENGTH];
static char statusstr[2][256];
static int statusContinue = 1;
void (*writestatus) () = set_root;
static const char *delim = "|";
static int block_size = 0;
static Block *blocks;

inline void free_blocks(void)
{
	for (int i = 0; i < block_size; i++)
		free(blocks[i].cmd);
	free(blocks);
}

const char *xdg_config_home(void) {
	char *xdgh = getenv("XDG_CONFIG_HOME");
	char *home = getenv("HOME");
	const char *default_dir = "/.config";

	if (!xdgh)
	{
		if (!home)
			return NULL;

		xdgh = malloc(strlen(home) + strlen(default_dir) + 1);

		strcpy(xdgh, home);
		strcat(xdgh, default_dir);
	}
	else
		xdgh = strdup(xdgh);
	return xdgh;
}

FILE *open_config_file_at(const char *base, char **out_path)
{
	static const char *config_path = "/phyos/dwmblocks/dwmblocks.cfg";
	int len = strlen(base) + strlen(config_path);
	char *out = malloc(len);
	strcpy(out, base);
	strcat(out, config_path);

	FILE *ret = fopen(out, "r");

	if (ret && out_path)
	{
		*out_path = out;
		return ret;
	}

	free(out);
	return NULL;
}

char *parse_libconfig(void)
{
	static int is_init = 0;
	is_init = (block_size ? 1 : 0);
	config_t cfg;
	config_setting_t *setting;
	char *path = NULL;
	const char *str = xdg_config_home();
	config_init(&cfg);


	if (!str)
	{
		fprintf(stderr, "Could not locate path for config home\n");
		goto err;
	}
	FILE *f = open_config_file_at(str, &path);
	free((char *)str);

	if (!path)
	{
		fprintf(stderr, "Could not find path for config file\n");
		goto err;
	}

	char *pos = strrchr(path, '/');
	if (pos)
		*pos = '\0';
	config_set_include_dir(&cfg, path);
	*pos = '/';


	if (!config_read_file(&cfg, path) || config_read(&cfg, f) == CONFIG_FALSE)
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
		config_error_line(&cfg), config_error_text(&cfg));
		fclose(f);
		goto err;
	}
	fclose(f);

	setting = config_lookup(&cfg, "blocks");

	if (setting)
	{
		int count = config_setting_length(setting);
		int i;
		if (!is_init)
		{
			block_size = count;
			blocks = malloc(block_size * sizeof(Block));
			statusbar = calloc(block_size, sizeof(*statusbar));
			is_init = 1;
		}
		else
		{
			free_blocks();
			block_size = count;
			blocks = malloc(sizeof(Block) * block_size);
			statusbar = realloc(statusbar, block_size * sizeof(*statusbar));
		}

		for (i = 0; i < count; i++)
		{
			config_setting_t *block = config_setting_get_elem(setting, i);

			const char *cmd;
			int interval, signal;

			if (!(config_setting_lookup_string(block, "cmd", &cmd)
				  && config_setting_lookup_int(block, "interval", &interval)
				  && config_setting_lookup_int(block, "signal", &signal)))
				return NULL;
			blocks[i] = (Block) {.cmd = strdup(cmd), .interval = interval, .signal = signal};
		}
		config_destroy(&cfg);
		return path;
	}
err:
	fprintf(stderr, "Error occurred on parsing/reading config file. Reverting to defaults\n");
	free(path);
	config_destroy(&cfg);
	return NULL;
}


void replace(char *str, char old, char new)
{
	int N = strlen(str);
	for(int i = 0; i < N; i++)
		if(str[i] == old)
			str[i] = new;
}

void remove_all(char *str, char to_remove) {
	char *read = str;
	char *write = str;
	while (*read) {
		if (*read == to_remove) {
			read++;
			*write = *read;
		}
		read++;
		write++;
	}
}

//opens process *cmd and stores output in *output
void get_cmd(const Block *block, char *output)
{
	if (block->signal)
	{
		output[0] = block->signal;
		output++;
	}
	const char *cmd = block->cmd;
	FILE *cmdf = popen(cmd,"r");
	if (!cmdf)
		return;
	int i = 0;
	fgets(output+i, CMDLENGTH-(strlen(delim)+1), cmdf);
	remove_all(output, '\n');
	i = strlen(output);
    if ((i > 0 && block != &blocks[block_size - 1]))
        strcat(output, delim);

    i+=strlen(delim);
	output[i++] = '\0';
	pclose(cmdf);
}

void get_cmds(int time)
{
	const Block* current;
	for(int i = 0; i < block_size; i++)
	{
		current = blocks + i;
		if ((current->interval != 0 && time % current->interval == 0) || time == -1)
			get_cmd(current,statusbar[i]);
	}
}

#ifndef __OpenBSD__
void get_sig_cmds(int signal)
{
	const Block *current;
	for (int i = 0; i < block_size; i++)
	{
		current = blocks + i;
		if (current->signal == signal)
			get_cmd(current,statusbar[i]);
	}
}

void setup_signals()
{
	struct sigaction sa;
	for(int i = 0; i < block_size; i++)
	{
		if (blocks[i].signal > 0)
		{
			signal(SIGRTMIN+blocks[i].signal, sig_handler);
			sigaddset(&sa.sa_mask, SIGRTMIN+blocks[i].signal);
		}
	}
	sa.sa_sigaction = button_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sa, NULL);
	struct sigaction sigchld_action = {
  		.sa_handler = SIG_DFL,
  		.sa_flags = SA_NOCLDWAIT
	};
	sigaction(SIGCHLD, &sigchld_action, NULL);

}
#endif

int get_status(char *str, char *last)
{
	strcpy(last, str);
	str[0] = '\0';
    for(int i = 0; i < block_size; i++) {
		strcat(str, statusbar[i]);
        if (i == block_size - 1)
            strcat(str, " ");
    }
	str[strlen(str)-1] = '\0';
	return strcmp(str, last);//0 if they are the same
}

void set_root()
{
	if (!get_status(statusstr[0], statusstr[1]))//Only set root if text has changed.
		return;
	Display *d = XOpenDisplay(NULL);
	if (d) {
		dpy = d;
	}
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	XStoreName(dpy, root, statusstr[0]);
	XCloseDisplay(dpy);
}

void pstdout()
{
	if (!get_status(statusstr[0], statusstr[1]))//Only write out if text has changed.
		return;
	printf("%s\n",statusstr[0]);
	fflush(stdout);
}


void status_loop()
{
#ifndef __OpenBSD__
	setup_signals();
#endif
	int i = 0;
	get_cmds(-1);
	while(statusContinue)
	{
		get_cmds(i);
		writestatus();
		sleep(1.0);
		i++;
	}
}

#ifndef __OpenBSD__
void sig_handler(int signum)
{
	get_sig_cmds(signum-SIGRTMIN);
	writestatus();
}

void button_handler(int sig, siginfo_t *si, void *ucontext)
{
	char button[2] = {('0' + si->si_value.sival_int) & 0xff, '\0'};
	pid_t process_id = getpid();
	sig = si->si_value.sival_int >> 8;
	if (fork() == 0)
	{
		const Block *current;
		for (int i = 0; i < block_size; i++)
		{
			current = blocks + i;
			if (current->signal == sig)
				break;
		}
		char shcmd[1024];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
		sprintf(shcmd,"%s && kill -%d %d",current->cmd, current->signal+34,process_id);
#pragma GCC diagnostic pop
		char *command[] = { "/bin/sh", "-c", shcmd, NULL };
		setenv("BLOCK_BUTTON", button, 1);
		setsid();
		execvp(command[0], command);
		exit(EXIT_SUCCESS);
	}
}
#endif

void term_handler(int signum)
{
	statusContinue = 0;
	free_blocks();
	free(statusbar);
	exit(0);
}

int main(int argc, char** argv)
{
	for(int i = 0; i < argc; i++)
	{
		if (!strcmp("-d",argv[i]))
			delim = argv[++i];
		else if(!strcmp("-p",argv[i]))
			writestatus = pstdout;
	}

	char *path = parse_libconfig();

	if (!path)
	{
		#include "dwmblocks.h"
		block_size = BLOCK_SIZE;
	}
	else
		free(path);

	signal(SIGTERM, term_handler);
	signal(SIGINT, term_handler);
	status_loop();
}

