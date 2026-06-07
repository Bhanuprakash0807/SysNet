The changes i have made in xv6 in each file:

Changes in proc.h:
    In proc.h i have declared the variables creation_time for FCFS and vruntime,weight,time_slice,nice,slice_start for CFS
Changes in proc.c:
    In allocproc function i have intialized all the newly declared variables which were declared in proc.h
    In scheduler function i had written the code specific for each scheduler FCFS AND CFS integrated with RR
Changes in syscall.c:
   I have added the line extern uint64 sys_getreadcount(void); and [SYS_getreadcount] sys_getreadcount
Changes in syscall.h:
    I have added getreadcount() system call in it
Changes in sysproc.c:
    I have implemented the system call and declared the global variable which counts the number of bytes read
Changes in user.h:
    I have added the new systemcall
 