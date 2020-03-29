struct winsize {
    uint16 ws_row;
    uint16 ws_col;
    uint16 ws_xpixel;
    uint16 ws_ypixel;
};

#define	NCCS      20
#define	TCSANOW	  0
#define	TCSADRAIN 1
#define	TCSAFLUSH 2

typedef uint32  tcflag_t;
typedef uint8   cc_t;
typedef uint32  speed_t;

struct termios {
    tcflag_t c_iflag;    // input
    tcflag_t c_oflag;    // output
    tcflag_t c_cflag;    // control
    tcflag_t c_lflag;    // local
    cc_t     c_cc[NCCS]; // control characters
};


//Descriptions from: https://www.mkssoftware.com/docs/man5/struct_termios.5.asp
//Input flags
#define	IGNBRK 0x001     // Ignore break condition
#define	BRKINT 0x002     // Signal interrupt on break
#define	IGNPAR 0x004     // Ignore characters with parity errors
#define	PARMRK 0x008     // Mark parity errors
#define	INPCK  0x010     // Enable input parity check
#define	ISTRIP 0x020     // Strip character
#define	INLCR  0x040     // Map NL to CR on input
#define	IGNCR  0x080     // Ignore CR
#define	ICRNL  0x100     // Map CR to NL on input
#define	IXON   0x200     // Enable start/stop output control
#define	IXOFF  0x400     // Enable start/stop input control

//Output flags
#define OPOST  0x001     // Perform output processing

//Local flags
#define ICANON 0x002     // Canonical input (erase and kill processing)
#define ECHO   0x004     // Enable echo

//Non-Canonical Modes
#define VMIN   9         // MIN value
#define VTIME  16        // TIME value

char *ttyname(int32 fd);
