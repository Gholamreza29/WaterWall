#include "boringssl_server.h"
// #include "buffer_pool.h"
// #include "managers/socket_manager.h"
#include "loggers/network_logger.h"
// #include "utils/jsonutils.h"
// #include "hssl.h"
// #include <openssl/bio.h>
// #include <openssl/err.h>
// #include <openssl/pem.h>
// #include <ssl/ssl.h>

// #define STATE(x) ((oss_server_state_t *)((x)->state))
// #define CSTATE(x) ((oss_server_con_state_t *)((((x)->line->chains_state)[self->chain_index])))
// #define CSTATE_MUT(x) ((x)->line->chains_state)[self->chain_index]
// #define isAlive(x) (CSTATE(x) != NULL)

// typedef struct oss_server_state_s
// {

//     hssl_ctx_t ssl_context;
//     char *alpns;
//     // settings

// } oss_server_state_t;

// typedef struct oss_server_con_state_s
// {

//     bool handshake_completed;
//     SSL *ssl;

//     BIO *rbio;
//     BIO *wbio;
//     int fd;

//     bool first_sent;
//     bool init_sent;

// } oss_server_con_state_t;

// static int on_alpn_select(SSL *ssl,
//                           const unsigned char **out,
//                           unsigned char *outlen,
//                           const unsigned char *in,
//                           unsigned int inlen,
//                           void *arg)
// {
//     if (inlen == 0)
//     {
//         return SSL_TLSEXT_ERR_NOACK;
//     }

//     unsigned int offset = 0;
//     while (offset < inlen)
//     {
//         LOGD("client ALPN ->  %.*s", in[offset], &(in[1 + offset]));
//         offset = offset + 1 + in[offset];

//         // TODO alpn paths
//     }
//     // selecting first alpn -_-
//     *out = in + 1;
//     *outlen = in[0];
//     return SSL_TLSEXT_ERR_OK;
//     // return SSL_TLSEXT_ERR_NOACK;
// }

// enum sslstatus
// {
//     SSLSTATUS_OK,
//     SSLSTATUS_WANT_IO,
//     SSLSTATUS_FAIL
// };

// static enum sslstatus get_sslstatus(SSL *ssl, int n)
// {
//     switch (SSL_get_error(ssl, n))
//     {
//     case SSL_ERROR_NONE:
//         return SSLSTATUS_OK;
//     case SSL_ERROR_WANT_WRITE:
//     case SSL_ERROR_WANT_READ:
//         return SSLSTATUS_WANT_IO;
//     case SSL_ERROR_ZERO_RETURN:
//     case SSL_ERROR_SYSCALL:
//     default:
//         return SSLSTATUS_FAIL;
//     }
// }

// static void cleanup(tunnel_t *self, context_t *c)
// {
//     if (CSTATE(c) != NULL)
//     {
//         SSL_free(CSTATE(c)->ssl); /* free the SSL object and its BIO's */
//         wwmGlobalFree(CSTATE(c));
//         CSTATE_MUT(c) = NULL;
//     }
// }

// static void upStream(tunnel_t *self, context_t *c)
// {
//     oss_server_state_t *state = STATE(self);

//     if (c->payload != NULL)
//     {
//         oss_server_con_state_t *cstate = CSTATE(c);
//         enum sslstatus status;
//         int n;
//         size_t len = bufLen(c->payload);

//         while (len > 0)
//         {
//             n = BIO_write(cstate->rbio, rawBuf(c->payload), len);

//             if (n <= 0)
//             {
//                 /* if BIO write fails, assume unrecoverable */
//                 reuseContextBuffer(c);
//                 goto failed;
//             }
//             shiftr(c->payload, n);
//             len -= n;

//             if (!SSL_is_init_finished(cstate->ssl))
//             {
//                 n = SSL_accept(cstate->ssl);
//                 status = get_sslstatus(cstate->ssl, n);

