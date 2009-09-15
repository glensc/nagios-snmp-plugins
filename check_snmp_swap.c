/*************************************************************************
 *
 * plugins for Nagios
 *
 * (C) 2002-2008
 *   Henning P. Schmiedehausen
 *   INTERMETA - Gesellschaft fuer Mehrwertdienste mbH
 *   Hutweide 15
 *   D-91054 Buckenhof
 * (C) 2009
 *   Elan Ruusamäe, <glen@delfi.ee>
 *
 *************************************************************************
 *
 * Checks the swap for a given host via snmp and the ucd snmp interface.
 *
 *************************************************************************
 *
 * Distributed under GPL.
 *
 * $Id: disk_plugin.c,v 1.6 2002/01/27 22:10:24 henning Exp $
 *
 */

#include "snmp_common.h"

#define RCS_VERSION "$Revision: 1.6 $ ($Date: 2002/01/27 22:10:24 $)"

#define MEM_INDEX_MIB        ".1.3.6.1.4.1.2021.4.1"
#define MEM_ERRORFLAG_MIB    ".1.3.6.1.4.1.2021.4.100"
#define MEM_ERRORMSG_MIB     ".1.3.6.1.4.1.2021.4.101"

int report_swap(void);

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

  ret = report_swap();

  if(verbose)
    printf("%s: Returning %d\n", bn, ret);

  exit(ret);
}

int report_swap()
{
  int cnt;

  long *errors;
  void *pnt;
  int i;
  int gotErrors = 0;
  char **errormsg = NULL;

  
  if((cnt = fetch_table(MEM_INDEX_MIB, null_callback, NULL, 0)) < 0)
  {
    printf("%s: Could not fetch mem index\n", bn);
    return STATE_UNKNOWN;
  }

  if(!cnt)  // not configured
  {
    printf("%s: Not configure.\n", bn);
    return STATE_WARNING;
  }
  

  if(!(errors  = calloc(sizeof(long), cnt)))
  {
    printf("%s: Could not allocate memory for information\n", bn);
    return STATE_CRITICAL;
  }

  pnt = errors;
  if(fetch_table(MEM_ERRORFLAG_MIB, integer_callback, pnt, cnt) < 0)
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

  errormsg = calloc(sizeof(char **), cnt);
  if(!errormsg)
  {
    printf("%s: Could not allocate memory for error information\n", bn);
    return STATE_CRITICAL;
  }

  pnt = errormsg;

  if(fetch_table(MEM_ERRORMSG_MIB, string_callback, pnt, cnt) < 0)
  {
    printf("%s: Could not fetch error messages\n", bn);
    return STATE_CRITICAL;
  }

  for(i=0; i < cnt; i++)
  {
    if(errors[i])
      printf("%s\n", errormsg[i]);
  }

  if(gotErrors == 0)
  {
	printf("Checked memory for %d entries.\n", cnt);
    return STATE_OK;
  }
  
  return STATE_CRITICAL;
}
