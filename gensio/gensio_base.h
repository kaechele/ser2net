/*
 *  gensio - A library for abstracting stream I/O
 *  Copyright (C) 2018  Corey Minyard <minyard@acm.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GENSIO_BASE_H
#define GENSIO_BASE_H

#define container_of(ptr, type, member)			\
    ((type *)(((char *) ptr) - offsetof(type, member)))

#include <gensio/gensio.h>

struct gensio_filter;

typedef int (*gensio_ul_filter_data_handler)(void *cb_data,
					     unsigned int *rcount,
					     const unsigned char *buf,
					     unsigned int buflen);

typedef int (*gensio_ll_filter_data_handler)(void *cb_data,
					     unsigned int *rcount,
					     unsigned char *buf,
					     unsigned int buflen);

/*
 * The filter has some asynchronously generated data that it needs
 * to send, tell the gensio base to recalculate its enables.
 */
#define GENSIO_FILTER_CB_OUTPUT_READY	1

/*
 * Tell gensio base to start it's timer and call the timeout
 * at the appropriate interval.
 * timeout => data
 */
#define GENSIO_FILTER_CB_START_TIMER	2

typedef int (*gensio_filter_cb)(void *cb_data, int func, void *data);


/*
 * Set the callback function for the filter.
 *
 *  const struct gensio_filter_callbacks *cbs => func
 *  void *cb_data => data
 */
#define GENSIO_FILTER_FUNC_SET_CALLBACK		1
void gensio_filter_set_callback(struct gensio_filter *filter,
				gensio_filter_cb cb, void *cb_data);

/*
 * Is there data ready to be read from the top of the filter? 
 */
#define GENSIO_FILTER_FUNC_UL_READ_PENDING	2
bool gensio_filter_ul_read_pending(struct gensio_filter *filter);

/*
 * Is there data ready to be written out of the bottom of the filter?
 */
#define GENSIO_FILTER_FUNC_UL_WRITE_PENDING	3
bool gensio_filter_ll_write_pending(struct gensio_filter *filter);

/*
 * Is the filter expecting that data should come in the bottom?
 */
#define GENSIO_FILTER_FUNC_LL_READ_NEEDED	4
bool gensio_filter_ll_read_needed(struct gensio_filter *filter);

/*
 * Provides a way to verify keys and such after the open is complete.
 * Returning an error will abort the connection before the open is
 * returned.
 */
#define GENSIO_FILTER_FUNC_CHECK_OPEN_DONE	5
int gensio_filter_check_open_done(struct gensio_filter *filter);

/*
 * Attempt to start a connection on the filter.  Returns 0 on
 * immediate success.  Returns EINPROGRESS if the connect attempt
 * should be retried when any I/O occurs.  Returns EAGAIN if the
 * connect attempt should be retried after any I/O or when the
 * timeout occurs.
 *
 * struct timeval *timeout => data
 */
#define GENSIO_FILTER_FUNC_TRY_CONNECT		6
int gensio_filter_try_connect(struct gensio_filter *filter,
			      struct timeval *timeout);

/*
 * Attempt to disconnect the filter.  Returns 0 on immediate
 * success.  Returns EINPROGRESS if the connect attempt should be
 * retried.  Returns EAGAIN if the connect attempt should be
 * retried after any I/O or when the timeout occurs.
 *
 * struct timeval *timeout => data
 */
#define GENSIO_FILTER_FUNC_TRY_DISCONNECT	7
int gensio_filter_try_disconnect(struct gensio_filter *filter,
				 struct timeval *timeout);

/*
 * Write data into the top of the filter.  If no data is provided
 * (buf is NULL) then this will just attempt to write any pending
 * data out of the bottom of the filter into the handler.
 *
 * gensio_ul_filter_data_handler handler => func
 * void *cb_data => data
 * unsigned int *rcount => count
 * const unsigned char *buf => cbuf
 * unsigned int buflen => buflen
 */
#define GENSIO_FILTER_FUNC_UL_WRITE		8
int gensio_filter_ul_write(struct gensio_filter *filter,
			   gensio_ul_filter_data_handler handler, void *cb_data,
			   unsigned int *rcount,
			   const unsigned char *buf, unsigned int buflen);

