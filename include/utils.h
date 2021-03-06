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

#define UNUSED(x) 	x = x;

#define LIST_ADD(head, type, field, value)	\
	if (NULL == head) {			\
		head = malloc(sizeof(type));	\
		head->field = value;		\
		head->next = NULL;		\
	} else {				\
		type *node = head;		\
		while (NULL != node->next)	\
			node = node->next;	\
		node->next = malloc(sizeof(type));	\
		node = node->next;		\
		node->field = value;		\
		node->next = NULL;		\
	}

long util_get_time(void);
int util_diff_time_us(long time_start, long time_stop);

#endif /* __UTILS_H_ */
