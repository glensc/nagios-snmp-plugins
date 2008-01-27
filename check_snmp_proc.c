/*************************************************************************
 *
 * plugins for Netsaint
 *
 * (C) 2002-2007
 *   Henning P. Schmiedehausen
 *   INTERMETA - Gesellschaft fuer Mehrwertdienste mbH
 *   Hutweide 15
 *   D-91054 Buckenhof
 *
 *************************************************************************
 *
 * Checks the running processes for a given host via snmp and the
 * ucd snmp interface.
 *
 *************************************************************************
 *
 * Distributed under GPL.
 *
 * $Id: proc_plugin.c,v 1.6 2002/01/27 22:10:24 henning Exp $
 *
 */

#include "snmp_common.h"

#define RCS_VERSION "$Revision: 1.6 $ ($Date: 2002/01/27 22:10:24 $)"

#define PROC_INDEX_MIB       ".1.3.6.1.4.1.2021.2.1.1"
#define PROC_NAME_MIB        ".1.3.6.1.4.1.2021.2.1.2"
#define PROC_MIN_MIB         ".1.3.6.1.4.1.2021.2.1.3"
#define PROC_MAX_MIB         ".1.3.6.1.4.1.2021.2.1.4"
#define PROC_RUNNING_MIB     ".1.3.6.1.4.1.2021.2.1.5"
#define PROC_ERRORFLAG_MIB   ".1.3.6.1.4.1.2021.2.1.100"
#define PROC_ERRORMSG_MIB    ".1.3.6.1.4.1.2021.2.1.101"

int report_proc(void);

int main (int argc, char *argv[])
{
  static struct option long_options[] = {
    { "help",      no_argument,       0, 'h' },
    { "version",   no_argument,       0, 'V' },
    { "timeout",   required_argument, 0, 't' },
    { "community", required_argument, 0, 'C' },
    { "hostname",  required_argument, 0, 'H' },
    { "verbose",   no_argument,       0, 'v' },
    { "list",      no_argument,       0, 'l' },
    { 0, 0, 0, 0 },
  };
  int option_index = 0;
  int c;

  int ret = STATE_UNKNOWN;

  bn = strdup(basename(argv[0]));
  version = VERSION;

#define OPTS "?hVvlt:c:w:C:H:"
  
  while(1)
  {
    c = getopt_long(argc, argv, OPTS, long_options, &option_index);

    if(c == -1 || c == EOF)
      break;

    switch(c)
    {
      case '?':
      case 'h':
        print_help();
        exit(STATE_UNKNOWN);

      case 'V':
        print_version();
        exit(STATE_UNKNOWN);


      case 't':
        if(!is_integer(optarg))
        {
          printf("%s: Timeout interval (%s)must be integer!\n",
                 bn,
                 optarg);
          exit(STATE_UNKNOWN);
        }
        
        timeout = atoi(optarg);
        if(verbose)
          printf("%s: Timeout set to %d\n", bn, timeout);
        break;

      case 'C':
        community = strdup(optarg);

        if(verbose)
          printf("%s: Community set to %s\n", bn, community);
        
        break;

      case 'H':
        hostname = strdup(optarg);

        if(verbose)
          printf("%s: Hostname set to %s\n", bn, hostname);

        break;

      case 'v':
        verbose = 1;
        printf("%s: Verbose mode activated\n", bn);
        break;

      case 'l':
        listing = 1;
		if(verbose)
			printf("%s: List mode activated\n", bn);
        break;
    }
  }

  if(!hostname || !community)
  {
    printf("%s: Hostname or Community missing!\n", bn);
    print_help();
    exit(STATE_UNKNOWN);
  }

  ret = report_proc();

  if(verbose)
    printf("%s: Returning %d\n", bn, ret);

  exit(ret);
}