/*
 * Write data into the bottom of the filter.  If no data is
 * provided (buf is NULL) then this will just attempt to write any
 * pending data out of the top of the filter into the handler.
 *
 * gensio_ul_filter_data_handler handler => func
 * void *cb_data => data
 * unsigned int *rcount => count
 * unsigned char *buf => buf
 * unsigned int buflen => buflen
 */
#define GENSIO_FILTER_FUNC_LL_WRITE		9
int gensio_filter_ll_write(struct gensio_filter *filter,
			   gensio_ll_filter_data_handler handler, void *cb_data,
			   unsigned int *rcount,
			   unsigned char *buf, unsigned int buflen);

/*
 * Report urgent data indication came in.
 */
#define GENSIO_FILTER_FUNC_LL_URGENT		10
void gensio_filter_ll_urgent(struct gensio_filter *filter);

/*
 * Report a timeout for a timer the base started.
 */
#define GENSIO_FILTER_FUNC_TIMEOUT		11
void gensio_filter_timeout(struct gensio_filter *filter);

/*
 * Allocate data and configure the filter.
 */
#define GENSIO_FILTER_FUNC_SETUP		12
int gensio_filter_setup(struct gensio_filter *filter);

/*
 * Reset all internal data.
 */
#define GENSIO_FILTER_FUNC_CLEANUP		13
void gensio_filter_cleanup(struct gensio_filter *filter);

/*
 * Free the filter.
 */
#define GENSIO_FILTER_FUNC_FREE			14
void gensio_filter_free(struct gensio_filter *filter);

typedef int (*gensio_filter_func)(struct gensio_filter *filter, int op,
				  const void *func, void *data,
				  unsigned int *count,
				  void *buf, const void *cbuf,
				  unsigned int buflen);

struct gensio_filter {
    gensio_filter_func func;
};

/* FIXME - make args const */
int gensio_ssl_filter_alloc(struct gensio_os_funcs *o, char *args[],
			    struct gensio_filter **rfilter);

int gensio_ssl_server_filter_alloc(struct gensio_os_funcs *o,
				   char *keyfile,
				   char *certfile,
				   char *CAfilepath,
				   unsigned int max_read_size,
				   unsigned int max_write_size,
				   struct gensio_filter **rfilter);

struct gensio_telnet_filter_callbacks {
    void (*got_sync)(void *handler_data);
    void (*got_cmd)(void *handler_data, unsigned char cmd);
    int (*com_port_will_do)(void *handler_data, unsigned char cmd);
    void (*com_port_cmd)(void *handler_data, const unsigned char *option,
			 unsigned int len);
    void (*timeout)(void *handler_data);
    void (*free)(void *handler_data);
};

struct gensio_telnet_filter_rops {
    void (*send_option)(struct gensio_filter *filter,
			const unsigned char *buf, unsigned int len);
    void (*start_timer)(struct gensio_filter *filter, struct timeval *timeout);
};

int gensio_telnet_filter_alloc(struct gensio_os_funcs *o, char *args[],
			       const struct gensio_telnet_filter_callbacks *cbs,
			       void *handler_data,
			       const struct gensio_telnet_filter_rops **rops,
			       struct gensio_filter **rfilter);

int gensio_telnet_server_filter_alloc(
		     struct gensio_os_funcs *o,
		     bool allow_rfc2217,
		     unsigned int max_read_size,
		     unsigned int max_write_size,
		     const struct gensio_telnet_filter_callbacks *cbs,
		     void *handler_data,
		     const struct gensio_telnet_filter_rops **rops,
		     struct gensio_filter **rfilter);

struct gensio_ll {
    const struct gensio_ll_ops *ops;
};

typedef void (*gensio_ll_open_done)(void *cb_data, int err, void *open_data);
typedef void (*gensio_ll_close_done)(void *cb_data, void *close_data);

struct gensio_ll_callbacks {
    unsigned int (*read_callback)(void *cb_data, int readerr,
				  unsigned char *buf,
				  unsigned int buflen);
    void (*write_callback)(void *cb_data);
    void (*urgent_callback)(void *cb_data);
};

