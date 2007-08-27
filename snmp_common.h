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
 * Common code
 *
 *************************************************************************
 *
 * Distributed under GPL.
 *
 * $Id: snmp_common.h,v 1.2 2002/01/27 15:27:19 henning Exp $
 *
 */

#include "config.h"

#include <sys/types.h>
#include <stdio.h>
#include <libgen.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if HAVE_GETOPT_H
#define _GNU_SOURCE
#include <getopt.h>
#endif

#ifdef HAVE_NET_SNMP
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#endif

/*
 * Callback pointer type
 */
typedef void (*callback_t)(struct variable_list *, void **);

/*
 * Routines from snmp_common.c
 */

void print_help(void);
void print_version(void);
int is_integer(char *);
int fetch_table(char *, callback_t, void *, int);

void null_callback   (struct variable_list *, void ** );
void integer_callback(struct variable_list *, void ** );
void string_callback (struct variable_list *, void ** );

/*
 * getopt
 */

extern char *optarg;
extern int opterr;
extern int optopt;
extern int optind;

/*
 * snmp_common Variables
 *
 */

extern char *bn;         // Basename of the program
extern char *version;    // Version of the Program

extern int timeout;      // Timeout (set with -t)
extern int verbose;      // Verbosity (set with -v)
extern char *community;  // SNMP Community (set with -C)
extern char *hostname;   // Hostname (set with -H)

/*
 * From Nagios Include
 *
 */

#define STATE_OK                        0
#define STATE_WARNING                   1
#define STATE_CRITICAL                  2
#define STATE_UNKNOWN                   3       /* changed from -1 on 02/24/2001 */

