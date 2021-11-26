// ed - a line oriented text editor
// Copyright (c) 2019 David Čepelík <d@dcepelik.cz>
// MIT License
// Source: https://github.com/dcepelik/ed

#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define LINE_MAX_LEN 1024

struct line {
	char *text;
	struct line *next;
	struct line *prev;
};

static char *read_line(FILE *f)
{
	char buf[LINE_MAX_LEN];
	if (!fgets(buf, sizeof(buf), f)) {
		if (feof(f))
			return NULL;
		fprintf(stderr, "fgets");
	}
	size_t l = strlen(buf);
	if(l <= 0) {
		return NULL;
	}
	if (buf[l - 1] != '\n' && !feof(f))
		fprintf(stderr, "input line longer than %d bytes", sizeof(buf));
	char *str = malloc(l + 1);
	if (!str)
		fprintf(stderr, "malloc");
	strncpy(str, buf, l);
	str[l] = '\0';
	return str;
}

enum err {
	E_NONE,
	E_BAD_ADDR,
	E_BAD_CMD_SUFFIX,
	E_UNEXP_CMD_SUFFIX,
	E_UNEXP_ADDR,
	E_CMD,
	E_INPUT,
	E_NO_FN,
	E_BAD_OF,
	E_BUF_MODIFIED,
};

struct buffer {
	struct line *first;
	long int nlines;
	long int cur_line;
	int print_errors;
	int changed;
};

static void buffer_init(struct buffer *b)
{
	b->first = NULL;
	b->nlines = 0;
	b->cur_line = 1;
	b->print_errors = 0;
	b->changed = 0;
}

static void buffer_init_load(struct buffer *b, FILE *f)
{
	size_t l = 0;
	struct line *first = NULL, *prev = NULL;
	char *text;
	b->nlines = 0;
	while ((text = read_line(f)) != NULL) {
		if (l > LINE_MAX_LEN)
			fprintf(stderr, "input line too long at %zu bytes", l);
		struct line *l = malloc(sizeof(*l));
		if (!first)
			first = l;
		l->text = text;
		l->prev = prev;
		if (prev)
			prev->next = l;
		prev = l;
		l->next = NULL;
		b->nlines++;
	}
	b->first = first;
	b->cur_line = b->nlines;
	b->print_errors = 0;
	b->changed = 0;
}

static void buffer_free(struct buffer *buf)
{
	struct line *line = buf->first, *prev;
	while (line) {
		free(line->text);
		prev = line;
		line = line->next;
		free(prev);
	}
}

static void xusage(int eval, char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "usage: ed [file]\n");
	va_end(va);
	exit(eval);
}

struct cmd {
	char cmd;
	long int a;
	long int b;
	int addr_given;
	char *fname;
};

static long int parse_lineno(char *str, char **endp, struct buffer *buf)
{
	if (*str != '-' && !isdigit(*str) && *str != '.' && *str != '$') {
		*endp = str;
		return 0;
	}
	long int n = strtol(str, endp, 10);
	if (*endp != str) {
		if (n < 0)
			return buf->cur_line + n;
		return n;
	}
	if (*str == '.') {
		*endp = str + 1;
		return buf->cur_line;
	}
	if (*str == '$') {
		*endp = str + 1;
		return buf->nlines;
	}
	*endp = str;
	return buf->cur_line;
}

static int parse_command(struct buffer *buf, struct cmd *cmd)
{
	enum err e = E_NONE;
	cmd->b = 0;
	cmd->addr_given = 0;
	cmd->fname = NULL;
	char *str = read_line(stdin), *ostr = str;
	if (!str) { /* <=> EOF reached on stdin */
		cmd->cmd = 'q';
		cmd->addr_given = 0;
		return E_NONE;
	}
	if (*str == ' ') {
		e = E_BAD_ADDR;
		goto out_err;
	}
	if (*str == 'w') {
		cmd->cmd = 'w';
		if (str[1] != ' ' && str[1] != '\n') {
			e = E_UNEXP_CMD_SUFFIX;
			goto out_err;
		}
		if (str[1] == ' ' && str[2] != '\n') {
			size_t len = strlen(str + 2);
			cmd->fname = malloc(len + 1);
			strncpy(cmd->fname, str + 2, len + 1);
			assert(cmd->fname[len - 1] == '\n');
			cmd->fname[len - 1] = '\0';
		}
		return E_NONE;
	}
	char *endp;
	int have_comma = (index(str, ',') != NULL);
	cmd->a = parse_lineno(str, &endp, buf);
	if (endp != str) {
		str = endp;
		if (*str == ',') {
			str++;
			cmd->b = parse_lineno(str, &endp, buf);
			if (endp == str) {
				e = E_BAD_ADDR;
				goto out_err;
			}
			str = endp;
		} else if (have_comma) {
			e = E_BAD_ADDR;
			goto out_err;
		} else {
			cmd->b = cmd->a;
		}
		cmd->addr_given = 1;
	} else if (have_comma) {
		e = E_BAD_ADDR;
		goto out_err;
	}
	cmd->cmd = *str++;
	if (*str != '\0' && *str != '\n')
		e = E_BAD_CMD_SUFFIX;
out_err:
	free(ostr);
	return e;
}

