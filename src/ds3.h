/*
 * ******************************************************************************
 *   Copyright 2014 Spectra Logic Corporation. All Rights Reserved.
 *   Licensed under the Apache License, Version 2.0 (the "License"). You may not use
 *   this file except in compliance with the License. A copy of the License is located at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *   or in the "license" file accompanying this file.
 *   This file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *   CONDITIONS OF ANY KIND, either express or implied. See the License for the
 *   specific language governing permissions and limitations under the License.
 * ****************************************************************************
 */

#ifndef __DS3_HEADER__
#define __DS3_HEADER__

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// For windows DLL symbol exports.
#ifdef _WIN32
#    ifdef LIBRARY_EXPORTS
#        define LIBRARY_API __declspec(dllexport)
#    else
#        define LIBRARY_API __declspec(dllimport)
#    endif
#else
#    define LIBRARY_API
#endif

typedef enum {
    False, True
}ds3_bool;

typedef enum {
  HTTP_GET, HTTP_PUT, HTTP_POST, HTTP_DELETE, HTTP_HEAD
}http_verb;

typedef struct {
    char*   access_id;
    size_t  access_id_len;
    char*   secret_key;
    size_t  secret_key_len;
}ds3_creds;

typedef struct {
    char*       endpoint;
    size_t      endpoint_len;
    char*       proxy;
    size_t      proxy_len;
    uint64_t    num_redirects;
    ds3_creds*  creds; 
}ds3_client;

typedef struct _ds3_request ds3_request;

typedef struct {
    char*   creation_date;
    size_t  creation_date_size;
    char*   name;
    size_t  name_size;
}ds3_bucket;

typedef struct {
    char*   name;
    size_t  name_size;
    char*   id;
    size_t  id_size;
}ds3_owner;

typedef struct {
    char*       name;
    size_t      name_size;
    char*       etag;
    size_t      etag_size;
    uint64_t    size;
    ds3_owner*  owner;
    char*       last_modified;
    size_t      last_modified_size;
    char*       storage_class;
    size_t      storage_class_size;
}ds3_object;

typedef struct {
    ds3_bucket* buckets;
    size_t      num_buckets;
    ds3_owner*  owner;
}ds3_get_service_response;

typedef struct {
    ds3_object* objects;
    size_t      num_objects;
    char*       creation_date;
    size_t      creation_date_size;
    ds3_bool    is_truncated;
    char*       marker;
    size_t      marker_size;
    char*       delimiter;
    size_t      delimiter_size;
    uint32_t    max_keys;
    char*       name;
    size_t      name_size;
    char*       next_marker;
    size_t      next_marker_size;
    char*       prefix;
    size_t      prefix_size;
}ds3_get_bucket_response;

typedef struct {
    char*     name;
    size_t    name_size;
    uint64_t  size;
}ds3_bulk_object;

typedef struct {
    ds3_bulk_object*  list;
    uint64_t          size;
    uint64_t          chunk_number;
    char*             server_id;
    size_t            server_id_size;
}ds3_bulk_object_list;

typedef struct {
    char*                   job_id;
    size_t                  job_id_size;
    ds3_bulk_object_list**  list;
    size_t                  list_size;
}ds3_bulk_response;

typedef enum {
  DS3_ERROR_INVALID_XML, DS3_ERROR_CURL_HANDLE, DS3_ERROR_REQUEST_FAILED, DS3_ERROR_MISSING_ARGS, DS3_ERROR_BAD_STATUS_CODE
}ds3_error_code;

typedef struct {
    uint64_t  status_code;
    char*     status_message;
    size_t    status_message_size;
    char*     error_body;
    size_t    error_body_size;
}ds3_error_response;

typedef struct {
    ds3_error_code      code;
    char*               message;
    size_t              message_size;
    ds3_error_response* error;
}ds3_error;

LIBRARY_API ds3_creds* ds3_create_creds(const char* access_id, const char* secret_key);
LIBRARY_API ds3_client* ds3_create_client(const char* endpoint, ds3_creds* creds);

LIBRARY_API ds3_request* ds3_init_get_service(void);
LIBRARY_API ds3_request* ds3_init_get_bucket(const char* bucket_name);
LIBRARY_API ds3_request* ds3_init_get_object(const char* bucket_name, const char* object_name);
LIBRARY_API ds3_request* ds3_init_put_bucket(const char* bucket_name);
LIBRARY_API ds3_request* ds3_init_put_object(const char* bucket_name, const char* object_name, uint64_t size);
LIBRARY_API ds3_request* ds3_init_delete_bucket(const char* bucket_name);
LIBRARY_API ds3_request* ds3_init_delete_object(const char* bucket_name, const char* object_name);

LIBRARY_API ds3_request* ds3_init_put_bulk(const char* bucket_name, ds3_bulk_object_list* object_list);
LIBRARY_API ds3_request* ds3_init_get_bulk(const char* bucket_name, ds3_bulk_object_list* object_list);

LIBRARY_API void ds3_client_proxy(ds3_client* client, const char* proxy);

LIBRARY_API ds3_error* ds3_get_service(const ds3_client* client, const ds3_request* request, ds3_get_service_response** response);
LIBRARY_API ds3_error* ds3_get_bucket(const ds3_client* client, const ds3_request* request, ds3_get_bucket_response** response);
LIBRARY_API ds3_error* ds3_bulk(const ds3_client* client, const ds3_request* request, ds3_bulk_response** response);

LIBRARY_API ds3_error* ds3_put_bucket(const ds3_client* client, const ds3_request* request);
LIBRARY_API ds3_error* ds3_delete_bucket(const ds3_client* client, const ds3_request* request);
LIBRARY_API ds3_error* ds3_get_object(const ds3_client* client, const ds3_request* request, void* user_data, size_t (* callback)(void*, size_t, size_t, void*));
LIBRARY_API ds3_error* ds3_put_object(const ds3_client* client, const ds3_request* request, void* user_data, size_t (* callback)(void*, size_t, size_t, void*));
LIBRARY_API ds3_error* ds3_delete_object(const ds3_client* client, const ds3_request* request);

LIBRARY_API void ds3_free_service_response(ds3_get_service_response* response);
LIBRARY_API void ds3_free_bucket_response(ds3_get_bucket_response* response);
LIBRARY_API void ds3_free_bulk_response(ds3_bulk_response* response);
LIBRARY_API void ds3_free_error(ds3_error* error);

LIBRARY_API void ds3_free_bucket(ds3_bucket* bucket);
LIBRARY_API void ds3_free_owner(ds3_owner* owner);
LIBRARY_API void ds3_free_creds(ds3_creds* client);
LIBRARY_API void ds3_free_client(ds3_client* client);
LIBRARY_API void ds3_free_request(ds3_request* request);
LIBRARY_API void ds3_cleanup(void);

LIBRARY_API void ds3_print_request(const ds3_request* request);
// provided helpers
LIBRARY_API size_t ds3_write_to_file(void* buffer, size_t size, size_t nmemb, void* user_data);
LIBRARY_API size_t ds3_read_from_file(void* buffer, size_t size, size_t nmemb, void* user_data);

LIBRARY_API ds3_bulk_object_list * ds3_convert_file_list(const char** file_list, uint64_t num_files);
LIBRARY_API void ds3_free_bulk_object_list(ds3_bulk_object_list* object_list);

#ifdef __cplusplus
}
#endif
#endif