void format_report(int cnt, long *errors, char **errormsg)
{
  int i;
  size_t nmany = 0, nfew = 0, nunkn = 0;
  char **errmany = NULL, **errfew = NULL, **errunkn = NULL;
  char **pmany = NULL, **pfew = NULL, **punkn = NULL;

  errmany = calloc(sizeof(char **), cnt);
  if (!errmany) {
    printf("%s: Could not allocate memory for error information\n", bn);
    return;
  }
  errfew = calloc(sizeof(char **), cnt);
  if (!errfew) {
    printf("%s: Could not allocate memory for error information\n", bn);
    return;
  }
  errunkn = calloc(sizeof(char **), cnt);
  if (!errunkn) {
    printf("%s: Could not allocate memory for error information\n", bn);
    return;
  }

#define _msg_few "Too few "
#define _msg_many "Too many "
  pmany = errmany; pfew = errfew; punkn = errunkn;
  for (i = 0; i < cnt; i++) {
    if (errors[i]) {
      size_t len = strlen(errormsg[i]);
      if (strncmp(errormsg[i], _msg_many, sizeof(_msg_many) - 1) == 0) {
        *(pmany++) = errormsg[i] + sizeof(_msg_many) - 1;
        nmany++;

      } else if (strncmp(errormsg[i], _msg_few, sizeof(_msg_few) - 1) == 0) {
        *(pfew++) = errormsg[i] + sizeof(_msg_few) - 1;
        nfew++;

      } else {
        *(punkn++) = errormsg[i];
        nunkn++;
      }
    }
  }

  if (verbose) {
    printf("%s: Got %ld few, %ld many, %ld unknown messages\n", bn, nfew, nmany, nunkn);
  }

#define _msg_running " running"
  if (nmany) {
    char *p;
    printf("%s", _msg_many);
    for (i = 0; i < nmany- 1; i++) {
      p = strstr(errmany[i], _msg_running);
      printf("%.*s%s, ", p - errmany[i], errmany[i], p + sizeof(_msg_running) - 1);
    }
    p = strstr(errmany[i], _msg_running);
    printf("%.*s%s", p - errmany[i], errmany[i], p + sizeof(_msg_running) - 1);
    printf("%s", _msg_running);
  }
  if (nfew) {
    char *p;
    if (nmany) {
      printf(". ");
    }
    printf("%s", _msg_few);
    for (i = 0; i < nfew - 1; i++) {
      p = strstr(errfew[i], _msg_running);
      printf("%.*s%s, ", p - errfew[i], errfew[i], p + sizeof(_msg_running) - 1);
    }
    p = strstr(errfew[i], _msg_running);
    printf("%.*s%s", p - errfew[i], errfew[i], p + sizeof(_msg_running) - 1);
    printf("%s", _msg_running);
  }
  if (nunkn) {
    if (nfew) {
      printf(". ");
    }
    for (i = 0; i < nunkn - 1; i++) {
      printf("%s", errunkn[i]);
    }
    printf("%s", errunkn[i]);
  }
  printf("\n");
}

int report_proc()
{
  int cnt;

  long *errors;
  void *pnt;
  int i;
  int gotErrors = 0;
  char **errormsg = NULL;
  char **procname = NULL;
  
  if((cnt = fetch_table(PROC_INDEX_MIB, null_callback, NULL, 0)) < 0)
  {
    printf("%s: Could not fetch list of processes\n", bn);
    return STATE_UNKNOWN;
  }

  if(!cnt)  // No procs configured
  {
    printf("%s: No processes found.\n", bn);
    return STATE_OK;
  }

  if(!(errors  = calloc(sizeof(long), cnt)))
  {
    printf("%s: Could not allocate memory for information\n", bn);
    return STATE_CRITICAL;
  }

  procname = calloc(sizeof(char **), cnt);

  pnt = errors;
  if(fetch_table(PROC_ERRORFLAG_MIB, integer_callback, pnt, cnt) < 0)
  {
    printf("%s: Could not fetch error list!\n", bn);
    return STATE_UNKNOWN;
  }

  for(i=0; i<cnt; i++)
  {
    if(verbose)
      printf("%s: Got Flag %ld for %d\n", bn, errors[i], i);

    if(errors[i])
    {
      gotErrors = 1;
      break;
    }
  }

	pnt = procname;
	if(fetch_table(PROC_NAME_MIB, string_callback, pnt, cnt) < 0)
	{
		printf("%s: Could not fetch process list\n", bn);
		return STATE_CRITICAL;
	}

  if(gotErrors == 0)
  {
	if(listing)
	{
		printf("Checked %d process groups. ( ", cnt);
		for(i=0; i < cnt; i++)
		{
			printf( "%s ", procname[i] );
		}
		printf( ")\n");
	}
	else
    	printf("Checked %d process groups.\n", cnt);
    return STATE_OK;
  }

  errormsg = calloc(sizeof(char **), cnt);
  if(!errormsg)
  {
    printf("%s: Could not allocate memory for error information\n", bn);
    return STATE_CRITICAL;
  }
  
  pnt = errormsg;
  
  if(fetch_table(PROC_ERRORMSG_MIB, string_callback, pnt, cnt) < 0)
  {
    printf("%s: Could not fetch error messages\n", bn);
    return STATE_CRITICAL;
  }

  format_report(cnt, errors, errormsg);

  return STATE_CRITICAL;
}
