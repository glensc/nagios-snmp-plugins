/*************************************************************************
 *
 * plugins for Nagios
 *
 * (C) 2002-2008
 *   Henning P. Schmiedehausen
 *   INTERMETA - Gesellschaft fuer Mehrwertdienste mbH
 *   Hutweide 15
 *   D-91054 Buckenhof
 *
 *************************************************************************
 *
 * Common code
 *
 *************************************************************************
 *
 * Distributed under GPL.
 *
 * $Id: snmp_common.c,v 1.3 2002/01/27 15:31:50 henning Exp $
 *
 */

#include <limits.h>
#include "snmp_common.h"

char *snmp_version ="$Revision: 1.3 $ ($Date: 2002/01/27 15:31:50 $)";

char *version;         // Version of the program (set in main)
char *bn;              // basename of the program (set in main)

int timeout = 0;       // Timeout for Queries (0 == default)
int verbose = 0;       // Verbosity (0 == not verbose, for production)
int listing = 0;       // Listing mode (0 == default)

char *community = NULL;  // Community, must be set
char *hostname  = NULL;  // Hostname, must be set

/**
 * Print out the command line options
 */

void print_help()
{
  printf("Usage for %s\n", bn);
  printf("-h, --help:                  Display this text\n");
  printf("-V, --version:               Display program version\n");
  printf("-t <secs>, --timeout <secs>: Set program timeout\n");
  printf("-C, --community <name>:      Set SNMP community (required)\n");
  printf("-H, --hostname <name>:       Set Hostname to scan (required)\n");
  printf("-v, --verbose:               Increase verbosity (for testing)\n");
}

/**
 * Print out the program version.
 */

void print_version()
{
  printf("%s: %s\nsnmp_common: %s\n", bn, version, snmp_version);
}

/**
 * Returns 1 if the supplied number is an integer, 0 if not
 */

int is_integer(char *number)
{
  long int n;

  if(strspn(number,"-0123456789 ") != strlen(number))
    return(0);

  n=strtol(number,NULL,10);
  if((errno != ERANGE) && (n >= INT_MIN) && (n <= INT_MAX))
    return(1);

  return(0);
}

/**
 * Fetches an SNMP Table from the supplied OID base.
 *
 * callback gets called for each found element.
 * pnt is a generic pointer argument passed to callback
 * nof is the maximum number of elements to look for. If zero,
 * no limit.
 */

int fetch_table(char *base, callback_t callback, void *pnt, int nof)
{
  int res = 0;

  struct snmp_session  session, *ss;
  struct snmp_pdu *pdu, *response;
  struct variable_list *vars;

  oid root[MAX_OID_LEN];
  oid    name[MAX_OID_LEN];
  size_t rootlen = MAX_OID_LEN;
  size_t namelen;

  int running;
  int status;

  snmp_sess_init( &session );

  init_snmp("disk_plugin");

  if(session.version == SNMP_DEFAULT_VERSION)
    session.version = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_SNMPVERSION);
    
  session.peername = hostname;
  session.community = (unsigned char *)community;
  session.community_len = strlen(community);

  if(timeout)
    session.timeout = timeout  * 1000000L;

  if(!snmp_parse_oid(base, root, &rootlen))
  {
    return -1;
  }
  
  SOCK_STARTUP;
  
  ss = snmp_open(&session);

  if(!ss)
  {
    SOCK_CLEANUP;
    return -1;
  }
  
  memmove(name, root, rootlen * sizeof(oid));
  namelen = rootlen;

  running = -1;

  while(running)
  {
    pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
    snmp_add_null_var(pdu, name, namelen);

    status = snmp_synch_response(ss, pdu, &response);
    if(status != STAT_SUCCESS)
    {
      if(verbose)
        printf("%s: Error #1\n", bn);
      running = 0;
      res = -1;
      continue;
    }
      
    if(response->errstat != SNMP_ERR_NOERROR)
    {
      running = 0;
      if (response->errstat != SNMP_ERR_NOSUCHNAME)
      {
        if(verbose)
          printf("%s: Error #2\n", bn);
        res = -1;
      }
      continue;
    }
      
    for(vars = response->variables; vars; vars = vars->next_variable)
    {
      if((vars->name_length < rootlen) ||
         (memcmp(root, vars->name, rootlen * sizeof(oid)) != 0))
      {
        running = 0;
        continue;
      }

      if(verbose)
        printf("%s: Found another element\n", bn);


      callback(vars, &pnt);

      res++;
      //
      // We have a maximum count. check it
      //
      if(nof && !(--nof))
      {
        running = 0;
        continue;
      }
      
      if ((vars->type != SNMP_ENDOFMIBVIEW) &&
          (vars->type != SNMP_NOSUCHOBJECT) &&
          (vars->type != SNMP_NOSUCHINSTANCE))
      {
        memmove((char *)name, (char *)vars->name,
                vars->name_length * sizeof(oid));
        
        namelen = vars->name_length;
      }
      else
      {
        running = 0;
      }
    } /* for */

    if(response)
      snmp_free_pdu(response);
  }

  snmp_close(ss);
  SOCK_CLEANUP;
  return res;
}

/**
 * Callback that does nothing. Used for counting the number
 * of elements in a table.
 */

void null_callback(struct variable_list *vars, void **pnt)
{
  if(verbose)
    printf("%s: Null Callback called with %lx\n", bn, vars);
}

/**
 * Stores the integer value of the supplied variable element
 * into pnt. Advances pnt
 */

void integer_callback(struct variable_list *vars, void **pnt)
{
  long *data = (long *)(*pnt);

  if(verbose)
    printf("%s: Found Integer Value %ld into %lx\n", bn, *vars->val.integer, data);

  *data = *vars->val.integer;

  *pnt = ++data;
}

/**
 * Stores the string value of the supplied variable element
 * into pnt. Advances pnt
 */


void string_callback(struct variable_list *vars, void **pnt)
{
  char **data = (char **)(*pnt);
  
  if(verbose)
    printf("%s: Putting String into %lx\n", bn, data);

  if((vars->val.string) && (*data = calloc(sizeof(char), vars->val_len+1)))
  {
    memcpy(*data, vars->val.string, vars->val_len);
  }
  else
  {
    *data = "";
  }

  *pnt = ++data;
}

