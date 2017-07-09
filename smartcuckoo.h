#ifndef _SMARTCUCKOO_H_
#define _SMARTCUCKOO_H_

#include "smartcuckoo_config.h"

#include <unistd.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <string>

#include "union_find.h"
#include "MurmurHash3.h"
#include <inttypes.h>
#include <stdint.h>
#include <vector>


using namespace std;

const int capacity = 1 << __SC_POWER;
const uint32_t MAX_SIZE  = 1 << __SC_POWER;
#define TABLE_COUNT 2  
#define BREAK_THRESHOLD 200000
const int INIT_ITEM_COUNT = MAX_SIZE * TABLE_COUNT;

struct Node {
    int app;    
    int occu;   
    int sub;    
} node[TABLE_COUNT][MAX_SIZE];

Data table[TABLE_COUNT][MAX_SIZE];

struct UFSet *pset;             
int sub_group[MAX_SIZE];    
int group_isfull[MAX_SIZE]; 


int subnumber_table[MAX_SIZE];
int groupnumber_table[MAX_SIZE];
int sub_queue_front;
int sub_queue_rear;
int group_queue_front;
int group_queue_rear;

vector<Data> stash;
const int STASH_SIZE = 4;

uint32_t hh[2];

uint32_t seeds[2];

typedef string Key;

uint32_t defaultHash(const Key &k, uint32_t seed) {
    uint32_t h;
    MurmurHash3_x86_32(k.c_str(), k.size(), seed, &h);
    return h;
}

void Hash(const Data &k)
{
	uint32_t mask = MAX_SIZE - 1;
	hh[0] = defaultHash(k, seeds[0]) & mask;
	hh[1] = defaultHash(k, seeds[1]) & mask;
}

void init()
{                 
    for (int i = 0; i < TABLE_COUNT; i++)
    {
        for (int j = 0; j < MAX_SIZE; j++)
        {
            table[i][j] = -1;      
            node[i][j].app = 0; 
            node[i][j].occu = 0;   
            node[i][j].sub = -1;  
        }
    }

    for (int i = 0; i < MAX_SIZE; i++)
    {
        sub_group[i] = -1;
        group_isfull[i] = -2;
    }


    for (int i = 0; i < MAX_SIZE; i++)
    {
        subnumber_table[i] = i;  
        groupnumber_table[i] = i; 
    }
    sub_queue_front = 0;
    sub_queue_rear = MAX_SIZE - 1;
    group_queue_front = 0;
    group_queue_rear = MAX_SIZE - 1;

    pset = newUFSet(MAX_SIZE);

    for (int i = 0; i < 2; i++) {
        seeds[i] = uint32_t(rand());
    }
}

int judge_v_num(int ha, int hb)
{

    if ((node[0][ha].app == 0) && (node[1][hb].app == 0))
    {	
        return 2;
    }
    if ((node[0][ha].app != 0) && (node[1][hb].app != 0))
    {	
        return 0;
    }

    return 1;
}

int subgraph_isfull(int t, int h)
{	
    int subnum = node[t][h].sub;

    int setnum = find(pset, subnum);

    return group_isfull[setnum];
}

void kick_out(const Data &m, int table_num, int ha)
{	
    int i, j;
    Data temp, num;
    int a, b;
    int kick_count;

    kick_count = 0;  

    i = table_num;
    j = ha;
    num = m;


    while (node[i][j].occu != 0)
    {
        kick_count++;
        temp = table[i][j];
        table[i][j] = num;

        Hash(temp);
        a = hh[0];
        b = hh[1];
        num = temp;

        i = !i;
        if (j != a)
        {	//j == b
            j = a;
        }
        else
        {	//j == a
            j = b;
        }
    }

    table[i][j] = num;
    node[i][j].occu = 1;
}

int find_sub_num()
{
    int i;

    i = subnumber_table[sub_queue_front];
    sub_queue_front = (sub_queue_front + 1) % (MAX_SIZE);

    return i;
}

int find_group_num()
{
    int i = groupnumber_table[group_queue_front];
    group_queue_front = (group_queue_front + 1) % (MAX_SIZE);

    return i;
}