//                 /* Did SSL request to write bytes? */
//                 if (status == SSLSTATUS_WANT_IO)
//                     do
//                     {
//                         shift_buffer_t *buf = popBuffer(getContextBufferPool(c));
//                         size_t avail = rCap(buf);
//                         n = BIO_read(cstate->wbio, rawBuf(buf), avail);
//                         // assert(-1 == BIO_read(cstate->wbio, rawBuf(buf), avail));
//                         if (n > 0)
//                         {
//                             setLen(buf, n);
//                             context_t *answer = newContext(c->line);
//                             answer->payload = buf;
//                             self->dw->downStream(self->dw, answer);
//                             if (!isAlive(c->line))
//                             {
//                                 reuseContextBuffer(c);
//                                 destroyContext(c);
//                                 return;
//                             }
//                         }
//                         else if (!BIO_should_retry(cstate->wbio))
//                         {
//                             // If BIO_should_retry() is false then the cause is an error condition.
//                             reuseContextBuffer(c);
//                             reuseBuffer(getContextBufferPool(c), buf);
//                             goto failed;
//                         }
//                         else
//                         {
//                             reuseBuffer(getContextBufferPool(c), buf);
//                         }
//                     } while (n > 0);

//                 if (status == SSLSTATUS_FAIL)
//                 {
//                     reuseContextBuffer(c);
//                     goto failed;
//                 }

//                 if (!SSL_is_init_finished(cstate->ssl))
//                 {
//                     reuseContextBuffer(c);
//                     destroyContext(c);
//                     return;
//                 }
//                 else
//                 {
//                     LOGD("Tls handshake complete");
//                     cstate->handshake_completed = true;
//                     context_t *up_init_ctx = newContext(c->line);
//                     up_init_ctx->init = true;
//                     up_init_ctx->src_io = c->src_io;

//                     self->up->upStream(self->up, up_init_ctx);
//                     if (!isAlive(c->line))
//                     {
//                         LOGW("Openssl server: next node instantly closed the init with fin");
//                         reuseContextBuffer(c);
//                         destroyContext(c);

//                         return;
//                     }
//                     cstate->init_sent = true;
//                 }
//             }

//             /* The encrypted data is now in the input bio so now we can perform actual
//              * read of unencrypted data. */
//             shift_buffer_t *buf = popBuffer(getContextBufferPool(c));
//             shiftl(buf, 8192 / 2);
//             setLen(buf, 0);
//             do
//             {
//                 size_t avail = rCap(buf) - bufLen(buf);
//                 n = SSL_read(cstate->ssl, rawBuf(buf) + bufLen(buf), avail);
//                 if (n > 0)
//                 {
//                     setLen(buf, bufLen(buf) + n);
//                 }

//             } while (n > 0);

//             if (bufLen(buf) > 0)
//             {
//                 context_t *up_ctx = newContext(c->line);
//                 up_ctx->payload = buf;
//                 up_ctx->src_io = c->src_io;
//                 if (!(cstate->first_sent))
//                 {
//                     up_ctx->first = true;
//                     cstate->first_sent = true;
//                 }
//                 self->up->upStream(self->up, up_ctx);
//                 if (!isAlive(c->line))
//                 {
//                     reuseContextBuffer(c);
//                     destroyContext(c);
//                     return;
//                 }
//             }
//             else
//             {
//                 reuseBuffer(getContextBufferPool(c), buf);
//             }

//             status = get_sslstatus(cstate->ssl, n);

//             /* Did SSL request to write bytes? This can happen if peer has requested SSL
//              * renegotiation. */
//             if (status == SSLSTATUS_WANT_IO)
//                 do
//                 {
//                     shift_buffer_t *buf = popBuffer(getContextBufferPool(c));
//                     size_t avail = rCap(buf);

//                     n = BIO_read(cstate->wbio, rawBuf(buf), avail);
//                     if (n > 0)
//                     {
//                         setLen(buf, n);
//                         context_t *answer = newContext(c->line);
//                         answer->payload = buf;
//                         self->dw->downStream(self->dw, answer);
//                         if (!isAlive(c->line))
//                         {
//                             reuseContextBuffer(c);
//                             destroyContext(c);

//                             return;
//                         }
//                     }
//                     else if (!BIO_should_retry(cstate->wbio))
//                     {
//                         // If BIO_should_retry() is false then the cause is an error condition.
//                         reuseBuffer(getContextBufferPool(c), buf);
//                         reuseContextBuffer(c);
//                         destroyContext(c);

//                         goto failed_after_establishment;
//                     }
//                     else
//                     {
//                         reuseBuffer(getContextBufferPool(c), buf);
//                     }
//                 } while (n > 0);

