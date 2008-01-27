########################################################################
#
#  (C) 2002-2008
#    Henning P. Schmiedehausen
#    INTERMETA - Gesellschaft fuer Mehrwertdienste mbH
#    Hutweide 15
#    D-91054 Buckenhof
# 
########################################################################

bin_PROGRAMS = check_snmp_disk check_snmp_proc

SNMP_OBJ = snmp_common.o

check_snmp_disk_LDADD = $(SNMP_OBJ)
check_snmp_proc_LDADD = $(SNMP_OBJ)

check_snmp_disk_DEPENDENCIES = $(SNMP_OBJ)
check_snmp_proc_DEPENDENCIES = $(SNMP_OBJ)

snmp_common.o: snmp_common.c snmp_common.h

check_snmp_disk.o: check_snmp_disk.c snmp_common.h
check_snmp_proc.o: check_snmp_proc.c snmp_common.h