static void buffer_print_range(struct buffer *buf, long int a, long int b,
                               int with_line_numbers)
{
	struct line *line;
	long int line_no = 1;
	for (line = buf->first; line != NULL; line = line->next, line_no++) {
		if (line_no > b)
			break;
		if (line_no < a)
			continue;
		if (with_line_numbers)
			printf("%d\t%s", line_no, line->text);
		else
			printf("%s", line->text);
	}
}

static void buffer_cut_range(struct buffer *buf, long int a, long int b)
{
	struct line *cur, *next = NULL;
	long int line_no = 1;
	for (cur = buf->first; cur != NULL; cur = next, line_no++) {
		next = cur->next;
		if (line_no > b)
			break;
		if (line_no < a)
			continue;
		if (cur->prev)
			cur->prev->next = next;
		else
			buf->first = next;
		if (next)
			next->prev = cur->prev;
		buf->nlines--;
		buf->changed = 1;
		free(cur->text);
		free(cur);
	}
}

static void buffer_insert(struct buffer *buf, long int before, struct line *l)
{
	struct line *line, *last = NULL;
	long int line_no = 1;
	for (line = buf->first; line != NULL; line = line->next, line_no++) {
		last = line;
		if (line_no == before)
			break;
	}
	if (line && line_no == before && buf->nlines != 0)
		line = line->prev;
	else
		line = last;
	if (line) {
		if (line->next)
			line->next->prev = l;
		l->next = line->next;
		l->prev = line;
		line->next = l;
	} else {
		/* TODO Using a list with a head, we wouldn't need this branch. */
		l->next = buf->first;
		if (buf->first)
			buf->first->prev = l;
		buf->first = l;
	}
	buf->nlines++;
	buf->changed = 1;
}

static enum err buf_write(struct buffer *buf, const char *fname)
{
	FILE *f = fopen(fname, "w");
	size_t written = 0;
	if (!f) {
		fprintf(stderr, "%s: %s\n", fname, strerror(errno));
		return E_BAD_OF;
	}
	for (struct line *l = buf->first; l != NULL; l = l->next)
		written += (size_t)fprintf(f, "%s", l->text);
	fclose(f);
	buf->changed = 0;
	printf("%d\n", written);
	return E_NONE;
}

static const char *buf_err_str(enum err e)
{
	switch (e) {
	case E_NONE:
		return "No error";
	case E_BAD_ADDR:
		return "Invalid address";
	case E_BAD_CMD_SUFFIX:
		return "Invalid command suffix";
	case E_UNEXP_CMD_SUFFIX:
		return "Unexpected command suffix";
	case E_UNEXP_ADDR:
		return "Unexpected address";
	case E_CMD:
		return "Unknown command";
	case E_INPUT:
		return "Cannot open input file";
	case E_NO_FN:
		return "No current filename";
	case E_BAD_OF:
		return "Cannot open output file";
	case E_BUF_MODIFIED:
		return "Warning: buffer modified";
	default:
		assert(0);
	}
	return "Unknown error";
}

static int buffer_validate_addr(struct buffer *buf, struct cmd *cmd)
{
	long int addr_min = 1;
	if (cmd->cmd == 'i')
		addr_min = 0;
	return cmd->a <= cmd->b && cmd->a >= addr_min &&
	       cmd->a <= buf->nlines && cmd->b >= addr_min &&
	       cmd->b <= buf->nlines;
}

