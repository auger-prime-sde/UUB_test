#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sde_sc.h"
#define VERSION_STRING " Version 3.1"
#define LSB_TO_5V 1.868
#define LSB_TO_24V 8.88
#define LSB_TO_12V 4.43
#define LSB_TO_3V3 1.20
#define LSB_TO_1V8 0.674
#define LSB_TO_1V2 0.421
#define LSB_TO_1V0 0.366
#define SC_ADDR		0x0f
char buf[160];
short int adc_buffer[MAX_VARS];
static const char *optString = "d:w:VW:klL:rsStp:v:P::I::Aah?";
void higth();
void write_array(float *pmtvalue, int flag);
void reset();
//void write_hig();
void low();
int old_hv1, old_hv2, old_hv3, old_hv4, old_hv5, old_hv6;
//void write_low();
/* Read SlowControl serial Number */
void sc_serial ( int file, char *b)
{   char reg[] ={0x01, 0x00};

//	reg[0] = 0x01;
//  reg[2] = 0x00;
	  if (write(file, reg, 2) != 2) {
	          	 	exit(3);
	           	}
	  usleep (100000);
	  if (read(file,b,8)!= 8) {
		  	  	  	exit(4);
	  }
	  return ;
}
void sc_status (int file, char *b)
{   char reg[] ={0x02, 0x00};
     if (write(file, reg, 2) != 2) {
        	 	exit(3);
     }
     usleep (100000);
     if (read(file,b,2)!= 2) {
	  	  	  	exit(4);
	 }
     return ;
}
void sc_livetime (int file , int *l)
{char reg [6] = {0x03, 0x00, 0x0, 0x0, 0x0, 0x0};
 char b[4];
	if (write(file, reg, 2) != 2) {
        	 	exit(3);
    }
	usleep (100000);
	if (read(file,b,4)!= 4) {
		  	  	exit(4);
	}
//	printf("b: 0x%x 0x%x 0x%x 0x%x\n", b[0], b[1], b[2], b[3]);
	*l =  b[0]
		+ ( b[1] << 8 )
		+ ( b[2] << 16 )
		+ ( b[3] << 24 );
}
void sc_watchdog (int file, int value)
{	char reg [4] = {0x07, 0x00, 0x0, 0x0};
	reg [2] = (char) (value & 0xff);
	if (write (file, reg, 4) != 4) {
		exit (3);
	}

	return;
}
void sc_ADC_en (int file, int value)
{	char reg [4] = {0x0d, 0x00, 0x0, 0x0};
	reg [2] = (char) (value & 0xff);
	if (write (file, reg, 4) != 4) {
		exit (3);
	}

	return;
}
void sc_ANALOG ( int file, int value)
{	char reg [4] = {0x10, 0x00, 0x0, 0x0};
	reg [2] = (char) (value & 0xff);
	if (write (file, reg, 4) != 4) {
		exit (3);
	}
}
void kill( int file )
{
	char reg[2] = {0x0b, 0x00};
	if (write (file, reg, 2) != 2) {
		exit (3);
	}

}
void radio_rst (int file)
{
	char reg[2] = {0x0c, 0x00};
	if (write (file, reg, 2) != 2) {
		exit (3);
	}


}
void sc_version ( int file, char*b )
{ char reg[] = {0x0e,0x00};
if (write(file, reg, 2) != 2) {
        	 	exit(3);
     }
     usleep (100000);
     if (read(file,b,2)!= 2) {
     	  	  	  	exit(4);
     }
     return ;
}
void sc_test_reg (int file, char *b)
{   char reg[] ={0x0a, 0x00};
     if (write(file, reg, 2) != 2) {
        	 	exit(3);
     }
     usleep (100000);
     if (read(file,b,2)!= 2) {
	  	  	  	exit(4);
	 }
     return ;
}
void sc_get_ADC_values (int file) //, char *b)
{   char reg[] ={0x09, 0x00};
     if (write(file, reg, 2) != 2) {
        	 	exit(3);
     }
     usleep (100000);
     if (read(file,(char *)adc_buffer,2*MAX_VARS)!= 2*MAX_VARS) {
	  	  	  	exit(4);
	 }

     return ;
}
void sc_powerControl_reg (int file, char *b)
{   char reg[] ={0x04, 0x00};
     if (write(file, reg, 2) != 2) {
        	 	exit(3);
     }
     usleep (100000);
     if (read(file,b,2)!= 2) {
	  	  	  	exit(4);
	 }
     return ;
}
void sc_powerControl_reg_w (int file, char *b)
{   char reg[4] ={0x04, 0x00, 0x0, 0x0};
	 reg [2] = *b++;
	 reg [3] = *b;
	 if (write(file, reg, 4) != 4) {
        	 	exit(3);
     }
     usleep (100000);
     return ;
}
void sc_ident_reg_w( int file, char*buf)
{
int i;
char reg[6] ={0x0f, 0x00, 0x0, 0x0, 0x0,0x0};
	for (i=0;i<4; i++) {
		reg[i+2] = buf[i];

	}
	 if (write(file, reg, 6) != 6) {
       	 	exit(3);
    }
	 return;
}

