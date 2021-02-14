// linux_dig_ifc_test

// Test program for digital interface

// 25-Mar DFN New original version

#include "dig_ifc_test.h"
#include "string.h"


volatile u32 *dig_ifc_regs;


int main()
{
  int i, status;
  int xfr_done;
  int read0, read1;
  int write0, write1;
  int writec, readc;
  int expected0, expected1;
  int dig_ifc_id;
  int nerrors = 0;
  bool OK, ch0, ch1, ch2, ch3, state0[4], state1[4], state2[4], state3[4];

  // Map registers
    //printf("Calling map_dig_ifc\n");
    map_dig_ifc();
    //printf("Return from map_dig_ifc\n");
  sleep(1);

 // Check for sane IDs
  dig_ifc_id = read_dig_ifc(DIG_IFC_ID_ADDR);
  //printf("dig_ifc_test: dig_ifc_id=%x\n", dig_ifc_id);
  sleep(5);

    // Set which bits are output
//printf("bit 0-4-5-7 are in input (from DIG-INT), bit 1-2-3-6 are in output (to DIG-INT)\n");
    writec = CTL_SET0 | (CTL_SET0 <<8);

    status = write_dig_ifc(DIG_IFC_CONTROL_ADDR,writec);
    sleep(3);

   readc = read_dig_ifc(DIG_IFC_CONTROL_ADDR);

    // Set which bits are 1
 //  printf("Put 1 in  1-2-3-6\n");
    write1 = CTL_SET0 | (CTL_SET0 <<8);
    status = write_dig_ifc(DIG_IFC_OUTPUT_ADDR,write1);
    read1 = read_dig_ifc(DIG_IFC_OUTPUT_ADDR);

    // Read values that are supposed to be looped back
    read0 = read_dig_ifc(DIG_IFC_INPUT_ADDR);

        OK = ( ((read0 >> 0) & 1) == ((write1 >> 1) & 1)
           && ((read0 >> 4) & 1) == ((write1 >> 2) & 1)
           && ((read0 >> 5) & 1) == ((write1 >> 3) & 1)
           && ((read0 >> 7) & 1) == ((write1 >> 6) & 1));


        if (((read0 >> 0) & 1) == ((write1 >> 1) & 1)) state0[0]=true;
        		else state0[0]=false;

        if (((read0 >> 4) & 1) == ((write1 >> 2) & 1)) state1[0]=true;
                		else state1[0]=false;

        if (((read0 >> 5) & 1) == ((write1 >> 3) & 1)) state2[0]=true;
                		else state2[0]=false;

        if (((read0 >> 7) & 1) == ((write1 >> 6) & 1)) state3[0]=true;
                		else state3[0]=false;

/*
        printf("read bit 0 = %d - write bit 1= %d\n", ((read0 >> 0) & 1) ,((write1 >> 1) & 1));
        printf("read bit 4 = %d - write bit 2= %d\n", ((read0 >> 4) & 1) ,((write1 >> 2) & 1));

        printf("read bit 5 = %d - write bit 3= %d\n", ((read0 >> 5) & 1) ,((write1 >> 3) & 1));
        printf("read bit 7 = %d - write bit 6= %d\n", ((read0 >> 7) & 1) ,((write1 >> 6) & 1));
*/
    // Now send all 0
   // printf("Now send all 0\n");
    write1 = 0;
    status = write_dig_ifc(DIG_IFC_OUTPUT_ADDR,write1);
    read1 = read_dig_ifc(DIG_IFC_OUTPUT_ADDR);

    // Read values that are supposed to be looped back
    read0 = read_dig_ifc(DIG_IFC_INPUT_ADDR);

    OK = ( ((read0 >> 0) & 1) == ((write1 >> 1) & 1)
       && ((read0 >> 4) & 1) == ((write1 >> 2) & 1)
       && ((read0 >> 5) & 1) == ((write1 >> 3) & 1)
       && ((read0 >> 7) & 1) == ((write1 >> 6) & 1));

          if (((read0 >> 0) & 1) == ((write1 >> 1) & 1)) state0[1]=true;
          		else state0[1]=false;

          if (((read0 >> 4) & 1) == ((write1 >> 2) & 1)) state1[1]=true;
                  		else state1[1]=false;

          if (((read0 >> 5) & 1) == ((write1 >> 3) & 1)) state2[1]=true;
                  		else state2[1]=false;

          if (((read0 >> 7) & 1) == ((write1 >> 6) & 1)) state3[1]=true;
                  		else state3[1]=false;
/*
          printf("read bit 0 = %d - write bit 1= %d\n", ((read0 >> 0) & 1) ,((write1 >> 1) & 1));
          printf("read bit 4 = %d - write bit 2= %d\n", ((read0 >> 4) & 1) ,((write1 >> 2) & 1));

          printf("read bit 5 = %d - write bit 3= %d\n", ((read0 >> 5) & 1) ,((write1 >> 3) & 1));
          printf("read bit 7 = %d - write bit 6= %d\n", ((read0 >> 7) & 1) ,((write1 >> 6) & 1));
*/
   // printf("bit 0-4-5-7 are in output 1-2-3-6 are in input\n");
    // Set which bits are output -- flip order from above
    writec = CTL_SET1 | (CTL_SET1 <<8);
    status = write_dig_ifc(DIG_IFC_CONTROL_ADDR,writec);
    readc = read_dig_ifc(DIG_IFC_CONTROL_ADDR);

    // Set which bits are 1
    write1 = CTL_SET1 | (CTL_SET1 << 8);
    status = write_dig_ifc(DIG_IFC_OUTPUT_ADDR,write1);
    read1 = read_dig_ifc(DIG_IFC_OUTPUT_ADDR);

    // Read values that are supposed to be looped back
    read0 = read_dig_ifc(DIG_IFC_INPUT_ADDR);

    OK = ( ((read0 >> 1) & 1) == ((write1 >> 0) & 1)
               && ((read0 >> 2) & 1) == ((write1 >> 4) & 1)
               && ((read0 >> 3) & 1) == ((write1 >> 5) & 1)
               && ((read0 >> 6) & 1) == ((write1 >> 7) & 1));


          if (((read0 >> 1) & 1) == ((write1 >> 0) & 1)) state0[2]=true;
    		else state0[2]=false;

          if (((read0 >> 2) & 1) == ((write1 >> 4) & 1)) state1[2]=true;
    		else state1[2]=false;

          if (((read0 >> 3) & 1) == ((write1 >> 5) & 1)) state2[2]=true;
    		else state2[2]=false;

          if (((read0 >> 6) & 1) == ((write1 >> 7) & 1)) state3[2]=true;
    		else state3[2]=false;

/*
          printf("read bit 1 = %d - write bit 0= %d\n", ((read0 >> 1) & 1), ((write1 >> 0) & 1));
          printf("read bit 2 = %d - write bit 4= %d\n", ((read0 >> 2) & 1) ,((write1 >> 4) & 1));
          printf("read bit 3 = %d - write bit 5= %d\n", ((read0 >> 3) & 1), ((write1 >> 5) & 1));
          printf("read bit 6 = %d - write bit 7= %d\n", ((read0 >> 6) & 1) ,((write1 >> 7) & 1));

*/
    //printf("Now send all 0\n");
    write1 = 0;
    status = write_dig_ifc(DIG_IFC_OUTPUT_ADDR,write1);
    read1 = read_dig_ifc(DIG_IFC_OUTPUT_ADDR);

    // Read values that are supposed to be looped back
    read0 = read_dig_ifc(DIG_IFC_INPUT_ADDR);

    OK = ( ((read0 >> 1) & 1) == ((write1 >> 0) & 1)
                 && ((read0 >> 2) & 1) == ((write1 >> 4) & 1)
                 && ((read0 >> 3) & 1) == ((write1 >> 5) & 1)
                 && ((read0 >> 6) & 1) == ((write1 >> 7) & 1));


             if (((read0 >> 1) & 1) == ((write1 >> 0) & 1)) state0[3]=true;
       		else state0[3]=false;

             if (((read0 >> 2) & 1) == ((write1 >> 4) & 1)) state1[3]=true;
       		else state1[3]=false;

             if (((read0 >> 3) & 1) == ((write1 >> 5) & 1)) state2[3]=true;
       		else state2[3]=false;

             if (((read0 >> 6) & 1) == ((write1 >> 7) & 1)) state3[3]=true;
       		else state3[3]=false;
/*
             printf("read bit 1 = %d - write bit 0= %d\n", ((read0 >> 1) & 1), ((write1 >> 0) & 1));
             printf("read bit 2 = %d - write bit 4= %d\n", ((read0 >> 2) & 1) ,((write1 >> 4) & 1));
             printf("read bit 3 = %d - write bit 5= %d\n", ((read0 >> 3) & 1), ((write1 >> 5) & 1));
             printf("read bit 6 = %d - write bit 7= %d\n", ((read0 >> 6) & 1) ,((write1 >> 7) & 1));
*/
             ch0=state0[0] && state0[1] && state0[2] && state0[3];
             ch1=state1[0] && state1[1] && state1[2] && state1[3];
             ch2=state2[0] && state2[1] && state2[2] && state2[3];
             ch3=state3[0] && state3[1] && state3[2] && state3[3];

             //JSON

        	 printf("{");

        	 printf ("\"ch0\":%d,",ch0);// aggiunta correzione !!!!!!!!!!!!!!
        	 printf ("\"ch1\":%d,",ch1);// aggiunta correzione !!!!!!!!!!!!!!
        	 printf ("\"ch2\":%d,",ch2);// aggiunta correzione !!!!!!!!!!!!!!
        	 printf ("\"ch3\":%d",ch3);// aggiunta correzione !!!!!!!!!!!!!!

        	 printf("}");

}
