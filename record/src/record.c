// UUB simple signals acquisition utility and store on json file
// written by R.Assiro and G. Marsella
// peak-peak read register written by Fabio Convenga

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
#include <math.h>

//#include "fe_lib.h" /*this include automatically the shwr_evt_defs.h */
//#include "fe_kernel_interface_defs.h"
//#include "read_evt.h"
#include "shwr_evt_defs.h"

#include "xparameters.h"
#include "sde_trigger_defs.h"
#include "time_tagging.h"
#include <time.h>


//#include <ctype.h>
//#include <termios.h>


#define SIG_WAKEUP SIGRTMIN+14

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int max(int a[], int dim);

int max2(int a[], int dim);


struct read_evt_global
{
  uint32_t id_counter;
  uint32_t volatile *shwr_pt[5];
  int shwr_mem_size;

  uint32_t volatile *regs;
  int regs_size;

  sigset_t sigset; /*used to wake the process periodically */
};

static struct read_evt_global gl;

unsigned int shwr_addr[5]={
  TRIGGER_MEMORY_SHWR0_BASE,
  TRIGGER_MEMORY_SHWR1_BASE,
  TRIGGER_MEMORY_SHWR2_BASE,
  TRIGGER_MEMORY_SHWR3_BASE,
  TRIGGER_MEMORY_SHWR4_BASE
};

int main(int argc, char *argv[])
{
    int fd, file,i,j, Status, data_trig, ord, width, time;
    int nev = 4096;
    int value = 0;
    int aux;
    unsigned page_addr, page_offset;
    void *ptr,*pt[5],*ptrt,*ptrt1;
    unsigned page_size=sysconf(_SC_PAGESIZE);
    page_offset = 16;
    FILE *fp, *fp1, *fp2;
    int nevt = 0;
    char c;
    void *map_base, *virt_addr;
	unsigned long read_result, writeval;
	off_t target;
	time = 6200000; //width of pulse 100ns
	width = 10; //number x 100ns

    aux=read_evt_init();
    if(aux!=0){
      printf("FeShwrRead: Problem in start the Front-End - (shower read) %d \n",aux);
      return(0);
    }

    gl.regs[170]=atoi(argv[1]);
   	  gl.regs[171]=atoi(argv[2]);
   	 gl.regs[172]=atoi(argv[3]);
   	 gl.regs[173]=atoi(argv[4]);
   	  gl.regs[SHWR_BUF_TRIG_MASK_ADDR]=atoi(argv[6]);
   	  gl.regs[SB_TRIG_ENAB_ADDR]=atoi(argv[5]);
    clock_t start,end;
    double tempo;
    start=clock();
/*
    char command1[100];
    sprintf(command1, "trigger %d %d %d %d %d %d", atoi(argv[1]),atoi(argv[2]),atoi(argv[3]),atoi(argv[4]),atoi(argv[5]),atoi(argv[6]));
	system(command1);

*/

    while(nevt<1)
    {
    	if (gl.regs[SHWR_BUF_TRIG_MASK_ADDR] == 0x10000){
    		target = 0x43c203fc;
    		        	     if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1);
    		        	     	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    		        	     	virt_addr = map_base + (target & MAP_MASK);

    		        	     	writeval = (time * width);
    		        	     	*((unsigned long *) virt_addr) = writeval;
    		        	     	usleep (100);
    		        	     	 writeval = (time * width) +1;
    		        	     	*((unsigned long *) virt_addr) = writeval;
    		        	     	close(fd);

    	}
    	FeShwrRead_test(1);
  //  	printf("nevt %d\n", nevt);
        nevt++;
    }

    end=clock();
    tempo=((double)(end-start))/CLOCKS_PER_SEC;
   // printf("tempo %lf\n", tempo);
    read_evt_end();

}


/*please do not use the signal SIGRTMIN+15 because it will probaly be
  used in other part of the code ...*/

