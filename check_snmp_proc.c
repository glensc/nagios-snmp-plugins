/*************************************************************************
 *
 * plugins for Netsaint
 *
 * (C) 2002-2004
 *   Henning P. Schmiedehausen
 *   INTERMETA - Gesellschaft fuer Mehrwertdienste mbH
 *   Am Schwabachgrund 22
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
    { 0, 0, 0, 0 },
  };
  int option_index = 0;
  int c;

  int ret = STATE_UNKNOWN;

  bn = strdup(basename(argv[0]));
  version = VERSION;

#define OPTS "?hVvt:c:w:C:H:"
  
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

int report_proc()
{
  int cnt;

  long *errors;
  void *pnt;
  int i;
  int gotErrors = 0;
  char **errormsg = NULL;
  
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

  if(gotErrors == 0)
  {
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
  
  for(i=0; i < cnt; i++)
  {
    if(errors[i])
      printf("%s\n", errormsg[i]);
  }

  return STATE_CRITICAL;
}