void sc_ident_reg( int file, char*buf)
{
char reg[5] ={0x0f, 0x00, 0x0, 0x0, 0x0};
	 if (write(file, reg, 2) != 2) {
       	 	exit(3);
    }
     usleep (100000);
     if (read(file,buf,4)!= 4) {
	  	  	  	exit(4);
	 }
     return;
}

void sc_set_dac (int file, int chan, int value)
{	char reg[4] ={0x05, 0x00, 0x0, 0x0};
	static char ch_msk[6] = {0x00, 0x20, 0x60, 0x30, 0x40, 0x50};
		chan = chan -1;
		if ( (chan >= 0 && chan <=5) &&
				(value >=0 && value <= 4095) ) {
		 reg [2] = (char) (value & 0xff);
		 reg [3] = ch_msk[chan] | (char) ((value >>8) & 0x0f);
//		 printf ("DAC: 0x%.2x%.2x\n",reg[3],reg[2]);
		 if (write(file, reg, 4) != 4) {
	        	 exit(3);
	     	 	 }
	     usleep (100000);
		 } else printf ( "invalid value %d %d\n", chan+1, value);
	 return ;
}


void display_usage( char *s )
{
    puts( "Usage:" );
    printf ( "%s [-aAst] [-P[HEX]] [-p P_ARG -v V_ARG]\n", s);
    puts ("Options:");
    puts ("-a \t show a map of environment variables in human readable form");
    puts ("-A \t show a map of environment variables as hex raw data");
    puts ("-s \t show content of status register in hex");
    puts ("-t \t show content of test register [0x4321]");
    puts ("-P \t if [HEX] omitted show content of power control register in hex");
    puts ("   \t with [HEX] set power control register to [HEX]");
    puts ("   \t the program takes care that you cannot switch off power supplies used by FPGA");
    puts ("-p");
    puts ("-v \t -p -v are used to set the high voltage for PMT1-6 P_ARG=(1-6) ");
    puts ("   \t to value V_ARG(0x0....0x0fff)");
    puts ("   \t for further information refer to the slow control register map documentation");
    exit( EXIT_FAILURE );
}



