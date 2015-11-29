/*******************************************************
                          cache.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
#include <stdio.h>
using namespace std;

void Cache::Cache_c(int s,int a,int b, int l, Cache* nextLevel )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;
   invalidates=BusRdX=0;
   interventions=0;
   cache2cache=0;
   memtrnsfr=0;
   flush =0;
   somemore=0;
   back_invalidations = 0;
   tag_L1 = 0; 
   evictions = 0; 
   fillLine_calls = 0;
   L1_cache_fills = 0; 

   level      = l;
   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   next_level = nextLevel;
   cout<<"cache contructor pointer "<<next_level<<endl;
  //printf("calling cache contructor \n");
  // printf("assoc: %lu",assoc);
  // printf("  size: %lu",size);
  // printf("  blocksize  %lu",lineSize); 
   //*******************//
   //initialize your counters here//
   //*******************//
 
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
		tagMask <<= 1;
        tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
	     cache[i][j].invalidate();
       cache[i][j].invalidate_st();
      }
   }      
 // printf("class constructor ended"); 
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
int Cache::Access(ulong addr,uchar op)
{   
    int hit=0;
     currentCycle++;/*per cache global counter to maintain LRU order  among cache ways, updated on every cache access*/
       
    
        if(op == 'w') writes++;
	    else          reads++;
    
	
    cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
		if(op == 'w') writeMisses++;
		else  readMisses++;
 
    //the below function is commented as L1 is WTNA. This has to be explitcly called for L1 reads and all L2
   		if(level == 1) 
      {
        if(op == 'r')
        {
	        cacheLine *newline = fillLine(addr);
          newline = newline;
       // newline->setFlags(DIRTY); newline->setStates(MODIFIED_ST);
         }
      }
     if(level == 2)
      {
        cacheLine *newline = fillLine(addr);
          if(op == 'w')  newline->setFlags(DIRTY); 
      }

	  //  line_up=newline;	
	}
	else
	{
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);
        if(line->getFlags() == DIRTY)
        {
            hit=1; 
        }
        else if((line->getFlags() == EXCLUSIVE))
        {
            hit = 1;
        }
        else if((op == 'r')&&(line->getFlags() == VALID))
        {
            hit=1;
        }
		if(op == 'w') 
        {
            line->setFlags(DIRTY);
            line->setStates(MODIFIED_ST);
        }
           
            // line_up=line;
	
    //if(mode=='p') { if(line==NULL)return newline; else return line;}
    //else return line;
   // return line;
    }
    //if((op == 'w') && (level == 1)) hit = 0;
    return hit;


}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	        if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   if((level == 2)&& (next_level != NULL))
   {
     ulong tag_L = cache[i][victim].getTag();
 //    cout<<" L2 tag " <<tag_L<<endl;
     ulong addr2L1 = calcAddr4Tag(tag_L);
     addr2L1 = addr2L1;
     //cout<<hex<<addr2L1<<endl;
    // cout<<"pointer "<<next_level<< "level "<<level<<endl;
     cacheLine *L1_inv = next_level->findLine(addr2L1);
    // L1_inv = L1_inv;
      if(L1_inv != NULL)  
       {
         //cout<<" L1 tag "<<L1_inv->getTag()<<endl;
        // L1_inv->setFlags(INVALID);
         if(L1_inv->isValid()) L1_inv->invalidate();
         next_level->incr_back_invalidations();
       }
    // cout<<"test"<<endl;
   }
   //evictions++;
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
   //if(victim->isValid()) evictions++; 	
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
 //  ulong tag_v = victim->getTag();
 //  ulong addr2L1 = calcAddr4Tag(tag_v);
 //  cout<<hex<<addr2L1<<endl;
 //  if(next_level != NULL)
 //  {
 //   cacheLine *L1_inv = next_level->findLine(addr2L1);
 //   if(L1_inv != NULL)  L1_inv->setFlags(INVALID);
 //   next_level->incr_back_invalidations();
 //  }
   if(victim->getFlags() != INVALID) evictions++; 	
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(VALID);
   victim->setStates(SHARED_ST);
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/
   fillLine_calls++;
   return victim;
}

void Cache::printStats()
{ 
	//printf("===== Simulation results Cache %d  =====\n",0);
    printf("01. number of reads: %lu \n", reads);
    printf("02. number of read misses: %lu \n",readMisses );
    printf("03. number of writes: %lu \n", writes);
    printf("04. number of write misses: %lu \n",writeMisses);
    printf("05. total miss rate: %.2f %% \n",(float)100*(readMisses+writeMisses)/(reads+writes));
    printf("06. number of writebacks: %lu \n", writeBacks);
    printf("07. number of cache-to-cache transfers: %lu \n",cache2cache);
    printf("08. number of memory transactions: %lu \n",readMisses+writeMisses+writeBacks-cache2cache+somemore);
    printf("09. number of interventions: %d \n",interventions);
    printf("10. number of invalidations: %d \n",invalidates);
    printf("11. number of flushes: %lu \n",flush);
    printf("12. number of BusRdX: %d \n",BusRdX);
    printf("13. number of back invalidations: %lu \n",back_invalidations);
    printf("14. number of Evictions: %lu \n",evictions);
    printf("15. number of L1 cache fills: %lu \n",L1_cache_fills);
    printf("16. number of fillLine calls: %lu \n",fillLine_calls);
}


void Cache::BusRdx_MESI( ulong addr, uchar op)
{
    cacheLine *line = findLine(addr);
   if(line!=NULL)
   {
       ulong addr2L1 = calcAddr4Tag(line->getTag());
       if(line->isValid())
       {
           if(line->getFlags() == DIRTY)
           {
                incr_flush();
                writeBack(addr);
           }
          line->invalidate();
          invalidates++;
         }
       cacheLine *L1_inv = next_level->findLine(addr2L1);
       if(L1_inv != NULL)
       {
         //L1_inv->setFlags(INVALID);
        if(L1_inv->isValid())  L1_inv->invalidate();

       }
      next_level->incr_back_invalidations();
    }
}

void Cache::BusRd_MESI( ulong addr, uchar op)
{
    cacheLine *line = findLine(addr);
    if( line !=NULL)
    {
        if(line->getFlags() == DIRTY)
        {
            line->setFlags(VALID);
            interventions++;
            //TODO flush
           incr_flush();
           writeBack(addr);
        }
        if((line->getFlags() == EXCLUSIVE))
        {
            line->setFlags(VALID);
              interventions++;
            //TODO flush opt
         }
        if(line->getFlags() == VALID)
        {
            line->setFlags(VALID);
        }
    }
}





    

void Cache::BusUpgr_MESI (ulong addr, uchar op)
{
    cacheLine *line = findLine(addr);
    if(line != NULL)
    {
        ulong addr2L1 = calcAddr4Tag(line->getTag());
        if(line->isValid())
        {
            line->invalidate();
            invalidates++;
        }
       cacheLine *L1_inv = next_level->findLine(addr2L1);
       if(L1_inv != NULL)
       {
         //L1_inv->setFlags(INVALID);
         L1_inv->invalidate();

       }
         next_level->incr_back_invalidations();
    }
}
/*
int BusCheck(ulong addr)
{ int c=0;
    for(int i=0;i<num_processors;i++)
    {
        if(i!=cpu_id)
        {
            cacheLine line = multi[i].findLine(addr);
            if(line!=NULL) c=1;
        }
    }
    return c;
}
*/



