#include "http_parser.h"

typedef struct Route{
    Http_Method method;
    char name[512];
    
    void(*func)(int socket, const char* param);
}Route;

// it's just a container for all routes ... now a simple dynamic array
typedef struct Router {
    Route* routes;
    int count;
    int capacity;
}Router;

Router* router_create(void);
void router_destroy(Router* router);
void router_dispatch(Router* router,Route* route, int connection, const char* param);

// To add different routes to a router
void route_method_post(Router* router,const char* route, void(*handler)(int, const char*));
void route_method_get(Router* router,const char* route, void(*handler)(int, const char*));
void route_method_delete(Router* router,const char* route, void(*handler)(int, const char*));