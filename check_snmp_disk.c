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
 * Checks the disks for a given host via snmp and the
 * ucd snmp interface.
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

#define DISK_INDEX_MIB       ".1.3.6.1.4.1.2021.9.1.1"
#define DISK_PATH_MIB        ".1.3.6.1.4.1.2021.9.1.2"
#define DISK_DEVICE_MIB      ".1.3.6.1.4.1.2021.9.1.3"
#define DISK_MINIMUM_MIB     ".1.3.6.1.4.1.2021.9.1.4"
#define DISK_MINPERCENT_MIB  ".1.3.6.1.4.1.2021.9.1.5"
#define DISK_TOTAL_MIB       ".1.3.6.1.4.1.2021.9.1.6"
#define DISK_AVAIL_MIB       ".1.3.6.1.4.1.2021.9.1.7"
#define DISK_USED_MIB        ".1.3.6.1.4.1.2021.9.1.8"
#define DISK_PERCENT_MIB     ".1.3.6.1.4.1.2021.9.1.9"
#define DISK_PERCENTNODE_MIB ".1.3.6.1.4.1.2021.9.1.10"
#define DISK_ERRORFLAG_MIB   ".1.3.6.1.4.1.2021.9.1.100"
#define DISK_ERRORMSG_MIB    ".1.3.6.1.4.1.2021.9.1.101"

int report_disk(void);

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

  ret = report_disk();

  if(verbose)
    printf("%s: Returning %d\n", bn, ret);

  exit(ret);
}

int report_disk()
{
  int cnt;

  long *errors;
  void *pnt;
  int i;
  int gotErrors = 0;
  char **errormsg = NULL;
  char **diskname = NULL;

  
  if((cnt = fetch_table(DISK_INDEX_MIB, null_callback, NULL, 0)) < 0)
  {
    printf("%s: Could not fetch list of disks\n", bn);
    return STATE_UNKNOWN;
  }

  if(!cnt)  // No disks configured
  {
    printf("%s: No disks found.\n", bn);
    return STATE_OK;
  }
  

  if(!(errors  = calloc(sizeof(long), cnt)))
  {
    printf("%s: Could not allocate memory for information\n", bn);
    return STATE_CRITICAL;
  }

  pnt = errors;
  if(fetch_table(DISK_ERRORFLAG_MIB, integer_callback, pnt, cnt) < 0)
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
  diskname = calloc(sizeof(char **), cnt);
  if(!errormsg || !diskname)
  {
    printf("%s: Could not allocate memory for error information\n", bn);
    return STATE_CRITICAL;
  }

  pnt = errormsg;

  if(fetch_table(DISK_ERRORMSG_MIB, string_callback, pnt, cnt) < 0)
  {
    printf("%s: Could not fetch error messages\n", bn);
    return STATE_CRITICAL;
  }

  pnt = diskname;

  if(fetch_table(DISK_DEVICE_MIB, string_callback, pnt, cnt) < 0)
  {
    printf("%s: Could not fetch device list\n", bn);
    return STATE_CRITICAL;
  }

  for(i=0; i < cnt; i++)
  {
    if(errors[i])
      printf("%s (%s)\n", errormsg[i], diskname[i]);
  }

  if(gotErrors == 0)
  {
	if(listing)
	{
		printf( "Checked %d disks. ( ", cnt );
		for(i=0; i < cnt; i++)
		{
			printf( "%s ", diskname[i] );
		}
		printf(")\n" );
	}
	else
    	printf("Checked %d disks.\n", cnt);
    return STATE_OK;
  }
  
  return STATE_CRITICAL;
}



