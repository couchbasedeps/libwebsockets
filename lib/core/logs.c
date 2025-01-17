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
 */

#include "core/private.h"

#ifdef LWS_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if defined(LWS_PLAT_OPTEE)
void lwsl_emit_optee(int level, const char *line);
#endif

int log_level = LLL_ERR | LLL_WARN | LLL_NOTICE;
static void (*lwsl_emit)(int level, const char *line)
#ifndef LWS_PLAT_OPTEE
	= lwsl_emit_stderr
#else
	= lwsl_emit_optee;
#endif
	;
#ifndef LWS_PLAT_OPTEE
static const char * const log_level_names[] = {
	"ERR",
	"WARN",
	"NOTICE",
	"INFO",
	"DEBUG",
	"PARSER",
	"HEADER",
	"EXTENSION",
	"CLIENT",
	"LATENCY",
	"USER",
	"THREAD",
	"?",
	"?"
};
#endif

LWS_VISIBLE int
lwsl_timestamp(int level, char *p, int len)
{
#ifndef LWS_PLAT_OPTEE
#ifndef _WIN32_WCE
	time_t o_now = time(NULL);
#endif
	unsigned long long now;
	struct tm *ptm = NULL;
#ifndef WIN32
	struct tm tm;
#endif
	int n;

#ifndef _WIN32_WCE
#ifdef WIN32
	ptm = localtime(&o_now);
#else
	if (localtime_r(&o_now, &tm))
		ptm = &tm;
#endif
#endif
	p[0] = '\0';
	for (n = 0; n < LLL_COUNT; n++) {
		if (level != (1 << n))
			continue;
		now = lws_time_in_microseconds() / 100;
		if (ptm)
			n = lws_snprintf(p, len,
				"[%04d/%02d/%02d %02d:%02d:%02d:%04d] %s: ",
				ptm->tm_year + 1900,
				ptm->tm_mon + 1,
				ptm->tm_mday,
				ptm->tm_hour,
				ptm->tm_min,
				ptm->tm_sec,
				(int)(now % 10000), log_level_names[n]);
		else
			n = lws_snprintf(p, len, "[%llu:%04d] %s: ",
					(unsigned long long) now / 10000,
					(int)(now % 10000), log_level_names[n]);
		return n;
	}
#else
	p[0] = '\0';
#endif

	return 0;
}

#ifndef LWS_PLAT_OPTEE
static const char * const colours[] = {
	"[31;1m", /* LLL_ERR */
	"[36;1m", /* LLL_WARN */
	"[35;1m", /* LLL_NOTICE */
	"[32;1m", /* LLL_INFO */
	"[34;1m", /* LLL_DEBUG */
	"[33;1m", /* LLL_PARSER */
	"[33m", /* LLL_HEADER */
	"[33m", /* LLL_EXT */
	"[33m", /* LLL_CLIENT */
	"[33;1m", /* LLL_LATENCY */
	"[30;1m", /* LLL_USER */
	"[31m", /* LLL_THREAD */
};

static char tty;

LWS_VISIBLE void
lwsl_emit_stderr(int level, const char *line)
{
	char buf[50];
	int n, m = LWS_ARRAY_SIZE(colours) - 1;

	if (!tty)
		tty = isatty(2) | 2;
	lwsl_timestamp(level, buf, sizeof(buf));

	if (tty == 3) {
		n = 1 << (LWS_ARRAY_SIZE(colours) - 1);
		while (n) {
			if (level & n)
				break;
			m--;
			n >>= 1;
		}
		fprintf(stderr, "%c%s%s%s%c[0m", 27, colours[m], buf, line, 27);
	} else
		fprintf(stderr, "%s%s", buf, line);
}

LWS_VISIBLE void
lwsl_emit_stderr_notimestamp(int level, const char *line)
{
	int n, m = LWS_ARRAY_SIZE(colours) - 1;

	if (!tty)
		tty = isatty(2) | 2;

	if (tty == 3) {
		n = 1 << (LWS_ARRAY_SIZE(colours) - 1);
		while (n) {
			if (level & n)
				break;
			m--;
			n >>= 1;
		}
		fprintf(stderr, "%c%s%s%c[0m", 27, colours[m], line, 27);
	} else
		fprintf(stderr, "%s", line);
}

#endif

#if !(defined(LWS_PLAT_OPTEE) && !defined(LWS_WITH_NETWORK))
LWS_VISIBLE void _lws_logv(int filter, const char *format, va_list vl)
{
	static char buf[256];
	int n;

	if (!(log_level & filter))
		return;

	n = vsnprintf(buf, sizeof(buf) - 1, format, vl);
	(void)n;
	/* vnsprintf returns what it would have written, even if truncated */
	if (n > (int)sizeof(buf) - 1) {
		n = sizeof(buf) - 5;
		buf[n++] = '.';
		buf[n++] = '.';
		buf[n++] = '.';
		buf[n++] = '\n';
		buf[n] = '\0';
	}
	if (n > 0)
		buf[n] = '\0';
	lwsl_emit(filter, buf);
}

LWS_VISIBLE void _lws_log(int filter, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	_lws_logv(filter, format, ap);
	va_end(ap);
}
#endif
LWS_VISIBLE void lws_set_log_level(int level,
				   void (*func)(int level, const char *line))
{
	log_level = level;
	if (func)
		lwsl_emit = func;
}

LWS_VISIBLE int lwsl_visible(int level)
{
	return log_level & level;
}

LWS_VISIBLE void
lwsl_hexdump_level(int hexdump_level, const void *vbuf, size_t len)
{
	unsigned char *buf = (unsigned char *)vbuf;
	unsigned int n;

	if (!lwsl_visible(hexdump_level))
		return;

	if (!len) {
		_lws_log(hexdump_level, "(hexdump: zero length)\n");
		return;
	}

	if (!vbuf) {
		_lws_log(hexdump_level, "(hexdump: trying to dump %d at NULL)\n",
					(int)len);
		return;
	}

	_lws_log(hexdump_level, "\n");

	for (n = 0; n < len;) {
		unsigned int start = n, m;
		char line[80], *p = line;

		p += snprintf(p, 10, "%04X: ", start);

		for (m = 0; m < 16 && n < len; m++)
			p += snprintf(p, 5, "%02X ", buf[n++]);
		while (m++ < 16)
			p += snprintf(p, 5, "   ");

		p += snprintf(p, 6, "   ");

		for (m = 0; m < 16 && (start + m) < len; m++) {
			if (buf[start + m] >= ' ' && buf[start + m] < 127)
				*p++ = buf[start + m];
			else
				*p++ = '.';
		}
		while (m++ < 16)
			*p++ = ' ';

		*p++ = '\n';
		*p = '\0';
		_lws_log(hexdump_level, "%s", line);
		(void)line;
	}

	_lws_log(hexdump_level, "\n");
}

LWS_VISIBLE void
lwsl_hexdump(const void *vbuf, size_t len)
{
#if defined(_DEBUG)
	lwsl_hexdump_level(LLL_DEBUG, vbuf, len);
#endif
}

#ifdef LWS_WITH_MBEDTLS
#include "ssl_port.h"
void ssl_debug_log(const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	_lws_logv(LLL_NOTICE, format, ap);
	va_end(ap);
}
#endif