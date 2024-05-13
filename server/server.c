
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <jwt.h>

#define PORT 8888
#define SECRET_KEY "your-secret-key"

static enum MHD_Result request_handler(void *cls, struct MHD_Connection *connection, const char *url,
                                       const char *method, const char *version, const char *upload_data,
                                       size_t *upload_data_size, void **con_cls)
{
    if (strcmp(url, "/login") == 0 && strcmp(method, "POST") == 0)
    {
        // Normally, you'd validate the user credentials here
        jwt_t *jwt;
        char *token;

        jwt_new(&jwt);
        jwt_add_grant(jwt, "sub", "user_id");
        jwt_set_alg(jwt, JWT_ALG_HS256, (unsigned char *)SECRET_KEY, strlen(SECRET_KEY));
        token = jwt_encode_str(jwt);
        jwt_free(jwt);

        struct MHD_Response *response;
        response = MHD_create_response_from_buffer(strlen(token), (void *)token, MHD_RESPMEM_MUST_FREE);
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);

        return ret;
    }
    else if (strcmp(url, "/protected") == 0 && strcmp(method, "GET") == 0)
    {
        const char *auth_header = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Authorization");
        if (auth_header && strncmp(auth_header, "Bearer ", 7) == 0)
        {
            jwt_t *jwt;
            const char *token = auth_header + 7;
            if (jwt_decode(&jwt, token, (unsigned char *)SECRET_KEY, strlen(SECRET_KEY)))
            {
                struct MHD_Response *response;
                const char *response_str = "Access Denied";
                response = MHD_create_response_from_buffer(strlen(response_str), (void *)response_str, MHD_RESPMEM_PERSISTENT);
                int ret = MHD_queue_response(connection, MHD_HTTP_UNAUTHORIZED, response);
                MHD_destroy_response(response);
                return ret;
            }

            jwt_free(jwt);
            const char *response_str = "Protected Resource";
            struct MHD_Response *response;
            response = MHD_create_response_from_buffer(strlen(response_str), (void *)response_str, MHD_RESPMEM_PERSISTENT);
            int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
            return ret;
        }
        else
        {
            struct MHD_Response *response;
            const char *response_str = "Authorization Header Missing or Invalid";
            response = MHD_create_response_from_buffer(strlen(response_str), (void *)response_str, MHD_RESPMEM_PERSISTENT);
            int ret = MHD_queue_response(connection, MHD_HTTP_UNAUTHORIZED, response);
            MHD_destroy_response(response);
            return ret;
        }
    }

    struct MHD_Response *response;
    const char *response_str = "Not Found";
    response = MHD_create_response_from_buffer(strlen(response_str), (void *)response_str, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);

    return ret;
}

int main()
{
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                              &request_handler, NULL, MHD_OPTION_END);
    if (NULL == daemon)
        return 1;

    printf("Server is running on port %d\n", PORT);
    getchar();

    MHD_stop_daemon(daemon);
    return 0;
}