void main(int argc, char *argv[]) {
	enum err err = E_NONE, err2, last_err = E_NONE;
	if (argc > 2)
		xusage(1, "");

	char *fname = NULL;
	int fname_needs_free = 0;
	if (argc == 2) {
		fname = argv[1];
		if (fname[0] == '-')
			xusage(1, "ed: illegal option -- %s\n", fname + 1);
	}

	struct buffer buf;
	if (fname) {
		FILE *f = fopen(fname, "r");
		if (!f) {
			fprintf(stderr, "%s: %s\n", fname, strerror(errno));
			buffer_init(&buf);
			last_err = E_INPUT;
		} else {
			buffer_init_load(&buf, f);
			fseek(f, 0, SEEK_END);
			printf("%d\n", ftell(f));
			fclose(f);
		}
	} else {
		buffer_init(&buf);
	}

	struct cmd cmd;
	int prev_was_q = 0;
	long int before;
	char *ofname;
	for (;;) {
		err2 = parse_command(&buf, &cmd);
		if (!cmd.addr_given) {
			if (cmd.cmd == '\n')
				cmd.a = cmd.b = buf.cur_line + 1;
			else
				cmd.a = cmd.b = buf.cur_line;
		}
		if (err2 != E_NONE && err2 != E_BAD_CMD_SUFFIX) {
			err = err2;
			goto skip_cmd;
		}
		if ((cmd.addr_given || cmd.cmd == '\n') &&
		    !buffer_validate_addr(&buf, &cmd)) {
			err = E_BAD_ADDR;
			goto skip_cmd;
		}
		switch (cmd.cmd) {
		case '\n':
		case 'n':
		case 'p':
		case 'H':
		case 'h':
		case 'q':
		case 'i':
		case 'd':
		case 'w':
			break;
		default:
			err = E_CMD;
			goto skip_cmd;
		}
		if (err2 == E_BAD_CMD_SUFFIX) {
			err = err2;
			goto skip_cmd;
		}
		switch (cmd.cmd) {
		case '\n':
		case 'n':
		case 'p':
			buffer_print_range(&buf, cmd.a, cmd.b, cmd.cmd == 'n');
			buf.cur_line = cmd.b;
			err = E_NONE;
			break;
		case 'H':
			if (cmd.addr_given) {
				err = E_UNEXP_ADDR;
			} else {
				buf.print_errors = !buf.print_errors;
				if (buf.print_errors && last_err != E_NONE)
					puts(buf_err_str(last_err));
				prev_was_q = 0;
				continue;
			}
			break;
		case 'h':
			if (cmd.addr_given) {
				err = E_UNEXP_ADDR;
				break;
			} else if (last_err != E_NONE)
				puts(buf_err_str(last_err));
			prev_was_q = 0;
			continue;
		case 'q':
			if (cmd.addr_given)
				err = E_UNEXP_ADDR;
			else if (buf.changed && !prev_was_q)
				err = E_BUF_MODIFIED;
			else
				goto quit;
			break;
		case 'd':
			buffer_cut_range(&buf, cmd.a, cmd.b);
			if (cmd.a <= buf.nlines)
				buf.cur_line = cmd.a;
			else
				buf.cur_line = buf.nlines;
			if (!buf.cur_line)
				buf.cur_line = 1;
			err = E_NONE;
			break;
		case 'i':
			before = cmd.a;
			if (before == 0)
				before = 1;
			while (1) {
				char *input = read_line(stdin);
				if (strcmp(input, ".\n") == 0) {
					free(input);
					break;
				}
				struct line *l = malloc(sizeof(*l));
				l->next = l->prev = NULL;
				l->text = input;
				buffer_insert(&buf, before, l);
				before++;
			}
			buf.cur_line = cmd.a;
			if (before - cmd.a > 1)
				buf.cur_line += before - cmd.a - 1;
			err = E_NONE;
			break;
		case 'w':
			ofname = cmd.fname;
			if (!ofname)
				ofname = (char *)fname;
			if (!ofname) {
				err = E_NO_FN;
				break;
			}
			if (!fname) {
				fname = malloc(sizeof(ofname) + 1);
				strcpy(fname, ofname);
				fname_needs_free = 1;
			}
			err = buf_write(&buf, ofname);
			if (cmd.fname)
				free(cmd.fname);
			break;
		default:
			err = E_CMD;
		}
		prev_was_q = (cmd.cmd == 'q');
		if (err == E_NONE && err2 != E_NONE)
			err = err2; // Just a hack to satisfy the assignment.
skip_cmd:
		if (err != E_NONE) {
			puts("?");
			if (buf.print_errors)
				puts(buf_err_str(err));
			last_err = err;
		}
	}
quit:
	buffer_free(&buf);
	if (fname_needs_free)
		free(fname);
	exit(last_err == E_NONE || last_err == E_INPUT ? 0 : 1);
}
