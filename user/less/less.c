// Micro-less
// Copyright (c) 2019 Alexander Mukhin
// MIT License
// Source: https://github.com/aimukhin/uless/blob/879d9940d631b94fc7c1d3d52d4e44aff357b952/uless.c

// maximal line width
#define BUFSZ 1024

// ANSI escape sequences
#define INV "\033[7m"
#define NORM "\033[m"
#define CLRLINE "\033[2K\r"

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <string.h>
#include <stdlib.h>

// line structure
struct line {
	int no; // line number
	int len; // length
	char *cs; // characters
	struct line *prev; // pointer to the previous line
	struct line *next; // pointer to the next line
};

// append line to the list
struct line *
append (struct line *rl, char *buf, int sz) {
	struct line *lptr; // line pointer
	// allocate new line structure
	lptr=malloc(sizeof(struct line));
	// copy line and add terminating 0
	lptr->len=sz;
	lptr->cs=malloc(sz+1);
	memcpy(lptr->cs,buf,sz);
	lptr->cs[sz]=0;
	// append line to the list
	lptr->next=NULL;
	lptr->prev=rl;
	if (rl) {
		lptr->no=rl->no+1;
		rl->next=lptr;
	} else
		lptr->no=1;
	return lptr;
}

// move pointer n lines before lptr
struct line *
sub (struct line *lptr, int n) {
	int i=0;
	while (lptr&&i<n) {
		if (lptr->prev==NULL)
			break;
		lptr=lptr->prev;
		++i;
	}
	return lptr;
}

// move pointer n lines after lptr
struct line *
add (struct line *lptr, int n) {
	int i=0;
	while (lptr&&i<n) {
		if (lptr->next==NULL)
			break;
		lptr=lptr->next;
		++i;
	}
	return lptr;
}

// clear status line
void
clrsl (void) {
	write(1,CLRLINE,strlen(CLRLINE));
}

// print status line
void
prtsl (char *s) {
	clrsl();
	write(1,INV,strlen(INV));
	write(1,s,strlen(s));
	write(1,NORM,strlen(NORM));
}

// print line and highlight matches
void
prthl (char *line, char *sub, int n) {
	char buf[BUFSZ+1]; // buffer
	char *b; // pointer to buffer
	char *m; // pointer to match
	// make buffer contain a null-terminated string
	strncpy(buf,line,n);
	buf[n+1]=0;
	b=buf;
	// loop
	do {
		// find match
		if (*sub==0)
			// empty search string
			m=NULL;
		else
			// non-empty search string
			m=strstr(b,sub);
		// print string and match
		if (m==NULL)
			// no match
			// print rest of line
			write(1,b,n-(b-buf));
		else {
			// match
			// print line before match
			b+=write(1,b,m-b);
			// print match highlighted
			write(1,INV,strlen(INV));
			b+=write(1,b,strlen(sub));
			write(1,NORM,strlen(NORM));
		}
		// repeat until no more matches left
	} while (m);
}

// print n lines before lptr
void
prt (struct line *lptr, int n, int eof, int w, int h, int l, char *sstr) {
	int i; // counter
	int prtlen; // number of characters to be printed
	int ateof; // last line is at EOF flag
	int sn,en; // start and end line numbers
	char sl[256]; // status line
	// is last line at EOF?
	ateof=lptr->next==NULL&&eof;
	// find end and start line numbers
	en=lptr->no;
	sn=(en-h+1)>0?en-h+1:1;
	// find start line pointer
	lptr=sub(lptr,n-1);
	// erase old status line
	clrsl();
	// print lines
	for (i=0; i<n; ++i) {
		if (lptr) {
			if (lptr->len-l>0) {
				prtlen=lptr->len-l;
				if (prtlen>w) {
					prthl(lptr->cs+l,sstr,w-1);
					// continuation sign
					write(1,INV,strlen(INV));
					write(1,">",1);
					write(1,NORM,strlen(NORM));
				} else
					prthl(lptr->cs+l,sstr,prtlen);
			}
			write(1,"\n",1);
			lptr=lptr->next;
		} else
			write(1,"~\n",2);
	}
	// prepare and print status line
	snprintf(sl,256,"Lines %d-%d%s",sn,en,ateof?" (EOF)":"");
	prtsl(sl);
}

