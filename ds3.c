#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "ds3.h"
#include "net.h"

static GHashTable * _create_hash_table(void) {
    GHashTable * hash =  g_hash_table_new(g_str_hash, g_str_equal);
    return hash;
}

ds3_creds * ds3_create_creds(const char * access_id, const char * secret_key) {
    ds3_creds * creds;
    if(access_id == NULL || secret_key == NULL) {
        fprintf(stderr, "Arguments cannot be NULL\n");
        return NULL;
    }
    
    creds = g_new0(ds3_creds, 1);

    creds->access_id = g_strdup(access_id);
    creds->access_id_len = strlen(creds->access_id);

    creds->secret_key = g_strdup(secret_key); 
    creds->secret_key_len = strlen(creds->secret_key);

    return creds;
}

ds3_client * ds3_create_client(const char * endpoint, ds3_creds * creds) {
    ds3_client * client;
    if(endpoint == NULL) {
        fprintf(stderr, "Null endpoint\n");
        return NULL;
    }

    client = g_new0(ds3_client, 1);
    
    client->endpoint = g_strdup(endpoint);
    client->endpoint_len = strlen(endpoint);
    
    client->creds = creds;
    return client;
}

void ds3_client_proxy(ds3_client * client, const char * proxy) {
    client->proxy = g_strdup(proxy);
    client->proxy_len = strlen(proxy);
}

ds3_request * _common_request_init(void){
    ds3_request * request = g_new0(ds3_request, 1);
    request->headers = _create_hash_table();
    request->query_params = _create_hash_table();
    return request;
}

ds3_request * ds3_init_get_service(void) {
    ds3_request * request = _common_request_init(); 
    request->verb = GET;
    request->path =  g_new0(char, 2);
    request->path [0] = '/';
    return request;
}

ds3_request * ds3_init_get_bucket(const char * bucket_name) {
    ds3_request * request = _common_request_init(); 
    request->verb = GET;
    request->path = g_strconcat("/", bucket_name, NULL);
    return request;
}

static void _internal_request_dispatcher(const ds3_client * client, const ds3_request * request,void * user_struct, size_t (*write_data)(void*, size_t, size_t, void*)) {
    if(client == NULL || request == NULL) {
        fprintf(stderr, "All arguments must be filled in\n");
        return;
    }
    net_process_request(client, request, user_struct, write_data);
}

static size_t load_xml_buff(void* contents, size_t size, size_t nmemb, void *user_data) {
    size_t realsize = size * nmemb;
    GByteArray* blob = (GByteArray*) user_data;
    
    g_byte_array_append(blob, contents, realsize);
    return realsize;
}

static void _parse_buckets(xmlDocPtr doc, xmlNodePtr buckets_node, ds3_get_service_response * response) {
}

static void _parse_owner(xmlDocPtr doc, xmlNodePtr owner_node, ds3_get_service_response * response) {
    xmlNodePtr child_node;
    xmlChar * text;
    ds3_owner * owner = g_new0(ds3_owner, 1);
    
    child_node = owner_node->xmlChildrenNode;

    while(child_node != NULL) {
        if(xmlStrcmp(child_node->name, (const xmlChar *) "DisplayName") == 0) {
            text = xmlNodeListGetString(doc, child_node->xmlChildrenNode, 1);
            owner->name = g_strdup(text);
            owner->name_size = strlen(text);
            xmlFree(text);
        }
        else if(xmlStrcmp(child_node->name, (const xmlChar *) "ID") == 0) {
            text = xmlNodeListGetString(doc, child_node->xmlChildrenNode, 1);
            owner->id = g_strdup(text);
            owner->id_size = strlen(text);
            xmlFree(text);
        }
        else {
            fprintf(stderr, "Unknown xml element: (%s)\n", child_node->name);
            return;
        }
        child_node = child_node->next;
    }

    response->owner = owner;
}

