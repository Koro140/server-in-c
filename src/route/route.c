#include "../route.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROUTER_INITIAL_CAP 60

Router* router_create()
{
    Router* router = malloc(sizeof(Router));
    if (router == NULL)
    {
        fprintf(stderr, "ERROR::Couldn't allocate memory for router\n");
        return NULL;
    }
    
    router->routes = malloc(sizeof(Route) * ROUTER_INITIAL_CAP);
    if (router->routes == NULL)
    {
        fprintf(stderr, "ERROR::Couldn't allocate memory for routes\n");
        return NULL;
    }
    
    router->capacity = ROUTER_INITIAL_CAP;
    router->count = 0;

    return router;
}

void router_destroy(Router* router) 
{
    free(router->routes);
    free(router);
}

static void router_append(Router* router,const char* route, Http_Method method, void(*handler)(int, const char*)) {
    if (router->count == router->capacity)
    {
        Route* new_routes_ptr = realloc(router->routes, sizeof(Route) * router->capacity * 2);
        if (new_routes_ptr == NULL)
        {
            fprintf(stderr, "ERROR::Couldn't allocate more memory for routes\n");
            return;
        }
        router->routes = new_routes_ptr;
        router->capacity *= 2;
    }
    
    Route* current_route = &router->routes[router->count];

    current_route->func = handler;
    current_route->method = method;
    strncpy(current_route->name, route, 512);

    router->count++;
}

void router_dispatch(Router* router,Route* route, int connection, const char* param) {
    int len = strlen(route->name);

    for (int i = 0; i < router->count; i++)
    {
        switch (router->routes[i].method)
        {
        case HTTP_METHOD_GET:
            if (strncmp(route->name, router->routes[i].name, len) == 0) {
                router->routes[i].func(connection, param);
            }
            break;
        case HTTP_METHOD_POST:
            if (strncmp(route->name, router->routes[i].name, len) == 0) {
                router->routes[i].func(connection, param);
            }
            break;
        case HTTP_METHOD_DELETE:
            if (strncmp(route->name, router->routes[i].name, len) == 0) {
                router->routes[i].func(connection, param);
            }
            break;
        default:
            break;
        }
    }
    
}


void route_method_post(Router* router,const char* route, void(*handler)(int, const char*)) {
    router_append(router, route, HTTP_METHOD_POST, handler);
}
void route_method_get(Router* router,const char* route, void(*handler)(int, const char*)) {
    router_append(router, route, HTTP_METHOD_GET, handler);
}

void route_method_delete(Router* router,const char* route, void(*handler)(int, const char*)) {
    router_append(router, route, HTTP_METHOD_DELETE, handler);
}