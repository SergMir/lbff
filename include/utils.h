#ifndef __UTILS_H_
#define __UTILS_H_

#define max(a, b) ((a > b) ? (a) : (b))
#define min(a, b) ((a < b) ? (a) : (b))

#define DBG_PREFIX "lbff"

#define ERR(msg)		\
	fprintf(stderr, DBG_PREFIX "[%s:%s:%d]: %s\n", __FILE__, __FUNCTION__, __LINE__, msg);

#define ABORT			\
	ERR("Abort");		\
	abort();

#define ASSERT(cond)		\
	if (!(cond)) {		\
		ABORT		\
	}

#define BREAK(cond)				\
	if (!(cond)) {				\
		ERR("Assertion failed");	\
		break;				\
	}

#define COND_OUT(cond)				\
	if (!(cond)) {				\
		ERR("Assertion failed");	\
		goto out;			\
	}

#endif /* __UTILS_H_ */
