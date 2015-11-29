/*******************************************************
                          cache.h
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>
#include <vector>
typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

/****add new states, based on the protocol****/
enum{
	INVALID = 0,
	VALID,
	DIRTY,
    EXCLUSIVE,
    Sm,
    Sc
};

enum {
    INVALID_ST=0,
    SHARED_ST,
    MODIFIED_ST,
    EXCLUSIVE_ST
};

class cacheLine 
{
protected:
   ulong tag;
   ulong Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong States;  //various states in protocol
   ulong seq; 
 
public:
   cacheLine()            { tag = 0; Flags = 0; States = 0; } //state initialisation was added
   ulong getTag()         { return tag; }
   ulong getFlags()			{ return Flags;}
   ulong getSeq()         { return seq; }
   void setSeq(ulong Seq)			{ seq = Seq;}
   void setFlags(ulong flags)			{  Flags = flags;}
   void setTag(ulong a)   { tag = a; }
   void invalidate()      { tag = 0; Flags = INVALID; }//useful function
   bool isValid()         { return ((Flags) != INVALID); }

   //newly added fuctions
   ulong getStates()      { return States;}
   void setStates(ulong states) { States = states;}
   void invalidate_st()  { States = INVALID_ST;}

};

class Cache
{
protected:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   int level;
   //******///
   //add coherence counters here///
   //******///

   cacheLine **cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)  { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag)   { return (tag << (log2Blk));}
   
public:
    ulong reads,readMisses,writes,writeMisses,writeBacks;
    ulong L1_cache_fills;
    ulong currentCycle;  
    unsigned int BusRdX;
    unsigned int invalidates; 
    unsigned int interventions;
    ulong cache2cache;
    ulong memtrnsfr;
    ulong flush;
    ulong somemore;
    ulong back_invalidations;
    ulong tag_L1;
    Cache* next_level;
    ulong evictions;

    Cache(){};
   
    void  Cache_c(int,int,int,int, Cache* );
 //   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM(){return readMisses;} ulong getWM(){return writeMisses;} 
   ulong getReads(){return reads;}ulong getWrites(){return writes;}
   ulong getWB(){return writeBacks;}
   
   void writeBack(ulong)   {writeBacks++;}
   int Access(ulong,uchar);
   void printStats();
   void updateLRU(cacheLine *);
   void incr_back_invalidations(){  back_invalidations++;}
   //functions for MSI
   void BusRdX_MSI(ulong addr,uchar op);
   void BusRd_MSI(ulong addr,uchar op);
   int PrWrRd_MSI(ulong addr,uchar op);
   
   void incr_BusRdX(){  BusRdX++;   };
   void incr_flush(){ flush++; }
   //functions for MESI
   void BusRdx_MESI(ulong addr, uchar op);
   void BusRd_MESI(ulong addr, uchar op);
   void BusUpgr_MESI(ulong addr, uchar op);
   
   //functions for dragon
   void BusRd_dragon(ulong addr, uchar op);
   void BusUpd_dragon(ulong addr, uchar op);

   
   //******///
   //add other functions to handle bus transactions///
   //******///

};

/*class CPU_cache 
{
    public:
        std:: vector<Cache> cpu_cache;
    int size,assoc,block,cpu_num;
    CPU_cache(int s , int a , int b,int num)//:cpu_cache(num,Cache(s,a,b))
    {
        cpu_cache.resize(num,Cache(s, a , b));
        std::cout<<"calling cpu cache contructor"<<std::endl;
        size=s;
        assoc=a;
        block=b;
        cpu_num=num;
    }
};
*/
#endif