//             if (status == SSLSTATUS_FAIL)
//             {
//                 reuseContextBuffer(c);
//                 goto failed_after_establishment;
//             }
//         }
//         // done with socket data
//         reuseContextBuffer(c);
//         destroyContext(c);
//     }
//     else
//     {

//         if (c->init)
//         {
//             CSTATE_MUT(c) = wwmGlobalMalloc(sizeof(oss_server_con_state_t));
//             memset(CSTATE(c), 0, sizeof(oss_server_con_state_t));
//             oss_server_con_state_t *cstate = CSTATE(c);
//             cstate->fd = hio_fd(c->src_io);
//             cstate->rbio = BIO_new(BIO_s_mem());
//             cstate->wbio = BIO_new(BIO_s_mem());

//             cstate->ssl = SSL_new(state->ssl_context);
//             SSL_set_accept_state(cstate->ssl); /* sets ssl to work in server mode. */
//             SSL_set_bio(cstate->ssl, cstate->rbio, cstate->wbio);
//             destroyContext(c);
//         }
//         else if (c->fin)
//         {
//             if (CSTATE(c)->init_sent)
//             {
//                 cleanup(self, c);
//                 self->up->upStream(self->up, c);
//             }
//             else
//             {
//                 cleanup(self, c);
//                 destroyLine(c->line);
//                 destroyContext(c);
//             }
//         }
//     }

//     return;

// failed_after_establishment:
//     context_t *fail_context_up = newContext(c->line);
//     fail_context_up->fin = true;
//     fail_context_up->src_io = c->src_io;
//     self->up->upStream(self->up, fail_context_up);
// failed:
//     context_t *fail_context = newContext(c->line);
//     fail_context->fin = true;
//     fail_context->src_io = NULL;
//     cleanup(self, c);
//     destroyContext(c);
//     self->dw->downStream(self->dw, fail_context);
//     return;
// }

// static void downStream(tunnel_t *self, context_t *c)
// {
//     oss_server_con_state_t *cstate = CSTATE(c);
//     if (c->payload != NULL)
//     {
//         // self->dw->downStream(self->dw, ctx);
//         // char buf[DEFAULT_BUF_SIZE];
//         enum sslstatus status;

//         if (!SSL_is_init_finished(cstate->ssl))
//         {
//             LOGF("How it is possible to receive data before sending init to upstream?");
//             exit(1);
//         }
//         size_t len = bufLen(c->payload);
//         while (len)
//         {
//             int n = SSL_write(cstate->ssl, rawBuf(c->payload), len);
//             status = get_sslstatus(cstate->ssl, n);

//             if (n > 0)
//             {
//                 /* consume the waiting bytes that have been used by SSL */
//                 shiftr(c->payload, n);
//                 len -= n;
//                 /* take the output of the SSL object and queue it for socket write */
//                 do
//                 {

//                     shift_buffer_t *buf = popBuffer(getContextBufferPool(c));
//                     size_t avail = rCap(buf);
//                     n = BIO_read(cstate->wbio, rawBuf(buf), avail);
//                     if (n > 0)
//                     {
//                         setLen(buf, n);
//                         context_t *dw_context = newContext(c->line);
//                         dw_context->payload = buf;
//                         dw_context->src_io = c->src_io;
//                         self->dw->downStream(self->dw, dw_context);
//                         if (!isAlive(c->line))
//                         {
//                             reuseContextBuffer(c);
//                             destroyContext(c);

//                             return;
//                         }
//                     }
//                     else if (!BIO_should_retry(cstate->wbio))
//                     {
//                         // If BIO_should_retry() is false then the cause is an error condition.
//                         reuseBuffer(getContextBufferPool(c), buf);
//                         reuseContextBuffer(c);
//                         goto failed_after_establishment;
//                     }
//                     else
//                     {
//                         reuseBuffer(getContextBufferPool(c), buf);
//                     }
//                 } while (n > 0);
//             }

//             if (status == SSLSTATUS_FAIL)
//             {
//                 reuseContextBuffer(c);
//                 goto failed_after_establishment;
//             }

//             if (n == 0)
//                 break;
//         }
//         assert(bufLen(c->payload) == 0);
//         reuseContextBuffer(c);
//         destroyContext(c);

//         return;
//     }
//     else
//     {
//         if (c->est)
//         {
//             self->dw->downStream(self->dw, c);
//             return;
//         }
//         else if (c->fin)
//         {
//             cleanup(self, c);
//             self->dw->downStream(self->dw, c);
//         }
//     }
//     return;