// search
struct line *
search (struct line *lptr, char *s, int fwd) {
	while (lptr) {
		if (strstr(lptr->cs,s))
			return lptr;
		if (fwd)
			lptr=lptr->next;
		else
			lptr=lptr->prev;
	}
	return NULL;
}

int
main (void) {
	int w,h; // window width and height
	int l=0; // left margin
	struct winsize winsz; // window size structure
	int termfd; // terminal file descriptor
	struct termios t,t2; // terminal structures
	struct pollfd pollfds[2]; // poll descriptors
	char c; // current character
	int n; // number of characters read
	char buf[BUFSZ]; // line buffer
	struct line *vfl=NULL; // very first line
	struct line *rl=NULL; // last read line
	struct line *dl=NULL; // last displayed line
	int cl,cc; // current line and character numbers
	int ltr; // lines to read
	int eof=0; // end-of-file reached flag
	int hstep; // horizontal scroll step
	char sstr[BUFSZ]=""; // search string
	int sfwd; // search forward flag
	struct line *fl=NULL; // last found line
	// find w and h
	ioctl(1,TIOCGWINSZ,&winsz);
	// -1 because some terminals add new line after last character
	w=winsz.ws_col-1;
	// -1 to save space for status line
	h=winsz.ws_row-1;
	if (w<=0||h<=0) {
		// give up
		fprintf(stderr,"Bad terminal size %dx%d\n",w+1,h+1);
		return 1;
	}
	// set hstep
	hstep=w/3;
	// open tty
	termfd=open(ttyname(1),O_RDONLY);
	if (termfd==-1)
		// stdout is not a terminal
		return 2;
	// put it to non-canonical mode
	tcgetattr(termfd,&t);
	t2=t;
	t2.c_lflag&=~(ICANON|ECHO);
	t2.c_cc[VMIN]=1;
	t2.c_cc[VTIME]=0;
	tcsetattr(termfd,TCSANOW,&t2);
	// prepare pollfd structures
	pollfds[0].fd=0; // stdin
	pollfds[0].events=POLLIN|POLLHUP;
	pollfds[1].fd=termfd; // tty
	pollfds[1].events=POLLIN;
	// our first goal is to read a screenful of lines
	ltr=h;
	cl=cc=0;
	// main loop
	while (1) {
		// wait for events
		poll(pollfds,2,-1);
		// process events
		if (pollfds[1].revents&POLLIN) {
			// read from tty
			read(termfd,&c,1);
			if (c=='q')
				break;
			if (c==' ') {
				if (dl) {
					if (dl->next==NULL) {
						if (!eof) {
							// read next screenful
							ltr=h;
							cl=cc=0;
						}
					} else {
						// page down
						dl=add(dl,h);
						prt(dl,h,eof,w,h,l,sstr);
					}
				}
			}
			if (c=='b') {
				if (dl) {
					// page up
					if (dl->no>2*h)
						dl=sub(dl,h);
					else if (dl->no>h)
						dl=sub(dl,dl->no-h);
					else
						continue;
					prt(dl,h,eof,w,h,l,sstr);
				}
			}
			if (c=='j'||c=='\n') {
				if (dl) {
					if (dl->next==NULL) {
						if (!eof) {
							// read next line
							ltr=1;
							cl=cc=0;
						}
					} else {
						// scroll down
						dl=dl->next;
						prt(dl,1,eof,w,h,l,sstr);
					}
				}
			}
			if (c=='k') {
				if (dl&&dl->no>h) {
					// scroll up
					dl=dl->prev;
					prt(dl,h,eof,w,h,l,sstr);
				}
			}
			if (c=='G') {
				if (rl) {
					// go to the last line
					dl=rl;
					prt(dl,h,eof,w,h,l,sstr);
				}
			}
			if (c=='g') {
				if (vfl) {
					// go to the first line
					dl=add(vfl,h-1);
					prt(dl,h,eof,w,h,l,sstr);
				}
			}
			if (c=='h') {
				if (dl&&l>0) {
					// scroll left
					l-=hstep;
					if (l<0) l=0;
					prt(dl,h,eof,w,h,l,sstr);
				}
			}
			if (c=='l') {
				if (dl&&l<BUFSZ-2) {
					// scroll right
					l+=hstep;
					if (l>BUFSZ-2) l=BUFSZ-2;
					prt(dl,h,eof,w,h,l,sstr);
				}
			}
			if (c=='/') {
				// search
				if (rl==NULL)
					// nowhere to search
					continue;
				// erase status line
				clrsl();
				// write '/'
				write(1,"/",1);
				// read search string
				cc=0;
				while (cc<BUFSZ) {
					read(termfd,&c,1);
					if (c=='\n') {
						buf[cc++]=0;
						break;
					} else if (c==t2.c_cc[VERASE]) {
						if (cc) {
							write(1,"\b \b",3);
							--cc;
						}
					} else {
						buf[cc++]=c;
						write(1,&c,1);
					}
				}
				if (buf[0]==0) {
					// no search string given
					prtsl("Search cancelled");
					continue;
				}
				// copy search string
				strncpy(sstr,buf,cc);
				// search forward
				sfwd=1;
				fl=search(sub(dl,h-1),sstr,sfwd);
				if (fl) {
					// found
					// redraw (to clear old highlights)
					// and show match on bottom
					if (fl->no>dl->no) dl=fl;
					prt(dl,h,eof,w,h,l,sstr);
				} else {
					// not found
					prtsl("No matches below");
				}
			}
			if (c=='n'||c=='N') {
				if (sstr[0]==0)
					// no search string given
					continue;
				if (c=='n'&&!sfwd) {
					// change direction
					sfwd=1;
				} else if (c=='N'&&sfwd) {
					// change direction
					sfwd=0;
				}
				// search for the next match
				if (sfwd)
					// below display
					fl=search(add(dl,1),sstr,sfwd);
				else
					// above display
					fl=search(sub(dl,h),sstr,sfwd);
				if (fl) {
					// found
					if (sfwd) {
						// show match on bottom
						int d=fl->no-dl->no;
						if (d>h) d=h;
						dl=fl;
						prt(dl,d,eof,w,h,l,sstr);
					} else {
						// show match on top
						dl=add(fl,h-1);
						prt(dl,h,eof,w,h,l,sstr);
					}
				} else {
					// not found
					if (sfwd)
						prtsl("No matches below");
					else
						prtsl("No matches above");
				}
			}
		} else if (pollfds[0].revents&POLLIN) {
			// read from stdin
			n=read(0,&c,1);
			if (n==0) {
				// stdin reached EOF
				if (rl==NULL&&cc==0)
					// stdin was an empty file
					// nothing to show
					break;
				if (cc) {
					// last line was without \n
					rl=append(rl,buf,cc);
					if (vfl==NULL) vfl=rl;
					++cl;
				}
				// mask stdin events
				pollfds[0].fd=-1;
				// set flag
				eof=1;
				// enough lines?
				if (cl<=ltr) {
					// set pointer
					dl=rl;
					// print new lines
					prt(dl,h,eof,w,h,l,sstr);
				}
				// skip to the next event
				continue;
			}
			if (cc==BUFSZ-1) {
				// ignore rest of line
				if (c!='\n')
					continue;
			}
			if (c!='\n') {
				// save character
				buf[cc++]=c;
			} else {
				// got full line
				rl=append(rl,buf,cc);
				if (vfl==NULL) vfl=rl;
				++cl;
				// goal reached?
				if (cl==ltr) {
					// set pointer
					dl=rl;
					// print new lines
					prt(dl,cl,eof,w,h,l,sstr);
				}
				// clear character count for the next line
				cc=0;
			}
		} else if (pollfds[0].revents&POLLHUP) {
			// stdin reached EOF
			if (rl==NULL&&cc==0)
				// stdin was an empty file
				// nothing to show
				break;
			if (cc) {
				// last line was without \n
				rl=append(rl,buf,cc);
				if (vfl==NULL) vfl=rl;
				++cl;
			}
			// mask stdin events
			pollfds[0].fd=-1;
			// set flag
			eof=1;
			// enough lines?
			if (cl<=ltr) {
				// set pointer
				dl=rl;
				// print new lines
				prt(dl,h,eof,w,h,l,sstr);
			}
		}
	}
	// restore terminal
	tcsetattr(termfd,TCSANOW,&t);
	// clear status line
	clrsl();
	// done
	return 0;
}
