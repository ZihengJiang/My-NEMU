#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_p(char *args) {
	if(!args) {
		printf("USAGE: p EXP\n");
		return 0;
	}
	bool success;
	uint32_t result = expr(args, &success);
	Assert(success, "Invalid expression\n");
	printf("0x%08x\n", result);
	return 0;
}

static int cmd_si(char *args) {
	if(!args) {
		printf("USAGE: si NUM\n");
		return 0;
	}
	int n = atoi(args);
	cpu_exec(n);
	return 0;
}

static int cmd_info(char *args) {
	if(!args) {
		printf("USAGE: info r/w\n");
		return 0;
	}

	if(strcmp(args, "r") == 0)
	{
		printf("  REG\t\tHEX \t\t\tDEC \n");
		printf("  eax\t\t0x%08x\t\t%10d\n", reg_l(0), reg_l(0));
		printf("  ecx\t\t0x%08x\t\t%10d\n", reg_l(1), reg_l(1));
		printf("  edx\t\t0x%08x\t\t%10d\n", reg_l(2), reg_l(2));
		printf("  ebx\t\t0x%08x\t\t%10d\n", reg_l(3), reg_l(3));
		printf("  esp\t\t0x%08x\t\t%10d\n", reg_l(4), reg_l(4));
		printf("  ebp\t\t0x%08x\t\t%10d\n", reg_l(5), reg_l(5));
		printf("  esi\t\t0x%08x\t\t%10d\n", reg_l(6), reg_l(6));
		printf("  edi\t\t0x%08x\t\t%10d\n", reg_l(7), reg_l(7));
		printf("  eip\t\t0x%08x\t\t%10d\n", cpu.eip, cpu.eip);
		printf("  eflags\tCF-%u PF-%u ZF-%u SF-%u IF-%u DF-%u OF-%u\n",
				cpu.eflags.CF, cpu.eflags.PF, cpu.eflags.ZF,
				cpu.eflags.SF, cpu.eflags.IF, cpu.eflags.DF, cpu.eflags.OF);
	}
	else if(strcmp(args, "w") == 0)
	{
		WP *wp = head_wp();
		while(wp)
		{
			printf("watchpoint %d: %s\n", wp->NO, wp->str);
			wp = wp->next;
		}
	}
	else {
		printf("USAGE: info r/w\n");
	}
	return 0;
}

static int cmd_x(char *args) {
	if(!args) {
		printf("USAGE: x EXP\n");
		return 0;
	}

	unsigned int i, n, addr;
	sscanf(args, "%u 0x%x", &n, &addr);
	for(i = 0; i < n; ++i)
	{
		unsigned int j, addroff = addr + i*4;
		printf("0x%08x:\t", addroff);
		for(j = 0; j < 4; ++j)
		{
			printf("%02x ", swaddr_read(addroff + j, 1));
		}
		printf("\n");
	}
	return 0;
}

static int cmd_w(char *args) {
	if(!args) {
		printf("USAGE: w EXP\n");
		return 0;
	}

	WP *wp = new_wp();
	strcpy(wp->str, args);
	bool success = false;
	uint32_t result = expr(args, &success);
	if(!success) {
		printf("Invalid expression.");
		return 0;
	}
	wp->oldvalue = result;
	printf("Add watchpoint %d: %s\n", wp->NO, wp->str);
	return 0;
}

static int cmd_d(char *args) {
	if(!args) {
		printf("USAGE: d NUM\n");
		return 0;
	}

	int n = atoi(args);
	WP *wp = head_wp();
	while(wp) {
		if(wp->NO == n) {
			printf("Delete watchpoint %d: %s\n", wp->NO, wp->str);
			free_wp(wp);
			return 0;
		}
		wp = wp->next;
	}
	printf("No watchpoint %d", n);
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Step NUM instructions", cmd_si },
	{ "info", "r-List of all registers and their contents\n\t  w-Print status of all watchpoints", cmd_info },
	{ "p", "Print value of expression EXP", cmd_p },
	{ "x", "Examine memory", cmd_x },
	{ "w", "Set a watchpoint for an expression", cmd_w },
	{ "d", "Delete a specified watchpoint", cmd_d }
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s\t- %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