// failed_after_establishment:
//     context_t *fail_context_up = newContext(c->line);
//     fail_context_up->fin = true;
//     fail_context_up->src_io = NULL;
//     self->up->upStream(self->up, fail_context_up);

//     context_t *fail_context = newContext(c->line);
//     fail_context->fin = true;
//     fail_context->src_io = c->src_io;
//     cleanup(self, c);
//     destroyContext(c);
//     self->dw->downStream(self->dw, fail_context);

//     return;
// }

// static bool check_libhv()
// {
//     if (!HV_WITH_SSL)
//     {
//         LOGF("BoringSSL-Server: libhv compiled without ssl backend");
//         return false;
//     }
//     if (strcmp(hssl_backend(), "openssl") != 0)
//     {
//         LOGF("BoringSSL-Server: wrong ssl backend in libhv, expecetd: %s but found %s", "openssl", hssl_backend());
//         return false;
//     }
//     return true;
// }
// static void openSSLUpStream(tunnel_t *self, context_t *c)
// {
//     upStream(self, c);
// }
// static void openSSLPacketUpStream(tunnel_t *self, context_t *c)
// {
//     upStream(self, c);
// }
// static void openSSLDownStream(tunnel_t *self, context_t *c)
// {
//     downStream(self, c);
// }
// static void openSSLPacketDownStream(tunnel_t *self, context_t *c)
// {
//     downStream(self, c);
// }

// tunnel_t *newBoringSSLServer(node_instance_context_t *instance_info)
// {
//     if (!check_libhv())
//     {
//         return NULL;
//     }

//     oss_server_state_t *state = wwmGlobalMalloc(sizeof(oss_server_state_t));
//     memset(state, 0, sizeof(oss_server_state_t));

//     hssl_ctx_opt_t *ssl_param = wwmGlobalMalloc(sizeof(hssl_ctx_opt_t));
//     memset(ssl_param, 0, sizeof(hssl_ctx_opt_t));
//     const cJSON *settings = instance_info->node_settings_json;

//     if (!(cJSON_IsObject(settings) && settings->child != NULL))
//     {
//         LOGF("JSON Error: BoringSSLServer->settings (object field) : The object was empty or invalid");
//         return NULL;
//     }

//     if (!getStringFromJsonObject((char **)&(ssl_param->crt_file), settings, "cert-file"))
//     {
//         LOGF("JSON Error: BoringSSLServer->settings->cert-file (string field) : The data was empty or invalid");
//         return NULL;
//     }
//     if (strlen(ssl_param->crt_file) == 0)
//     {
//         LOGF("JSON Error: BoringSSLServer->settings->cert-file (string field) : The data was empty");
//         return NULL;
//     }

//     if (!getStringFromJsonObject((char **)&(ssl_param->key_file), settings, "key-file"))
//     {
//         LOGF("JSON Error: BoringSSLServer->settings->key-file (string field) : The data was empty or invalid");
//         return NULL;
//     }
//     if (strlen(ssl_param->key_file) == 0)
//     {
//         LOGF("JSON Error: BoringSSLServer->settings->key-file (string field) : The data was empty");
//         return NULL;
//     }


//     ssl_param->verify_peer = 0; // no mtls
//     ssl_param->endpoint = HSSL_SERVER;
//     state->ssl_context = hssl_ctx_new(ssl_param);

//     // dont do that with APPLE TLS -_-
//     wwmGlobalFree((char *)ssl_param->crt_file);
//     wwmGlobalFree((char *)ssl_param->key_file);
//     wwmGlobalFree(ssl_param);

//     if (state->ssl_context == NULL)
//     {
//         LOGF("Could not create node ssl context");
//         return NULL;
//     }

//     SSL_CTX_set_alpn_select_cb(state->ssl_context, on_alpn_select, NULL);

//     tunnel_t *t = newTunnel();
//     t->state = state;

//     t->upStream = &openSSLUpStream;
//     t->packetUpStream = &openSSLPacketUpStream;
//     t->downStream = &openSSLDownStream;
//     t->packetDownStream = &openSSLPacketDownStream;
//     
//     return t;
// }

// void apiBoringSSLServer(tunnel_t *self, char *msg)
// {
// }

tunnel_t *destroyBoringSSLServer(tunnel_t *self)
{
    return NULL;
}
