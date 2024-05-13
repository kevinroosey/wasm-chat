
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <jwt.h>

#define PORT 8888
#define SECRET_KEY "your-secret-key"

static enum MHD_Result request_handler(void *cls, struct MHD_Connection *connection, const char *url,
                                       const char *method, const char *version, const char *upload_data,
                                       size_t *upload_data_size, void **con_cls)
{
    if (strcmp(url, "/signup") == 0 && strcmp(method, "POST") == 0)
    {
        return signup_handler(connection, upload_data, upload_data_size, con_cls);
    }
    else if (strcmp(url, "/signin") == 0 && strcmp(method, "POST") == 0)
    {
        return signin_handler(connection, upload_data, upload_data_size, con_cls);
    }
    else if (strcmp(url, "/signout") == 0 && strcmp(method, "POST") == 0)
    {
        return signout_handler(connection);
    }

    const char *response_str = "Not Found";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void *)response_str, MHD_RESPMEM_PERSISTENT);
    return MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
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