int read_evt_init()
{
  int fd,i;
  int size;
  struct sigevent sev;
  timer_t t_alarm;

  for(i=0;i<5;i++){
    gl.shwr_pt[i]=NULL;
  }
  gl.regs=NULL;

  fd=open("/dev/mem",O_RDWR);
  if(fd<0){
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
				fd,SDE_TRIGGER_BASE);
  if(gl.regs==MAP_FAILED){
    printf("Error - while trying to map the Registers\n");
    exit(1);
  }
  close(fd); //it is not needed to keep opened

  size=SHWR_MEM_DEPTH*SHWR_MEM_NBUF;
  if(size%sysconf(_SC_PAGE_SIZE)){
    size=(size/sysconf(_SC_PAGE_SIZE)+1)*sysconf(_SC_PAGE_SIZE);
  }
  gl.shwr_mem_size=size;
  for(i=0;i<5;i++){
    fd=open("/dev/mem",O_RDONLY);
    if(fd<0){
      printf("Error - open the /dev/mem for read shower buffers\n");
      exit(1);
    }
    gl.shwr_pt[i]=(uint32_t *)mmap(NULL,size,
				   PROT_READ,MAP_SHARED,
				   fd,shwr_addr[i]);
    if(gl.shwr_pt[i]==MAP_FAILED){
      printf("ERROR- failed to map the shower buffers: %d\n",i);
      exit(1);
    }
    close(fd);
  }



//  	  printf("led %x\n", gl.regs[SHWR_BUF_TRIG_MASK_ADDR]);


  //setting periodical process wakeup to check if there are event. It is ugly, but for now, it would work in this whay, until we figure a what to implement interruptions through the kernel

  //signal of alarm handler - it is going to be just blocked to be used with sigtimedwait system call.

  if(sigemptyset(&gl.sigset)!=0){
    printf("error while trying to set signals ... 1 \n");
    exit(1);
  }
  if(sigaddset(&gl.sigset,SIG_WAKEUP)!=0){
    printf("error while trying to set signals ... 2\n");
    exit(1);
  }
  if(sigprocmask(SIG_BLOCK,&gl.sigset,NULL)!=0){
    printf("error while trying to set signals ... 3 \n");
    exit(1);
  }

  // periodical signal generation
  sev.sigev_notify=SIGEV_SIGNAL;
  sev.sigev_signo=SIG_WAKEUP;
  if(timer_create(CLOCK_MONOTONIC,&sev,&t_alarm)!=0){
    printf("timer creation error\n");
    exit(1);
  } else {
    struct itimerspec ts;
    ts.it_interval.tv_sec=0;
    ts.it_interval.tv_nsec=100000; //.1 ms
    ts.it_value.tv_sec=0;
    ts.it_value.tv_nsec=100000;//the next interruption would appear in .1ms
    if(timer_settime(t_alarm, 0, &ts, NULL)!=0){
      exit(1);
    }
  }
  gl.id_counter=0;
  return(0);
}




int read_evt_end()
{
  int i;
  void *aux;

  if(gl.regs!=NULL){
    aux=(void *)gl.regs;
    munmap(aux,gl.regs_size);
  }
  for(i=0;i<5;i++){
    if(gl.shwr_pt[i]!=NULL){
      aux=(void *)gl.shwr_pt[i];
      munmap(aux,gl.shwr_mem_size);
    }
  }
}

int read_evt_read(struct shwr_evt_raw *shwr)
{
  uint32_t volatile *st;
  void *pt_aux;
  uint32_t aux;
  fd_set frds;
  int rd,sig,i;
  int offset;
  st=&(gl.regs[SHWR_BUF_STATUS_ADDR]);

  /*wait for the periodical signal and check if there is a
    event trigger
  */
  aux=SHWR_BUF_NFULL_MASK<<SHWR_BUF_NFULL_SHIFT;
  sig=SIG_WAKEUP;
  while( ((*st) & aux)==0 && sig==SIG_WAKEUP)
    sig=sigwaitinfo(&gl.sigset,NULL);

  if(sig==SIG_WAKEUP){
    rd=(((*st)>>SHWR_BUF_RNUM_SHIFT) & SHWR_BUF_RNUM_MASK);
    offset=rd*SHWR_NSAMPLES;
    for(i=0;i<5;i++){
      pt_aux=(void *)(gl.shwr_pt[i]+offset);
      memcpy(shwr->fadc_raw[i],
	     pt_aux,
	     sizeof(uint32_t)*SHWR_NSAMPLES);
    }
    shwr->id=gl.id_counter; /*just a internal counter */
    shwr->trace_start=gl.regs[SHWR_BUF_START_ADDR];
    shwr->Evt_type_1=gl.regs[SHWR_BUF_TRIG_ID_ADDR];
    shwr->Evt_type_2=0;
    shwr->ev_gps_info.second=gl.regs[TTAG_SHWR_SECONDS_ADDR];
    shwr->ev_gps_info.ticks=gl.regs[TTAG_SHWR_NANOSEC_ADDR];

    gl.regs[SHWR_BUF_CONTROL_ADDR]=rd;
    gl.id_counter++;
    return(0);
  }
  return(1);
}

