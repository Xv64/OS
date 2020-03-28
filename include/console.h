struct winsize {
    uint16 ws_row;
    uint16 ws_col;
    uint16 ws_xpixel;
    uint16 ws_ypixel;
};

#define	NCCS    20
typedef uint32  tcflag_t;
typedef uint8   cc_t;
typedef uint32  speed_t;

struct termios {
    tcflag_t c_iflag;   /* input              */
    tcflag_t c_oflag;   /* output             */
    tcflag_t c_cflag;   /* control            */
    tcflag_t c_lflag;   /* local              */
    cc_t     c_cc[NCCS];/* control characters */
};
