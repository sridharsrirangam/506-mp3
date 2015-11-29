/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <stdio.h>
#include <vector>
//#include <iostream>
using namespace std;

#include "cache.h"

int check_caches(ulong addr);

int main(int argc, char *argv[])
{
	
	ifstream fin;
	FILE * pFile;
    int cpu_no;
    char op;
    char addr_str[10];
    ulong addr;


	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

    /*****uncomment the next five lines*****/
	int L1_cache_size = atoi(argv[1]);
	int L2_cache_size = atoi(argv[2]);
	int L1_cache_assoc= atoi(argv[3]);
	int L2_cache_assoc= atoi(argv[4]);
	int blk_size   = atoi(argv[5]);
//	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
//	int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/
	char *fname =  (char *)malloc(20);
 	fname = argv[6];


	
	//****************************************************//
	//**printf("===== Simulator configuration =====\n");**//
	//*******print out simulator configuration here*******//
	//****************************************************//
    printf("===== 506 Personal information ===== \n");
    printf("Narasimha Sridhar Srirangam \n");
    printf("200084499 \n");
    printf("ECE492 Students? NO \n");
    printf("===== 506 SMP Simulator configuration ===== \n");
    printf("L1_SIZE:  %d \n",L1_cache_size);
    printf("L1_ASSOC: %d \n",L1_cache_assoc);
    printf("L2_SIZE:  %d \n",L2_cache_size);
    printf("L2_ASSOC: %d \n",L2_cache_assoc);
    printf("BLOCKSIZE: %d \n",blk_size);
    printf("TRACE FILE: %s \n ",fname);

 
	//*********************************************//
        //*****create an array of caches here**********//
	//*********************************************//	
    //L1 caches
    Cache *L1;
    L1 = new Cache[4];
    for(int i=0;i<4;i++)
    {
      L1[i].Cache_c(L1_cache_size, L1_cache_assoc,blk_size, 1, NULL);
      cout<<" Cache "<<i<<" pointer "<<&L1[i]<<endl;
    }
    Cache *multi;
    multi= new Cache[4];
    for(int i=0;i<4;i++)
    {
        multi[i].Cache_c(L2_cache_size,L2_cache_assoc,blk_size, 2, &L1[i]);
    }
	//CPU_cache multi_cache(cache_size,cache_assoc,blk_size,cpu_no);
    //std:: vector<Cache> multi(num_processors,Cache(cache_size,cache_assoc,blk_size));

    pFile = fopen (fname,"r");
	if(pFile == 0)
	{   

	
        printf("Trace file problem\n");
		exit(0);
	}
    while(fscanf(pFile," %d %c %s",&cpu_no,&op,addr_str))
    {
    if(feof(pFile)) break;
    addr=strtoul(addr_str,NULL,16);
   // printf(" cpu number is %d ",cpu_no);
   // printf(" operation is %c ",op);
   // printf(" address is %s ",addr_str);
   // printf("address in numbers is %#lx",addr);
   // printf("\n");
    //if(cpu_no==1)
    
   
       // printf("\n size %d \n",(int)multi.size());
       // multi_cache.call_cache(addr,op);
       
       // multi[0].Access(addr,op);
     
  //  cpu_no=cpu_no;
  // int hit = multi[cpu_no].PrWrRd_MSI(addr,op);
   
  // printf("cpu number %d \n ",cpu_no); 
 // int hit = multi[cpu_no].Access(addr,op);
   // printf("called pr read for processpr %d \n",cpu_no);
    

    cacheLine *L1_ptr = L1[cpu_no].findLine(addr);
    int L1_hit_d = L1[cpu_no].Access(addr,op);
    L1_hit_d = L1_hit_d;
    if((L1_ptr != NULL )&&( op == 'w'))
    {
     // cacheLine *line = multi[cpu_no].findLine(addr);
     // if(line != NULL)
     // {
     //    if(line->getFlags() == VALID)
     //    {
     //      for( int i=0;i<4;i++)
     //      {
     //        if(i != cpu_no)
     //         {
     //             multi[i].BusUpgr_MESI(addr, op);
     //         }
     //      }
     //      line->setFlags(DIRTY);
     //   }
     //   if(line->getFlags() == EXCLUSIVE)  line->setFlags(DIRTY);
     // multi[cpu_no].updateLRU(line);
       // multi[cpu_no].writes++;
    //  }
       //int L2_hit =  multi[cpu_no].Access(addr,op);
       //L2_hit = L2_hit;
      }

    if((L1_ptr == NULL)||(op == 'w'))
    {
      if(op == 'r')L1[cpu_no].L1_cache_fills++;
      int c = 0;
      int s_e = 0;
      cacheLine *line = multi[cpu_no].findLine(addr);
      if(line != NULL)
      {
        /* if(line->getFlags()==VALID) s_e=1;
         else if (line->getFlags()==EXCLUSIVE) s_e=2;
         else if (line->getFlags()==DIRTY) s_e=3;
         */
          s_e = line->getFlags();
      }

      for(int i=0; i<4;i++)
      {  
          if(i!=cpu_no)
          {
              cacheLine *line = multi[i].findLine(addr);
              if( line !=NULL) c=1;
          }
      }
      if((line==NULL)&&(c!=0)) multi[cpu_no].cache2cache++;
      int hit  = multi[cpu_no].Access(addr,op);
      cacheLine *linec = multi[cpu_no].findLine(addr);
      if( (op == 'r') && (c==0) && (linec->getFlags() ==VALID)&&(line == NULL))
      {
         // printf(" set to exclusize, was %lu before ",linec->getFlags());
          linec->setFlags(EXCLUSIVE);
         // printf(" after exclusive valie is %lu \n ", linec->getFlags());
      }
      if(hit == 0)
      {
         // printf("calline MESI functions");
          if(s_e == VALID)
          {
              if( op == 'w')
              {
                  for( int i=0;i<4;i++)
                  {
                      if(i != cpu_no)
                      {
                          multi[i].BusUpgr_MESI(addr, op);
                      }
                  }
              
              }
          }
          else if(s_e == INVALID )
          {
              if(op=='w') multi[cpu_no].incr_BusRdX();

            for(int i=0; i<4;i++)
            {
              if(i!=cpu_no)
               {
                   if( op =='r') multi[i].BusRd_MESI(addr, op);
                   else if (op == 'w') { multi[i].BusRdx_MESI(addr, op);}
                      }
                  }
              }
          

      }
    } 




 
    }//end of while
    



   
	///******************************************************************//
	//**read trace file,line by line,each(processor#,operation,address)**//
	//*****propagate each request down through memory hierarchy**********//
	//*****by calling cachesArray[processor#]->Access(...)***************//
	///******************************************************************//
	
    fclose(pFile);

	//********************************//
	//print out all caches' statistics //
	//********************************//
    for(int i=0;i<4;i++)
    {
        printf("============ Simulation results  L1 Cache (Processor %d) ============ \n",i);
        L1[i].printStats();	
        printf("============ Simulation results  L2 Cache (Processor %d) ============ \n",i);
        multi[i].printStats();	
    }
}