void change_subgraph_num(int pre, int cur)
{
    int i;

    for (i = 0; i < MAX_SIZE; i++)
    {
        if (sub_group[i] == pre)
        {
            sub_group[i] = cur;
        }
    }

    groupnumber_table[group_queue_rear] = pre;
    group_queue_rear = (group_queue_rear + 1) % (MAX_SIZE);
}

int search(const Data &m) {
    int ha, hb;

    Hash(m);

    ha = hh[0];
    hb = hh[1];

    if (table[0][ha] == m || table[1][hb] == m) return 1;


    for (int i = 0; i < stash.size(); i++) {
        if (m == stash[i]) {
            return 1;
        }
    }

    return 0;
}


int insert(const Data &m)
{
    int v_num;
    int isfull_one, isfull_two;
    int temp;
    int g, g_a, g_b;
    int t;

    int ha, hb;
    Hash(m);
    ha = hh[0];
    hb = hh[1];

    if (table[0][ha] == m || table[1][hb] == m) {
        // do update here
        return 0;
    }

    for (int i = 0; i < stash.size(); i++) {
        if (m == stash[i]) { 
            // find in stash, update
            return 0;
        }
    }

    v_num = judge_v_num(ha, hb);
    if (v_num == 2)
    {	
        temp = find_sub_num();      

        table[0][ha] = m;
        node[0][ha].app++;          
        node[0][ha].occu = 1;       
        node[0][ha].sub = temp;

        node[1][hb].app++;         
        node[1][hb].sub = temp;

        g = find(pset, temp);
        group_isfull[g] = -1;       
        return 0;
    }
    else if (v_num == 1)
    {	
        if (node[0][ha].app == 0)
        {
            table[0][ha] = m;
            node[0][ha].occu = 1;
            node[0][ha].sub = node[1][hb].sub;       
        }
        else
        {
            table[1][hb] = m;
            node[1][hb].occu = 1;
            node[1][hb].sub = node[0][ha].sub;
        }
        node[0][ha].app++;
        node[1][hb].app++;

        return 1;
    }
    else   
    {
        node[0][ha].app++;
        node[1][hb].app++;

        isfull_one = subgraph_isfull(0, ha);
        isfull_two = subgraph_isfull(1, hb);

        if ((isfull_one == -2) || (isfull_two == -2))
        {
            printf("insert: subgraph_isfull error\n");
            exit(-1);
        }

        if ((isfull_one == 0) && (isfull_two == 0))
        {	
            node[0][ha].app--;
            node[1][hb].app--;

            if (stash.size() < STASH_SIZE) {
                stash.push_back(m);
                return 0;
            } else {
                // return insert failed.
                return 2;
            }

        }

        if ((isfull_one == -1) && (isfull_two == -1))
        {	
            if (connected(pset, node[0][ha].sub, node[1][hb].sub))
            {	
                kick_out(m, 0, ha);
                node[0][ha].occu = 1;

                g = find(pset, node[0][ha].sub);
                group_isfull[g] = 0;
                return 3;
            }
            else
            {	
                kick_out(m, 0, ha);

                int setnum_a = find(pset, node[0][ha].sub);
                int setnum_b = find(pset, node[1][hb].sub);

                merge(pset, node[0][ha].sub, node[1][hb].sub);

                group_isfull[find(pset, node[0][ha].sub)] = -1;
                int another = setnum_a + setnum_b - find(pset, node[0][ha].sub);
                group_isfull[another] = -2;
                return 4;
            }
        }

        if (isfull_one == -1)
        {
            kick_out(m, 0, ha);
        }
        else
        {
            kick_out(m, 1, hb);
        }

        int setnum_a = find(pset, node[0][ha].sub);
        int setnum_b = find(pset, node[1][hb].sub);

        merge(pset, node[0][ha].sub, node[1][hb].sub);

        group_isfull[find(pset, node[0][ha].sub)] = 0;
        int another = setnum_a + setnum_b - find(pset, node[0][ha].sub);
        group_isfull[another] = -2;
        return 5;
    }
}

#endif // _SMARTCUCKOO_H_