FeShwrRead_test(int Nev)
{
  int aux;
  struct shwr_evt_raw evt;
  int nevts,i,j,index;
  int basech0[SHWR_NSAMPLES], basech1[SHWR_NSAMPLES], basech2[SHWR_NSAMPLES],basech3[SHWR_NSAMPLES],basech4[SHWR_NSAMPLES],basech5[SHWR_NSAMPLES],basech6[SHWR_NSAMPLES],basech7[SHWR_NSAMPLES],basech8[SHWR_NSAMPLES],basech9[SHWR_NSAMPLES];
 // FILE *fp;

  while(read_evt_read(&evt)!=0); /*wait for a available event */


	      // for(i=0;i<SHWR_RAW_NCH_MAX;i++){
	   	     for(j=0;j<SHWR_NSAMPLES;j++){
	   	       index=(j+evt.trace_start)%SHWR_NSAMPLES;

	   	    	 basech0[j]=evt.fadc_raw[0][index] & 0xFFF;
	   	    	 basech1[j]=(evt.fadc_raw[0][index]>>16) & 0xFFF;
	   	    	 basech2[j]=evt.fadc_raw[1][index] & 0xFFF;
	   	    	 basech3[j]=(evt.fadc_raw[1][index]>>16) & 0xFFF;
	   	    	 basech4[j]=evt.fadc_raw[2][index] & 0xFFF;
	   	    	 basech5[j]=(evt.fadc_raw[2][index]>>16) & 0xFFF;
	   	    	 basech6[j]=evt.fadc_raw[3][index] & 0xFFF;
	   	    	 basech7[j]=(evt.fadc_raw[3][index]>>16) & 0xFFF;
	   	    	 basech8[j]=evt.fadc_raw[4][index] & 0xFFF;
	   	    	 basech9[j]=(evt.fadc_raw[4][index]>>16) & 0xFFF;
	   	     }
	     //  }


 	  	 int sommach0=0, sommach1=0, sommach2=0, sommach3=0, sommach4=0, sommach5=0, sommach6=0, sommach7=0, sommach8=0, sommach9=0;
 	  	 float mediach0, mediach1, mediach2, mediach3, mediach4, mediach5, mediach6, mediach7, mediach8, mediach9;
 	  	 float scartoch0=0, scartoch1=0, scartoch2=0, scartoch3=0, scartoch4=0, scartoch5=0, scartoch6=0, scartoch7=0, scartoch8=0, scartoch9=0;
 	  	 float scatotch0=0, scatotch1=0, scatotch2=0, scatotch3=0, scatotch4=0, scatotch5=0, scatotch6=0, scatotch7=0, scatotch8=0, scatotch9=0;
 	  	 float rmsch0, rmsch1, rmsch2, rmsch3, rmsch4, rmsch5, rmsch6, rmsch7, rmsch8, rmsch9;



     for(j=0;j<600;j++){

    	 sommach0=sommach0+basech0[j],
         sommach1=sommach1+basech1[j],
         sommach2=sommach2+basech2[j],
         sommach3=sommach3+basech3[j],
         sommach4=sommach4+basech4[j],
         sommach5=sommach5+basech5[j],
         sommach6=sommach6+basech6[j],
         sommach7=sommach7+basech7[j],
         sommach8=sommach8+basech8[j],
         sommach9=sommach9+basech9[j];

       	  }

     mediach0=sommach0/600;
     mediach1=sommach1/600;
     mediach2=sommach2/600;
     mediach3=sommach3/600;
     mediach4=sommach4/600;
     mediach5=sommach5/600;
     mediach6=sommach6/600;
     mediach7=sommach7/600;
     mediach8=sommach8/600;
     mediach9=sommach9/600;

     for(j=0;j<600;j++){

    	 scartoch0=(basech0[j]-mediach0)*(basech0[j]-mediach0);
    	 scatotch0=scatotch0+scartoch0;

    	 scartoch1=(basech1[j]-mediach1)*(basech1[j]-mediach1);
    	 scatotch1=scatotch1+scartoch1;

    	 scartoch2=(basech2[j]-mediach2)*(basech2[j]-mediach2);
    	 scatotch2=scatotch2+scartoch2;

    	 scartoch3=(basech3[j]-mediach3)*(basech3[j]-mediach3);
    	 scatotch3=scatotch3+scartoch3;

    	 scartoch4=(basech4[j]-mediach4)*(basech4[j]-mediach4);
    	 scatotch4=scatotch4+scartoch4;

    	 scartoch5=(basech5[j]-mediach5)*(basech5[j]-mediach5);
    	 scatotch5=scatotch5+scartoch5;

    	 scartoch6=(basech6[j]-mediach6)*(basech6[j]-mediach6);
    	 scatotch6=scatotch6+scartoch6;

    	 scartoch7=(basech7[j]-mediach7)*(basech7[j]-mediach7);
    	 scatotch7=scatotch7+scartoch7;

    	 scartoch8=(basech8[j]-mediach8)*(basech8[j]-mediach8);
    	 scatotch8=scatotch8+scartoch8;

    	 scartoch9=(basech9[j]-mediach9)*(basech9[j]-mediach9);
    	 scatotch9=scatotch9+scartoch9;

     }


     rmsch0=sqrt(scatotch0/600);
     rmsch1=sqrt(scatotch1/600);
     rmsch2=sqrt(scatotch2/600);
     rmsch3=sqrt(scatotch3/600);
     rmsch4=sqrt(scatotch4/600);
     rmsch5=sqrt(scatotch5/600);
     rmsch6=sqrt(scatotch6/600);
     rmsch7=sqrt(scatotch7/600);
     rmsch8=sqrt(scatotch8/600);
     rmsch9=sqrt(scatotch9/600);



        printf("{");
     	   printf("\"baseline\":");
     	   printf("[");
     	   printf("{");

     	   printf("\"baseline_adc0\": \"%lf\"",mediach0);
     	   printf(", \"baseline_adc1\": \"%lf\"",mediach1);
     	   printf(", \"baseline_adc2\": \"%lf\"",mediach2);
     	   printf(", \"baseline_adc3\": \"%lf\"",mediach3);
     	   printf(", \"baseline_adc4\": \"%lf\"",mediach4);
     	   printf(", \"baseline_adc5\": \"%lf\"",mediach5);
     	   printf(", \"baseline_adc6\": \"%lf\"",mediach6);
     	   printf(", \"baseline_adc7\": \"%lf\"",mediach7);
     	   printf(", \"baseline_adc8\": \"%lf\"",mediach8);
     	   printf(", \"baseline_adc9\": \"%lf\"",mediach9);



     	      printf("}");
 		      printf("]");
	 	  	  printf(", ");

	 	  	printf("\"RMS\":");
	 	  	     	   printf("[");
	 	  	     	   printf("{");

	 	  	     	   printf("\"rms_adc0\": \"%lf\"",rmsch0);
	 	  	     	   printf(", \"rms_adc1\": \"%lf\"",rmsch1);
	 	  	     	   printf(", \"rms_adc2\": \"%lf\"",rmsch2);
	 	  	     	   printf(", \"rms_adc3\": \"%lf\"",rmsch3);
	 	  	     	   printf(", \"rms_adc4\": \"%lf\"",rmsch4);
	 	  	     	   printf(", \"rms_adc5\": \"%lf\"",rmsch5);
	 	  	     	   printf(", \"rms_adc6\": \"%lf\"",rmsch6);
	 	  	     	   printf(", \"rms_adc7\": \"%lf\"",rmsch7);
	 	  	     	   printf(", \"rms_adc8\": \"%lf\"",rmsch8);
	 	  	     	   printf(", \"rms_adc9\": \"%lf\"",rmsch9);



	 	  	     	      printf("}");
	 	  	 		      printf("]");
	 	  		 	  	  printf(", ");



	 	    printf("\"Pulsepos\":");
	 	  		 	  	     	   printf("[");
	 	  		 	  	     	   printf("{");

	 	  		 	  	     	   printf("\"pulsepos_adc0\": \"%d\"",max(basech0,SHWR_NSAMPLES));
	 	  		 	  	     	   printf(", \"pulsepos_adc1\": \"%d\"",max(basech1,SHWR_NSAMPLES));
	 	  		 	  	     	   printf(", \"pulsepos_adc2\": \"%d\"",max(basech2,SHWR_NSAMPLES));
	 	  		 	  	     	   printf(", \"pulsepos_adc3\": \"%d\"",max(basech3,SHWR_NSAMPLES));
	 	  		 	  	     	   printf(", \"pulsepos_adc4\": \"%d\"",max(basech4,SHWR_NSAMPLES));
	 	  		 	  	     	   printf(", \"pulsepos_adc5\": \"%d\"",max(basech5,SHWR_NSAMPLES));
	 	  		 	  	     	   printf(", \"pulsepos_adc6\": \"%d\"",max(basech6,SHWR_NSAMPLES));
	 	  		 	  	     	   printf(", \"pulsepos_adc7\": \"%d\"",max(basech7,SHWR_NSAMPLES));
	 	  		 	  	     	   printf(", \"pulsepos_adc8\": \"%d\"",max(basech8,SHWR_NSAMPLES));
	 	  		 	  	     	   printf(", \"pulsepos_adc9\": \"%d\"",max(basech9,SHWR_NSAMPLES));

	 	  		 	  	     	      printf("}");
	 	  		 	  	 		      printf("]");
	 	  		 	  		 	  	  printf(", ");

	 	  		 	  		 	   printf("\"valuepeak\":");
	 	  		 	  		 	   printf("[");
	 	  		 	  		 	   printf("{");

	 	  		 	  		 	   		          printf ("\"peak0\": \"%d\"",(gl.regs[SHWR_PEAK_AREA0_ADDR] >> SHWR_PEAK_SHIFT) & SHWR_PEAK_MASK);
	 	  		 	  		 	   		          printf(", \"peak1\": \"%d\"",(gl.regs[SHWR_PEAK_AREA1_ADDR] >> SHWR_PEAK_SHIFT) & SHWR_PEAK_MASK);
	 	  		 	  		 	   		          printf(", \"peak2\": \"%d\"",(gl.regs[SHWR_PEAK_AREA2_ADDR] >> SHWR_PEAK_SHIFT) & SHWR_PEAK_MASK);
	 	  		 	  		 	   		          printf(", \"peak3\": \"%d\"",(gl.regs[SHWR_PEAK_AREA3_ADDR] >> SHWR_PEAK_SHIFT) & SHWR_PEAK_MASK);
	 	  		 	  		 	   		          printf (", \"peak4\": \"%d\"",(gl.regs[SHWR_PEAK_AREA4_ADDR] >> SHWR_PEAK_SHIFT) & SHWR_PEAK_MASK);
	 	  		 	  		 	   		          printf(", \"peak5\": \"%d\"",(gl.regs[SHWR_PEAK_AREA5_ADDR] >> SHWR_PEAK_SHIFT) & SHWR_PEAK_MASK);
	 	  		 	  		 	   		          printf (", \"peak6\": \"%d\"",(gl.regs[SHWR_PEAK_AREA6_ADDR] >> SHWR_PEAK_SHIFT) & SHWR_PEAK_MASK);
	 	  		 	  		 	   		          printf(", \"peak7\": \"%d\"",(gl.regs[SHWR_PEAK_AREA7_ADDR] >> SHWR_PEAK_SHIFT) & SHWR_PEAK_MASK);
	 	  		 	  		 	   		          printf (", \"peak8\": \"%d\"",(gl.regs[SHWR_PEAK_AREA8_ADDR] >> SHWR_PEAK_SHIFT) & SHWR_PEAK_MASK);
	 	  		 	  		 	   		          printf(", \"peak9\": \"%d\"",(gl.regs[SHWR_PEAK_AREA9_ADDR] >> SHWR_PEAK_SHIFT) & SHWR_PEAK_MASK);

	 	  		 	  			 	   		      printf("}");
     	 	  	 	  		 	  		 	   	 printf("]");
	   	  		 		  	     	      printf("}");

/*
     int ind=max(basech0,SHWR_NSAMPLES);
     int ind2=max2(basech0,SHWR_NSAMPLES);
     printf("pulspos=%d, max=%d\n",ind,ind2);


	     for(j=0;j<SHWR_NSAMPLES;j++){

	   index=(j+evt.trace_start)%SHWR_NSAMPLES;

	   printf( "base=%d j=%d\n"	, basech0[j],j);
//	   printf( "evt=%d index=%d\n"	, evt.fadc_raw[0][index] & 0xFFF, index);

	     }
*/
 //	 fclose(fp);

}



int usage(void)
{
    printf("____________________________\n");
    printf("|   SCOPE (trigger option) |\n");
    printf("|   -e External SMA UUB    |\n");
    printf("|   -i internal trigger    |\n");
    printf("|   -t threshold trigger   |\n");
    printf("|   -l LED acquisition     |\n");
    printf("|                          |\n");
    printf("|    written by R.Assiro   |\n");
    printf("|      and G.Marsella      |\n");
    printf("|______and F.Convenga______|\n");
    exit(0);
}

int max(int a[], int dim)
{
    int max=0;
    int i,k;
  for (i=0; i<dim; i++) {
      if (max<a[i])
      {
          max=a[i];
          k=i;
      }
  }
return k;
}

int max2(int a[], int dim)
{
    int max=0;
    int i,k;
  for (i=0; i<dim; i++) {
      if (max<a[i])
      {
          max=a[i];
          k=i;
      }
  }
return max;
}
