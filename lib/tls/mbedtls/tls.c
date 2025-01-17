/*
 * libwebsockets - mbedTLS-specific lws apis
 *
 * Copyright (C) 2010 - 2019 Andy Green <andy@warmcat.com>
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
#include "tls/mbedtls/private.h"

void
lws_tls_err_describe(void)
{
}

int
lws_context_init_ssl_library(const struct lws_context_creation_info *info)
{
	lwsl_info(" Compiled with MbedTLS support\n");

	if (!lws_check_opt(info->options, LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT))
		lwsl_info(" SSL disabled: no "
			  "LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT\n");

	return 0;
}

mbedtls_x509_crt *
lws_ssl_ctx_get_mbedtls_x509_crt(lws_ssl_ctx *ssl_ctx) {
	return ssl_ctx_get_mbedtls_x509_crt((SSL_CTX*)ssl_ctx);
}

int lws_ssl_ctx_set_mbedtls_x509_crt(lws_ssl_ctx *ssl_ctx, mbedtls_x509_crt *crt) {
	return ssl_ctx_set_mbedtls_x509_crt((SSL_CTX*)ssl_ctx, crt) ? 0 : -1;
}

mbedtls_pk_context *
lws_ssl_ctx_get_mbedtls_key(lws_ssl_ctx *ssl_ctx) {
	return ssl_ctx_get_mbedtls_key((SSL_CTX*)ssl_ctx);

}

int lws_ssl_ctx_set_mbedtls_key(lws_ssl_ctx *ssl_ctx, mbedtls_pk_context *key) {
    return ssl_ctx_set_mbedtls_key((SSL_CTX*)ssl_ctx, key) ? 0 : -1;
}
