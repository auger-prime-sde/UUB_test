#include <stdlib.h>
#include <sys/select.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "time.h"
//#include "fe_lib.h" /*this include automatically the shwr_evt_defs.h */
//#include "fe_kernel_interface_defs.h"
//#include "read_evt.h"
#include "shwr_evt_defs.h"

#include "xparameters.h"
#include "sde_trigger_defs.h"
#include "time_tagging.h"


//#include <ctype.h>
//#include <termios.h>


#define SIG_WAKEUP SIGRTMIN+14

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

struct read_evt_global
{
  uint32_t id_counter;
  uint32_t volatile *shwr_pt[5];
  int shwr_mem_size;

  uint32_t volatile *regs;
  uint32_t volatile *ptr;
  uint32_t volatile *ptr1;
  int regs_size;
  int fd, fd0;
  int vec[500];
  sigset_t sigset; /*used to wake the process periodically */
};

static struct read_evt_global gl;

int main()
{
	  int size;


	gl.fd=open("/dev/mem",O_RDWR);
	  if(gl.fd<0){
	    printf("Error - it was not possible to open the /dev/mem for read/write regitsters\n");
	    exit(1);
	  }
	  size=256*sizeof(uint32_t);
	  if(size%sysconf(_SC_PAGE_SIZE)){
	    size=(size/sysconf(_SC_PAGE_SIZE)+1)*sysconf(_SC_PAGE_SIZE);
	  }
	  gl.regs_size=size;
	  gl.regs=(uint32_t *)mmap(NULL, size,
					PROT_READ | PROT_WRITE, MAP_SHARED,
					gl.fd,SDE_TRIGGER_BASE);
	  //gl.regs=(uint32_t *)mmap(NULL, size,
				//	PROT_READ | PROT_WRITE, MAP_SHARED,
					//gl.fd,0);
	  if(gl.regs==MAP_FAILED){
	    printf("Error - while trying to map the Registers \n");
	    exit(1);
	  }

	  printf("{");
	  printf("\"version\":%x",gl.regs[47]);
	  printf("}");


}


