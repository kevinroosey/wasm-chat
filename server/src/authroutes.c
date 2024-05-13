#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <jwt.h>

#define SECRET_KEY "your-secret-key"
static enum MHD_Result signup_handler(struct MHD_Connection *connection, const char *upload_data,
                                      size_t *upload_data_size, void **con_cls)
{
  static int dummy;
  if (&dummy != *con_cls)
  {
    *con_cls = &dummy;
    return MHD_YES;
  }

  if (*upload_data_size)
  {
    char *data = strdup(upload_data);
    char *username = strtok(data, "&");
    char *password = strtok(NULL, "&");
    if (username && password)
    {
      username = strchr(username, '=') + 1;
      password = strchr(password, '=') + 1;

      PGconn *conn = connect_db();
      const char *paramValues[2] = {username, password};
      PGresult *res = PQexecParams(conn,
                                   "INSERT INTO users (username, password) VALUES ($1, crypt($2, gen_salt('bf')))",
                                   2,    /* number of parameters */
                                   NULL, /* parameter types (can be NULL) */
                                   paramValues,
                                   NULL, /* parameter lengths (can be NULL) */
                                   NULL, /* parameter formats (can be NULL) */
                                   0);   /* result format: 0 = text, 1 = binary */

      if (PQresultStatus(res) != PGRES_COMMAND_OK)
      {
        fprintf(stderr, "Insert failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);

        const char *response_str = "Signup failed";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str),
                                                                        (void *)response_str, MHD_RESPMEM_PERSISTENT);
        return MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
      }

      PQclear(res);
      PQfinish(conn);

      // Generate a JWT token
      jwt_t *jwt;
      char *token;

      jwt_new(&jwt);
      jwt_add_grant(jwt, "sub", username);
      jwt_set_alg(jwt, JWT_ALG_HS256, (unsigned char *)SECRET_KEY, strlen(SECRET_KEY));
      token = jwt_encode_str(jwt);
      jwt_free(jwt);

      struct MHD_Response *response = MHD_create_response_from_buffer(strlen(token),
                                                                      (void *)token, MHD_RESPMEM_MUST_FREE);
      return MHD_queue_response(connection, MHD_HTTP_OK, response);
    }

    free(data);
  }

  const char *response_str = "Invalid input";
  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str),
                                                                  (void *)response_str, MHD_RESPMEM_PERSISTENT);
  return MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
}

static enum MHD_Result signin_handler(struct MHD_Connection *connection, const char *upload_data,
                                      size_t *upload_data_size, void **con_cls)
{
  static int dummy;
  if (&dummy != *con_cls)
  {
    *con_cls = &dummy;
    return MHD_YES;
  }

  if (*upload_data_size)
  {
    char *data = strdup(upload_data);
    char *username = strtok(data, "&");
    char *password = strtok(NULL, "&");
    if (username && password)
    {
      username = strchr(username, '=') + 1;
      password = strchr(password, '=') + 1;

      PGconn *conn = connect_db();
      const char *paramValues[2] = {username, password};
      PGresult *res = PQexecParams(conn,
                                   "SELECT id FROM users WHERE username = $1 AND password = crypt($2, password)",
                                   2,    /* number of parameters */
                                   NULL, /* parameter types (can be NULL) */
                                   paramValues,
                                   NULL, /* parameter lengths (can be NULL) */
                                   NULL, /* parameter formats (can be NULL) */
                                   0);   /* result format: 0 = text, 1 = binary */

      if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
        fprintf(stderr, "Select failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);

        const char *response_str = "Login failed";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str),
                                                                        (void *)response_str, MHD_RESPMEM_PERSISTENT);
        return MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
      }

      if (PQntuples(res) == 1)
      {
        // Generate a JWT token
        jwt_t *jwt;
        char *token;

        jwt_new(&jwt);
        jwt_add_grant(jwt, "sub", username);
        jwt_set_alg(jwt, JWT_ALG_HS256, (unsigned char *)SECRET_KEY, strlen(SECRET_KEY));
        token = jwt_encode_str(jwt);
        jwt_free(jwt);

        PQclear(res);
        PQfinish(conn);

        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(token),
                                                                        (void *)token, MHD_RESPMEM_MUST_FREE);
        return MHD_queue_response(connection, MHD_HTTP_OK, response);
      }
      else
      {
        PQclear(res);
        PQfinish(conn);

        const char *response_str = "Invalid username or password";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str),
                                                                        (void *)response_str, MHD_RESPMEM_PERSISTENT);
        return MHD_queue_response(connection, MHD_HTTP_UNAUTHORIZED, response);
      }
    }

    free(data);
  }

  const char *response_str = "Invalid input";
  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str),
                                                                  (void *)response_str, MHD_RESPMEM_PERSISTENT);
  return MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
}

static enum MHD_Result signout_handler(struct MHD_Connection *connection)
{
  const char *response_str = "Signed out";
  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str),
                                                                  (void *)response_str, MHD_RESPMEM_PERSISTENT);
  return MHD_queue_response(connection, MHD_HTTP_OK, response);
}