int main( int argc, char *argv[])
{
	FILE *fp, *fp1;
	int i;
	float pmtlow[10], pmthig[10];
	float pmtlow_cur[10], pmthig_cur[10];
	float pmtlow_tm[10], pmthig_tm[10];

	low();
	fflush(stdout);
	sleep(2);
	write_array(pmtlow,0);
	write_array(pmtlow_cur,1);
	write_array(pmtlow_tm,2);
	fflush(stdout);
	//fflush(stdout);
	sleep(3);
	higth();
    fflush(stdout);
	sleep(2);
	write_array(pmthig,0);
	write_array(pmthig_cur,1);
	write_array(pmthig_tm,2);



	//JSON

	 printf("{");

	 printf ("\"PMT1_HVM_diff\":%.f,",pmthig[0]-pmtlow[0]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT2_HVM_diff\":%.f,",pmthig[1]-pmtlow[1]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT3_HVM_diff\":%.f,",pmthig[2]-pmtlow[2]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT4_HVM_diff\":%.f,",pmthig[3]-pmtlow[3]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT5_HVM_diff\":%.f,",pmthig[4]-pmtlow[4]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT6_HVM_diff\":%.f,",pmthig[5]-pmtlow[5]);// aggiunta correzione !!!!!!!!!!!!!!

	 printf ("\"PMT1_CM_diff\":%.f,",pmthig_cur[0]-pmtlow_cur[0]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT2_CM_diff\":%.f,",pmthig_cur[1]-pmtlow_cur[1]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT3_CM_diff\":%.f,",pmthig_cur[2]-pmtlow_cur[2]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT4_CM_diff\":%.f,",pmthig_cur[3]-pmtlow_cur[3]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT5_CM_diff\":%.f,",pmthig_cur[4]-pmtlow_cur[4]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT6_CM_diff\":%.f,",pmthig_cur[5]-pmtlow_cur[5]);// aggiunta correzione !!!!!!!!!!!!!!


	 printf ("\"PMT1_TM_diff\":%.f,",pmthig_tm[0]-pmtlow_tm[0]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT2_TM_diff\":%.f,",pmthig_tm[1]-pmtlow_tm[1]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT3_TM_diff\":%.f,",pmthig_tm[2]-pmtlow_tm[2]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT4_TM_diff\":%.f,",pmthig_tm[3]-pmtlow_tm[3]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT5_TM_diff\":%.f,",pmthig_tm[4]-pmtlow_tm[4]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT6_TM_diff\":%.f",pmthig_tm[5]-pmtlow_tm[5]);// aggiunta correzione !!!!!!!!!!!!!!

	 printf("}");

reset();
return 0;

}

void reset(){

	  int opt = 0;


		int file, i, j, ch, val;
		char filename[20];

	//	opt = getopt( argc, argv, optString );
	  /* Geraetedatei oeffnen */

	//  printf("Opening device... ");
	  snprintf(filename, 19, "/dev/i2c-0");
	  	file = open(filename, O_RDWR);
	  	if (file < 0) {
	//  			printf("error opening %s\n",filename);
	  		perror(" Error opening file");
	  			exit (1);
	  	}
	  	if (ioctl(file, I2C_SLAVE, SC_ADDR) < 0) {
	  			printf("Fail to setup slave addr!\n");
	  			exit (1);
	  	}

 //	 sc_get_ADC_values (file);

 	             sc_set_dac (file, 1, old_hv1*1.306);
 		 	 	 sc_set_dac (file, 2, old_hv2*1.306);
 		 	 	 sc_set_dac (file, 3, old_hv3*1.306);
 		 	 	 sc_set_dac (file, 4, old_hv4*1.306);
 		 	 	 sc_set_dac (file, 5, old_hv5*1.306);
 		 	 	 sc_set_dac (file, 6, old_hv6*1.306);
}


