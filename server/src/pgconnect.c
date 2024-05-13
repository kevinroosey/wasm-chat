#include <libpq-fe.h>

PGconn *connect_db()
{
  const char *conninfo = "dbname=myapp user=myuser password=mypassword";
  PGconn *conn = PQconnectdb(conninfo);
  if (PQstatus(conn) != CONNECTION_OK)
  {
    fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
    PQfinish(conn);
    exit(1);
  }
  return conn;
}