ds3_get_service_response * ds3_get_service(const ds3_client * client, const ds3_request * request) {
    xmlDocPtr doc;
    xmlNodePtr root;
    xmlNodePtr child_node;
    GByteArray* xml_blob = g_byte_array_new();
    ds3_get_service_response * response;
    
    _internal_request_dispatcher(client, request, xml_blob, load_xml_buff);
   
    doc = xmlParseMemory(xml_blob->data, xml_blob->len);

    if(doc == NULL) {
        fprintf(stderr, "Failed to parse document.");
        fprintf(stdout, "Result: %s\n", xml_blob->data);
        xmlFreeDoc(doc);
        g_byte_array_free(xml_blob, TRUE);
        return NULL;
    }

    root = xmlDocGetRootElement(doc);
    
    if(xmlStrcmp(root->name, (const xmlChar*) "ListAllMyBucketsResult") != 0) {
        fprintf(stderr, "wrong document, expected root node to be ListAllMyBucketsResult");
        fprintf(stdout, "Result: %s\n", xml_blob->data);
        xmlFreeDoc(doc);
        g_byte_array_free(xml_blob, TRUE);
        return NULL;
    }

    response = g_new0(ds3_get_service_response, 1);
    child_node = root->xmlChildrenNode;
    while(child_node != NULL) {
        if(xmlStrcmp(child_node->name, (const xmlChar*) "Buckets") == 0) {
            //process buckets here
            _parse_buckets(doc, child_node, response);

        }
        else if(xmlStrcmp(child_node->name, (const xmlChar*) "Owner") == 0) {
            //process owner here
            _parse_owner(doc, child_node, response);
        }
        else {
            fprintf(stderr, "Unknown xml element: (%s)\b", child_node->name);
        }
        child_node = child_node->next;
    }

    xmlFreeDoc(doc);
    g_byte_array_free(xml_blob, TRUE);
    return response;    
}

void ds3_get_bucket(const ds3_client * client, const ds3_request * request) {
    GByteArray* xml_blob = g_byte_array_new();
    _internal_request_dispatcher(client, request, xml_blob, load_xml_buff);
    fprintf(stdout, "Result: %s\n", xml_blob->data);

    g_byte_array_free(xml_blob, TRUE);
}

void ds3_print_request(const ds3_request * request) {
    if(request == NULL) {
      fprintf(stderr, "Request object was null\n");
      return;
    }
    printf("Verb: %s\n", net_get_verb(request->verb));
    printf("Path: %s\n", request->path);
}

void ds3_free_service_response(ds3_get_service_response * response){
    size_t num_buckets;
    int i;

    if(response == NULL) {
        return;
    }

    num_buckets = response->num_buckets;

    for(i = 0; i<num_buckets; i++) {
        ds3_free_bucket(response->buckets[i]);
    }
    ds3_free_owner(response->owner);
    g_free(response->buckets);
    g_free(response);
}

void ds3_free_bucket(ds3_bucket * bucket) {
    if(bucket == NULL) {
        fprintf(stderr, "Bucket was NULL\n");
        return;
    }
    if(bucket->name != NULL) {
        g_free(bucket->name);
    }
    if(bucket->creation_date != NULL) {
        g_free(bucket->creation_date);
    }
    g_free(bucket);
}

void ds3_free_owner(ds3_owner * owner) {
    if(owner == NULL) {
        fprintf(stderr, "Owner was NULL\n");
        return;
    }
    if(owner->name != NULL) {
        g_free(owner->name);
    }
    if(owner->id != NULL) {
        g_free(owner->id);
    }
    g_free(owner);
}

void ds3_free_creds(ds3_creds * creds) {
    if(creds == NULL) {
        return;
    }

    if(creds->access_id != NULL) {
        g_free(creds->access_id);
    }

    if(creds->secret_key != NULL) {
        g_free(creds->secret_key);
    }
    g_free(creds);
}

void ds3_free_client(ds3_client * client) {
    if(client == NULL) {
      return;
    }
    if(client->endpoint != NULL) {
        g_free(client->endpoint);
    }
    if(client->proxy != NULL) {
        g_free(client->proxy);
    }
    g_free(client);
}

void ds3_free_request(ds3_request * request) {
    if(request == NULL) {
        return;
    }
    if(request->path != NULL) {
        g_free(request->path);
    }
    if(request->headers != NULL) {
        g_hash_table_destroy(request->headers);
    }
    if(request->query_params != NULL) {
        g_hash_table_destroy(request->query_params);
    }
    g_free(request);
}

void ds3_cleanup(void) {
    net_cleanup();
}