void write_array(float *pmtvalue, int flag)
{
	  int opt = 0;


		int file, i, j, ch, val;
		char filename[20];

	//	opt = getopt( argc, argv, optString );
	  /* Geraetedatei oeffnen */

	//  printf("Opening device... ");
	  snprintf(filename, 19, "/dev/i2c-0");
	  	file = open(filename, O_RDWR);
	  	if (file < 0) {
	//  			printf("error opening %s\n",filename);
	  		perror(" Error opening file");
	  			exit (1);
	  	}
	  	if (ioctl(file, I2C_SLAVE, SC_ADDR) < 0) {
	  			printf("Fail to setup slave addr!\n");
	  			exit (1);
	  	}

   	 sc_get_ADC_values (file);


if(flag==0){
	 pmtvalue[0]=(float)adc_buffer[PMT1_HVM]*LSB_TO_5V*1.;
	 pmtvalue[1]=(float)adc_buffer[PMT2_HVM]*LSB_TO_5V*1.;
	 pmtvalue[2]=(float)adc_buffer[PMT3_HVM]*LSB_TO_5V*1.;
	 pmtvalue[3]=(float)adc_buffer[PMT4_HVM]*LSB_TO_5V*1.;
	 pmtvalue[4]=(float)adc_buffer[PMT5_HVM]*LSB_TO_5V*1.;
	 pmtvalue[5]=(float)adc_buffer[PMT6_HVM]*LSB_TO_5V*1.;



};


if(flag==1){
	 pmtvalue[0]=(float)adc_buffer[PMT1_CM]*LSB_TO_5V*1.;
	 pmtvalue[1]=(float)adc_buffer[PMT2_CM]*LSB_TO_5V*1.;
	 pmtvalue[2]=(float)adc_buffer[PMT3_CM]*LSB_TO_5V*1.;
	 pmtvalue[3]=(float)adc_buffer[PMT4_CM]*LSB_TO_5V*1.;
	 pmtvalue[4]=(float)adc_buffer[PMT5_CM]*LSB_TO_5V*1.;
	 pmtvalue[5]=(float)adc_buffer[PMT6_CM]*LSB_TO_5V*1.;
};


if(flag==2){
	 pmtvalue[0]=(float)(adc_buffer[PMT1_TM])*LSB_TO_5V;
	 pmtvalue[1]=(float)(adc_buffer[PMT2_TM])*LSB_TO_5V;
	 pmtvalue[2]=(float)(adc_buffer[PMT3_TM])*LSB_TO_5V;
	 pmtvalue[3]=(float)(adc_buffer[PMT4_TM])*LSB_TO_5V;
	 pmtvalue[4]=(float)(adc_buffer[PMT5_TM])*LSB_TO_5V;
	 pmtvalue[5]=(float)(adc_buffer[PMT6_TM])*LSB_TO_5V;
};


  }

void higth(){

	  int opt = 0;


		int file, i, j, ch, val;
		char filename[20];

	//	opt = getopt( argc, argv, optString );
	  /* Geraetedatei oeffnen */

	//  printf("Opening device... ");
	  snprintf(filename, 19, "/dev/i2c-0");
	  	file = open(filename, O_RDWR);
	  	if (file < 0) {
	//  			printf("error opening %s\n",filename);
	  		perror(" Error opening file");
	  			exit (1);
	  	}
	  	if (ioctl(file, I2C_SLAVE, SC_ADDR) < 0) {
	  			printf("Fail to setup slave addr!\n");
	  			exit (1);
	  	}


	  sc_set_dac (file, 1, 4095);
	  sc_set_dac (file, 2, 4095);
	  sc_set_dac (file, 3, 4095);
	  sc_set_dac (file, 4, 4095);
	  sc_set_dac (file, 5, 4095);
	  sc_set_dac (file, 6, 4095);

}


void low(){

	  int opt = 0;


		int file, i, j, ch, val;
		char filename[20];

	//	opt = getopt( argc, argv, optString );
	  /* Geraetedatei oeffnen */

	//  printf("Opening device... ");
	  snprintf(filename, 19, "/dev/i2c-0");
	  	file = open(filename, O_RDWR);
	  	if (file < 0) {
	//  			printf("error opening %s\n",filename);
	  		perror(" Error opening file");
	  			exit (1);
	  	}
	  	if (ioctl(file, I2C_SLAVE, SC_ADDR) < 0) {
	  			printf("Fail to setup slave addr!\n");
	  			exit (1);
	  	}

   	 //sc_get_ADC_values (file);

         sc_set_dac (file, 1, 0);
	 	 sc_set_dac (file, 2, 0);
	 	 sc_set_dac (file, 3, 0);
	 	 sc_set_dac (file, 4, 0);
	 	 sc_set_dac (file, 5, 0);
	 	 sc_set_dac (file, 6, 0);
}

