/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010-2019 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 *
 * included from libwebsockets.h
 */

/** \defgroup smtp SMTP related functions
 * ##SMTP related functions
 * \ingroup lwsapi
 *
 * These apis let you communicate with a local SMTP server to send email from
 * lws.  It handles all the SMTP sequencing and protocol actions.
 *
 * Your system should have postfix, sendmail or another MTA listening on port
 * 25 and able to send email using the "mail" commandline app.  Usually distro
 * MTAs are configured for this by default.
 *
 * It runs via its own libuv events if initialized (which requires giving it
 * a libuv loop to attach to).
 *
 * It operates using three callbacks, on_next() queries if there is a new email
 * to send, on_get_body() asks for the body of the email, and on_sent() is
 * called after the email is successfully sent.
 *
 * To use it
 *
 *  - create an lws_email struct
 *
 *  - initialize data, loop, the email_* strings, max_content_size and
 *    the callbacks
 *
 *  - call lws_email_init()
 *
 *  When you have at least one email to send, call lws_email_check() to
 *  schedule starting to send it.
 */
//@{
#if defined(LWS_WITH_SMTP)

typedef struct lws_smtp_client lws_smtp_client_t;
typedef struct lws_abstract lws_abstract_t;

typedef struct lws_smtp_client_info {
	void *data;

	char helo[32];	/**< Fill before init, eg, "myserver.com" */

	const lws_abstract_t *abs;	/**< abstract transport to use */
	const lws_token_map_t	*abs_tokens;   /**< transport-specific metadata
						    for this particular
						    connection */
	struct lws_vhost *vh;

	time_t retry_interval;
	time_t delivery_timeout;

	size_t email_queue_max;
	size_t max_content_size;
} lws_smtp_client_info_t;

typedef struct lws_smtp_email {
	struct lws_dll2 list;

	void *data;
	void *extra;

	time_t added;
	time_t last_try;

	const char *email_from;
	const char *email_to;
	const char *payload;

	int (*done)(struct lws_smtp_email *e, void *buf, size_t len);

	int tries;
} lws_smtp_email_t;


/**
 * lws_smtp_client_create() - Initialize a struct lws_email
 *
 * \param abs: abstract transport to use with the new SMTP client
 * \param ci: parameters describing the new SMTP client characteristics
 *
 * Prepares a struct lws_email for use ending SMTP
 */
LWS_VISIBLE LWS_EXTERN lws_smtp_client_t *
lws_smtp_client_create(const lws_smtp_client_info_t *ci);

/**
 * lws_smtp_client_alloc_email_helper() - Allocates and inits an email object
 *
 * \param payload: the email payload string, with headers and terminating .
 * \param payload_len: size in bytes of the payload string
 * \param sender: the sender name and email
 * \param recipient: the recipient name and email
 *
 * Allocates an email object and copies the payload, sender and recipient into
 * it and initializes it.  Returns NULL if OOM, otherwise the allocated email
 * object.
 *
 * Because it copies the arguments into an allocated buffer, the original
 * arguments can be safely destroyed after calling this.
 *
 * The done() callback must free the email object.  It doesn't have to free any
 * individual members.
 */
LWS_VISIBLE LWS_EXTERN lws_smtp_email_t *
lws_smtp_client_alloc_email_helper(const char *payload, size_t payload_len,
				   const char *sender, const char *recipient,
				   const char *extra, size_t extra_len, void *data,
				   int (*done)(struct lws_smtp_email *e,
					       void *buf, size_t len));

/**
 * lws_smtp_client_add_email() - Add email to the list of ones being sent
 *
 * \param c: smtp client
 * \param e: email to queue for sending on \p c
 *
 * Adds an email to the linked-list of emails to send
 */
LWS_VISIBLE LWS_EXTERN int
lws_smtp_client_add_email(lws_smtp_client_t *c, lws_smtp_email_t *e);

/**
 * lws_smtp_client_kick() - Request check for new email
 *
 * \param email: lws_smtp_client_t context to kick
 *
 * Gives smtp client a chance to move things on
 */
LWS_VISIBLE LWS_EXTERN void
lws_smtp_client_kick(lws_smtp_client_t *email);

/**
 * lws_smtp_client_destroy() - stop using the struct lws_email
 *
 * \param email: the lws_smtp_client_t context
 *
 * Stop sending email using email and free allocations
 */
LWS_VISIBLE LWS_EXTERN void
lws_smtp_client_destroy(lws_smtp_client_t **email);


#endif
//@}
