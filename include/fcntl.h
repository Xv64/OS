#define _BIT(x) (1 << x)

#define O_RDONLY   _BIT(1)
#define O_WRONLY   _BIT(2)
#define O_RDWR     _BIT(3)
#define O_CREATE   _BIT(4)

#define F_ERROR    (-1)
#define FNOT_READY (-2)