struct gensio_ll_ops {
    void (*set_callbacks)(struct gensio_ll *ll,
			  const struct gensio_ll_callbacks *cbs,
			  void *cb_data);

    int (*write)(struct gensio_ll *ll, unsigned int *rcount,
		 const unsigned char *buf, unsigned int buflen);

    int (*raddr_to_str)(struct gensio_ll *ll, unsigned int *pos,
			char *buf, unsigned int buflen);

    int (*get_raddr)(struct gensio_ll *ll, void *addr, unsigned int *addrlen);

    int (*remote_id)(struct gensio_ll *ll, int *id);

    /*
     * Returns 0 if the open was immediate, EINPROGRESS if it was deferred,
     * and an errno otherwise.
     */
    int (*open)(struct gensio_ll *ll, gensio_ll_open_done done, void *open_data);

    /*
     * Returns 0 if the open was immediate, EINPROGRESS if it was deferred.
     * No other returns are allowed.
     */
    int (*close)(struct gensio_ll *ll, gensio_ll_close_done done,
		 void *close_data);

    void (*set_read_callback_enable)(struct gensio_ll *ll, bool enabled);

    void (*set_write_callback_enable)(struct gensio_ll *ll, bool enabled);

    void (*free)(struct gensio_ll *ll);
};

enum gensio_ll_close_state {
    GENSIO_LL_CLOSE_STATE_START,
    GENSIO_LL_CLOSE_STATE_DONE
};

struct gensio_fd_ll_ops {
    int (*sub_open)(void *handler_data,
		    int (**check_open)(void *handler_data, int fd),
		    int (**retry_open)(void *handler_data, int *fd),
		    int *fd);

    int (*raddr_to_str)(void *handler_data, unsigned int *pos,
			char *buf, unsigned int buflen);

    int (*get_raddr)(void *handler_data, void *addr, unsigned int *addrlen);

    int (*remote_id)(void *handler_data, int *id);

    /*
     * When GENSIO_LL_CLOSE_STATE_START, timeout will be NULL and the
     * return value is ignored.  Return 0.  When
     * GENSIO_LL_CLOSE_STATE_DONE, return EAGAIN to get called again
     * after next_timeout milliseconds, zero to continue the close.
     */
    int (*check_close)(void *handler_data, enum gensio_ll_close_state state,
		       struct timeval *next_timeout);

    void (*free)(void *handler_data);
};

struct gensio_ll *fd_gensio_ll_alloc(struct gensio_os_funcs *o,
				     int fd,
				     const struct gensio_fd_ll_ops *ops,
				     void *handler_data,
				     unsigned int max_read_size);

struct gensio_ll *gensio_gensio_ll_alloc(struct gensio_os_funcs *o,
					 struct gensio *child);

struct gensio *base_gensio_alloc(struct gensio_os_funcs *o,
				 struct gensio_ll *ll,
				 struct gensio_filter *filter,
				 const char *typename,
				 gensio_event cb, void *user_data);

struct gensio *base_gensio_server_alloc(struct gensio_os_funcs *o,
					struct gensio_ll *ll,
					struct gensio_filter *filter,
					const char *typename,
					void (*open_done)(struct gensio *net,
							  int err,
							  void *open_data),
					void *open_data);

struct gensio_gensio_acc_cbs {
    int (*connect_start)(void *acc_data, struct gensio *child,
			 struct gensio **new_net);
    int (*new_child)(void *acc_data, void **finish_data,
		     struct gensio_filter **filter);
    int (*finish_child)(void *acc_data, void *finish_data, struct gensio *io);
    void (*free)(void *acc_data);
};

int gensio_gensio_acceptor_alloc(struct gensio_acceptor *child,
				 struct gensio_os_funcs *o,
				 const char *typename,
				 gensio_acceptor_event cb, void *user_data,
				 const struct gensio_gensio_acc_cbs *acc_cbs,
				 void *acc_data,
				 struct gensio_acceptor **acceptor);

#endif /* GENSIO_BASE_H */
