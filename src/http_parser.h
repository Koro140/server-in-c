
typedef enum Http_Method {
    HTTP_METHOD_POST,
    HTTP_METHOD_GET,
    HTTP_METHOD_DELETE,
    // Add more later
}Http_Method;

typedef struct Http_Response {

}Http_Response;

Http_Response* http_parse_request(const char* req_str);
void http_response_destroy(Http_Response* parser);