#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sde_sc.h"
#define SC_ADDR		0x0f
char buf[160];
static const char *optString = "sStp:v:P::Aah?";
void higth();
void write_array(float *pmtvalue);

//void write_hig();
void low();
//void write_low();
/* Read SlowControl serial Number */
void sc_serial ( int file, char *b)
{   char reg[] ={0x01, 0x00};

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
void sc_set_dac (int file, int chan, int value)
{	char reg[4] ={0x05, 0x00, 0x0, 0x0};
	static char ch_msk[6] = {0x00, 0x20, 0x60, 0x30, 0x40, 0x50};
		chan = chan -1;
		if ( (chan >= 0 && chan <=5) &&
				(value >=0 && value <= 4095) ) {
		 reg [2] = (char) (value & 0xff);
		 reg [3] = ch_msk[chan] | (char) ((value >>8) & 0x0f);
		 //printf ("DAC: 0x%.2x%.2x\n",reg[3],reg[2]);
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
	higth();
	sleep(5);
	write_array(pmthig);
	sleep(5);
	low();
	sleep(5);
	write_array(pmtlow);

	//JSON

	 printf("{");

	 printf ("\"PMT1_HVM_diff\":%.1f,",pmthig[0]-pmtlow[0]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT2_HVM_diff\":%.1f,",pmthig[1]-pmtlow[1]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT3_HVM_diff\":%.1f,",pmthig[2]-pmtlow[2]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT4_HVM_diff\":%.1f,",pmthig[3]-pmtlow[3]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT5_HVM_diff\":%.1f,",pmthig[4]-pmtlow[4]);// aggiunta correzione !!!!!!!!!!!!!!
	 printf ("\"PMT6_HVM_diff\":%.1f",pmthig[5]-pmtlow[5]);// aggiunta correzione !!!!!!!!!!!!!!

	 printf("}");


  }

void write_array(float *pmtvalue)
{

    int opt = 0;
	int file, i, j, ch, val,a,b,c,d,e,f,g,h;
	FILE *fp, *fp1;
	char filename[20];
	char filename1[100];
	char namedate[20];
	time_t rawtime;
	struct tm *timeinfo;
 	char buffer[80];
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	strftime(namedate,20,"%y%m%d%H%M%S",timeinfo);

//  printf("Settings Slow Control... ");
  snprintf(filename, 19, "/dev/i2c-0");
  	file = open(filename, O_RDWR);
  	if (file < 0) {
  			exit("no open file");
  	}
  	if (ioctl(file, I2C_SLAVE, SC_ADDR) < 0) {
  			exit("Fail to setup slave addr!");
  	}



// system ("SiPM_control HPO");

// data from SiPM if connected
	#define BUFSIZE 128
    char *cmd = "SiPM_control HPO";
    char *buf[BUFSIZE];
    FILE *fg;
    if ((fg = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return -1;
    }


    char *string = buf;
//    printf("%c\n", string[1]);
    float dummy, voltage, current, temp;
    char string1[12],string2[10],string3[5],string4[7];

   while (fgets(buf, BUFSIZE, fg) != NULL) {
//    		printf("%s", buf);
    		sscanf (buf, "%s %s %s %s %f %f %f %f %f", string1,
    		   	  string2,string3,string4,&dummy,&voltage,&current,&temp);
    }

/*    int count;
	for (count = 0; count < 7; count++){
		printf("%s ", buf);
	}
*/

  char dac_ok;
  dac_ok = 0x0;
  sc_get_ADC_values (file);



	 pmtvalue[0]=(float)adc_buffer[PMT1_HVM] *LSB_TO_5V* 2.2;
	 pmtvalue[1]=(float)adc_buffer[PMT2_HVM] *LSB_TO_5V* 2.2;
	 pmtvalue[2]=(float)adc_buffer[PMT3_HVM] *LSB_TO_5V* 2.2;
	 pmtvalue[3]=(float)adc_buffer[PMT4_HVM] *LSB_TO_5V* 2.2;
	 pmtvalue[4]=(float)adc_buffer[PMT5_HVM] *LSB_TO_5V* 2.2;
	 pmtvalue[5]=(float)adc_buffer[PMT6_HVM] *LSB_TO_5V* 2.2;



  }

void higth(){
	 int opt = 0;
		int file, i, j, ch, val,a,b,c,d,e,f,g,h;
		FILE *fp, *fp1;
		char filename[20];
		char filename1[100];
		char namedate[20];
		time_t rawtime;
		struct tm *timeinfo;
	 	char buffer[80];
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		strftime(namedate,20,"%y%m%d%H%M%S",timeinfo);

	//  printf("Settings Slow Control... ");
	  snprintf(filename, 19, "/dev/i2c-0");
	  	file = open(filename, O_RDWR);
	  	if (file < 0) {
	  			exit("no open file");
	  	}
	  	if (ioctl(file, I2C_SLAVE, SC_ADDR) < 0) {
	  			exit("Fail to setup slave addr!");
	  	}



	// system ("SiPM_control HPO");

	// data from SiPM if connected
		#define BUFSIZE 128
	    char *cmd = "SiPM_control HPO";
	    char *buf[BUFSIZE];
	    FILE *fg;
	    if ((fg = popen(cmd, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        return -1;
	    }


	    char *string = buf;
	//    printf("%c\n", string[1]);
	    float dummy, voltage, current, temp;
	    char string1[12],string2[10],string3[5],string4[7];

	   while (fgets(buf, BUFSIZE, fg) != NULL) {
	//    		printf("%s", buf);
	    		sscanf (buf, "%s %s %s %s %f %f %f %f %f", string1,
	    		   	  string2,string3,string4,&dummy,&voltage,&current,&temp);
	    }

	/*    int count;
		for (count = 0; count < 7; count++){
			printf("%s ", buf);
		}
	*/

	  char dac_ok;
	  dac_ok = 0x0;
	  sc_get_ADC_values (file);

	  sc_set_dac (file, 1, 4095);
	  sc_set_dac (file, 2, 4095);
	  sc_set_dac (file, 3, 4095);
	  sc_set_dac (file, 4, 4095);
	  sc_set_dac (file, 5, 4095);
	  sc_set_dac (file, 6, 4095);

return 0;
}


void low(){
	 int opt = 0;
		int file, i, j, ch, val,a,b,c,d,e,f,g,h;
		FILE *fp, *fp1;
		char filename[20];
		char filename1[100];
		char namedate[20];
		time_t rawtime;
		struct tm *timeinfo;
	 	char buffer[80];
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		strftime(namedate,20,"%y%m%d%H%M%S",timeinfo);

	//  printf("Settings Slow Control... ");
	  snprintf(filename, 19, "/dev/i2c-0");
	  	file = open(filename, O_RDWR);
	  	if (file < 0) {
	  			exit("no open file");
	  	}
	  	if (ioctl(file, I2C_SLAVE, SC_ADDR) < 0) {
	  			exit("Fail to setup slave addr!");
	  	}



	// system ("SiPM_control HPO");

	// data from SiPM if connected
		#define BUFSIZE 128
	    char *cmd = "SiPM_control HPO";
	    char *buf[BUFSIZE];
	    FILE *fg;
	    if ((fg = popen(cmd, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        return -1;
	    }


	    char *string = buf;
	//    printf("%c\n", string[1]);
	    float dummy, voltage, current, temp;
	    char string1[12],string2[10],string3[5],string4[7];

	   while (fgets(buf, BUFSIZE, fg) != NULL) {
	//    		printf("%s", buf);
	    		sscanf (buf, "%s %s %s %s %f %f %f %f %f", string1,
	    		   	  string2,string3,string4,&dummy,&voltage,&current,&temp);
	    }

	/*    int count;
		for (count = 0; count < 7; count++){
			printf("%s ", buf);
		}
	*/

	  char dac_ok;
	  dac_ok = 0x0;
	  sc_get_ADC_values (file);

         sc_set_dac (file, 1, 0);
	 	 sc_set_dac (file, 2, 0);
	 	 sc_set_dac (file, 3, 0);
	 	 sc_set_dac (file, 4, 0);
	 	 sc_set_dac (file, 5, 0);
	 	 sc_set_dac (file, 6, 0);
}
