#pragma once
#include "api.h"


//  con (http2 stream)  <------>
//  con (http2 stream)  <------>  http2-client <------>  http2 connection (muxed con) 
//  con (http2 stream)  <------>


tunnel_t *        newHttp2Client(node_instance_context_t *instance_info);
api_result_t      apiHttp2Client(tunnel_t *self,const char *msg);
tunnel_t *        destroyHttp2Client(tunnel_t *self);
tunnel_metadata_t getMetadataHttp2Client(void);
