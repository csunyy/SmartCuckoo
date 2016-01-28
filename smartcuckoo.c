/**
*2016.1.16
*smartcuckoo
**/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define MAX_SIZE 100000
#define TABLE_COUNT 2
#define SIMU_TIMES 1
#define TRAD_KICK_THRES 100
#define BREAK_THRESHOLD 200000
#define TRADITION_BREAK_THRESHOLD 200000
#define INIT_ITEM_COUNT 200000

#define min(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)

//#define _DEBUG

struct Node{
    int app;    //the appear bit
    int occu;   //the occupancy bit
    int sub;    //the subgraph number
}node[TABLE_COUNT][MAX_SIZE];

typedef struct {
    int a;
    int b;
}Pos;

typedef unsigned long long Data;



Data table[TABLE_COUNT][MAX_SIZE];
Data trad_table[TABLE_COUNT][MAX_SIZE];

int sub_group[MAX_SIZE/2];
int group_isfull[MAX_SIZE/2];

int insert_count, trad_insert_count;
int insert_count_global_1;
int insert_count_global_2;
int temp_table[INIT_ITEM_COUNT];
int temp_count;
int trad_temp_table[INIT_ITEM_COUNT];
int trad_temp_count;

int dele_table[INIT_ITEM_COUNT];
int dele_count;


int subnumber_table[MAX_SIZE/2];

int groupnumber_table[MAX_SIZE/2];
int sub_queue_front;
int sub_queue_rear;
int group_queue_front;
int group_queue_rear;

int count[6];
int count_global[6];
int hh[2];

float time1;
float time2;
float time3;

float trad1;
float trad2;
float trad3;

struct timeval u_start, u_end;
float u_time;

void Hash(Data k)
{
    int i, j;

    j = (int)(MAX_SIZE * k);
    srand(j);
    hh[0] = (int)(MAX_SIZE * 1.0 * rand() / (RAND_MAX + 1.0));
	hh[1] = k * hh[0] % MAX_SIZE;
}

void init()
{
    int i, j;
    for(i = 0; i < TABLE_COUNT; i++)
    {
		for(j = 0; j < MAX_SIZE; j++)
		{
	    	table[i][j] = -1;
	    	node[i][j].app = 0;
	    	node[i][j].occu = 0;
	    	node[i][j].sub = -1;
		}
    }

    for(i = 0; i < MAX_SIZE / 2; i++)
    {
		sub_group[i] = -1;
		group_isfull[i] = -2;
    }

    insert_count = 0;
    for(i = 0; i < 6; i++)
	count[i] = 0;
    for(i = 0; i < MAX_SIZE / 2; i++)
    {
		subnumber_table[i] = i;
		groupnumber_table[i] = i;
    }
    sub_queue_front = 0;
    sub_queue_rear = MAX_SIZE / 2 - 1;
    group_queue_front = 0;
    group_queue_rear = MAX_SIZE / 2 - 1;

    for(i = 0; i < INIT_ITEM_COUNT; i++)
    {
		temp_table[i] = 0;
    }
    temp_count = 0;
    
    for(i = 0; i < TABLE_COUNT; i++)
    {
		for(j = 0; j < MAX_SIZE; j++)
		{
	    	trad_table[i][j] = -1;
		}
    }
    trad_temp_count = 0;
    for(i = 0; i < INIT_ITEM_COUNT; i++)
    {
		trad_temp_table[i] = 0;
    }

}

int judge_v_num(int ha, int hb)
{
    int i, j;
    
    if((node[0][ha].app == 0) && (node[1][hb].app == 0))
    {//v+2
		return 2;
    }
    if((node[0][ha].app != 0) && (node[1][hb].app != 0))
    {//v+0
		return 0;
    }
    //v+1
    return 1;
}

int subgraph_isfull(int t, int h)
{
    int i, j; 
    int subnum;
    int groupnum;

    subnum = node[t][h].sub;
    groupnum = sub_group[subnum];

    if(group_isfull[groupnum] == -1)
    {
		return -1;
    }

    if(group_isfull[groupnum] == 0)
    {
		return 0;
    }

    //error
    return -2;
}

void kick_out(Data m, int table_num, int ha)
{
    int i, j;
    Data temp, num;
    int a, b;
    int kick_count;

    kick_count = 0;

    i = table_num;
    j = ha;
    num = m;

    while(node[i][j].occu != 0)
    {
		kick_count++;
		temp = table[i][j];
		table[i][j] = num;

		Hash(temp);
		a = hh[0];
		b = hh[1];
		num = temp;

		i = !i; 
		if(j != a)
		{//j == b
	    	j = a;
		}
		else
		{ //j == a
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
    sub_queue_front = (sub_queue_front + 1) % (MAX_SIZE / 2);
    
    return i;
}

int find_group_num()
{
    int i;

    i = groupnumber_table[group_queue_front];
    group_queue_front = (group_queue_front + 1) % (MAX_SIZE / 2);
    
    return i;
}

void change_subgraph_num(int pre, int cur)
{
    int i;

    for(i = 0; i < MAX_SIZE / 2; i++)
    {
		if(sub_group[i] == pre)
		{
	    	sub_group[i] = cur;
		}
    }

    groupnumber_table[group_queue_rear] = pre;
    group_queue_rear = (group_queue_rear + 1) % (MAX_SIZE / 2);
}

int insert(Data m, int ha, int hb)
{
    int v_num;
    int isfull_one, isfull_two;
    int temp;
    int g, g_a, g_b;
    int t;

    v_num = judge_v_num(ha, hb);
    if(v_num == 2)
    {
		temp = find_sub_num();      

		table[0][ha] = m;
		node[0][ha].app++;          
		node[0][ha].occu = 1;       
        node[0][ha].sub = temp;

		node[1][hb].app++;          
        node[1][hb].sub = temp;

		g = find_group_num();       
		sub_group[temp] = g;
		group_isfull[g] = -1;       
		count[4]++;
		return 0;
    }
    else if(v_num == 1)
    {//v+1
		if(node[0][ha].app == 0)
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

		count[5]++;
		return 1;
    }
    else   //v_num == 0
    {
		node[0][ha].app++;
		node[1][hb].app++;

		isfull_one = subgraph_isfull(0, ha);
		isfull_two = subgraph_isfull(1, hb);

		if((isfull_one == -2) || (isfull_two == -2))
		{
	    	printf("insert: subgraph_isfull error\n");
	    	exit(-1);
		}

		if((isfull_one == 0) && (isfull_two == 0))
		{
	    	node[0][ha].app--;
	    	node[1][hb].app--;
	    	count[0]++;

	    	return 2;
		}
	
		if((isfull_one == -1) && (isfull_two == -1))
		{
	    	if(sub_group[node[0][ha].sub] == \
		          sub_group[node[1][hb].sub] )
	    	{
				kick_out(m, 0, ha);
				node[0][ha].occu = 1;

		
				g = sub_group[node[0][ha].sub];
				group_isfull[g] = 0;

				count[2]++;
				return 3;
	    	}
	    	else
	    	{
	     
		kick_out(m, 0, ha);
		
		gettimeofday(&u_start, NULL);
		
		g_a = min( sub_group[node[0][ha].sub], sub_group[node[1][hb].sub]);
                g_b = max( sub_group[node[0][ha].sub], sub_group[node[1][hb].sub]);
		change_subgraph_num(g_b, g_a);   
		gettimeofday(&u_end, NULL);
		u_time = (u_end.tv_sec - u_start.tv_sec) * 1000000 + \
		    (u_end.tv_usec - u_start.tv_usec);	

		group_isfull[g_a] = -1;     
		group_isfull[g_b] = -2;     
		count[1]++;
		return 4;
	    }
	}
	
        if(isfull_one == -1)
	{
	    kick_out(m, 0, ha);
	}
	else
	{
	    kick_out(m, 1, hb);
	}

	gettimeofday(&u_start, NULL);
	
	g_a = min( sub_group[node[0][ha].sub], sub_group[node[1][hb].sub]);
        g_b = max( sub_group[node[0][ha].sub], sub_group[node[1][hb].sub]);
	change_subgraph_num(g_b, g_a);
	gettimeofday(&u_end, NULL);
	u_time = (u_end.tv_sec - u_start.tv_sec) * 1000000 + \
		    (u_end.tv_usec - u_start.tv_usec);	

	group_isfull[g_a] = 0;             
	group_isfull[g_b] = -2;
	count[3]++;

	return 5;
    }

}

void print_ave_result(float sum_utili_1, float runtime_1, float u_time_total)
{
    int i;
    printf("\n**********************psedudoforest + cuckoo****************************\n");
        
    printf("The algorithm of pseudoforest + cuckoo path: \n");
    printf("The simulation of Insert is starting...\n");
    printf("The length of each hash table: %d, hash table count: %d\n", MAX_SIZE, TABLE_COUNT);
    printf("The simulation times: %d\n", SIMU_TIMES);
    printf("The BREAK_THRESHOLD: %d\n", BREAK_THRESHOLD);
    printf("\nThe Statistics Results:\n");
    printf("Adding two hash location(v+2): %.2f %\n", 100*count_global[4]/SIMU_TIMES/(2.0*MAX_SIZE));
    printf("Adding one hash location(v+1): %.2f %\n", 100*count_global[5]/SIMU_TIMES/(2.0*MAX_SIZE));
    printf("Hash location without adding(v+0)(including four sub-case):\n");
    printf("\tnot full, not full(NOT in same subgraph): %.2f %\n", 100*count_global[1]/SIMU_TIMES/(2.0*MAX_SIZE));
    printf("\tnot full, not full(in same subgraph): %.2f %\n", 100*count_global[2]/SIMU_TIMES/(2.0*MAX_SIZE));
    printf("\tfull, not full: %.2f %\n", 100*count_global[3]/SIMU_TIMES/(2.0*MAX_SIZE));
    printf("\tfull, full: %.2f %\n", 100*count_global[0]/SIMU_TIMES/(2.0*MAX_SIZE));

    printf("Running time (v+1):            %f us\n", time1 / SIMU_TIMES);
    printf("Running time (full, not full): %f us\n", time2 / SIMU_TIMES);
    printf("Running time (full, full):     %f us\n", time3 / SIMU_TIMES);

    printf("The insert succeed before (full,full): %d\n", insert_count_global_1/SIMU_TIMES);
    printf("Utilization ratio:   %.10f%\n", sum_utili_1 / SIMU_TIMES);
	printf("u time:   %f us\n", u_time_total / SIMU_TIMES);
    printf("Running time:   %f us\n", runtime_1 / SIMU_TIMES);
    printf("*********************************************************************\n\n");   
}

int traditional_insert(Data m, Data *pnum)
{
    int i, j;
    Data temp;
    int ha, hb;
    Data num;
    int kick_count; 
    int t_0, t_1;

    kick_count = 0;
    Hash(m);
    ha = hh[0];
    hb = hh[1];
	
	
    if(trad_table[0][ha] == -1)
    {
		 
		trad_table[0][ha] = m;
		return 0;
    }
	else if(trad_table[1][hb] == -1)
	{
		trad_table[1][hb] = m;
		return 0;
	}
    else
    {
		
		
		srand((int)time(0));
		i = rand() % TABLE_COUNT;
		if(i)
		{
			j = hb;
		}
		else
		{
			j=ha;
		}
		num = m;

		while(kick_count <= TRAD_KICK_THRES)
		{
			temp = trad_table[i][j];
			trad_table[i][j] = num;

			Hash(temp);
			ha = hh[0];
			hb = hh[1];

			num = temp;

			i = !i;
			if(j != ha)
			{//j == hb
			j = ha;
			}
			else
			{//j == ha
			j = hb;
			}

			kick_count++;

			if(trad_table[i][j] == -1)
			{
			trad_table[i][j] = num;
			return 0;
			}
		}

		*pnum = num;
		return -1;
    }

    return -1;
}

void print_tradition(float sum_utili_2, float runtime_2)
{
    printf("\n************************traditional cuckoo****************************\n");
    printf("The algorithm of traditional cuckoo: \n");
    printf("The simulation of Insert is starting...\n");
    printf("The length of each hash table: %d, hash table count: %d\n", MAX_SIZE, TABLE_COUNT);
    printf("The simulation times: %d\n", SIMU_TIMES);
    printf("The TRAD_KICK_THRES: %d\n", TRAD_KICK_THRES);
    printf("The TRADITION_BREAK_THRESHOLD: %d\n", TRADITION_BREAK_THRESHOLD);
    printf("\nThe Statistics Results: \n");
    
    printf("Traditional time (v+1):            %f us\n", trad1 / SIMU_TIMES);
    printf("Traditional time (full, not full): %f us\n", trad2 / SIMU_TIMES);
    printf("Traditional time (full, full):     %f us\n", trad3 / SIMU_TIMES);
    
    printf("The insert succeed: %d\n", insert_count_global_2/SIMU_TIMES);
    printf("Utilization ratio:   %.10f %\n", sum_utili_2 / SIMU_TIMES);
    printf("Running time:   %f us\n", runtime_2 / SIMU_TIMES);
    printf("*********************************************************************\n\n");
}

int main(int argc, char **argv)
{
    int i, j, k;
    int ha, hb;
    Data m;
    int ret, trad_ret;
    Data data[INIT_ITEM_COUNT];
    int c, d;
    int init_data_range;
    float sum_utili_1, sum_utili_2;
    int break_num, trad_break_num;
    Data last_num;
    float runtime_1, runtime_2, u_time_total;      
    struct timeval p_begin, p_end, t_begin, t_end;
    int lambda;
    int success_count;
	FILE *fp;
	int n;
	printf("|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
  
    sum_utili_1 = 0.0;
    insert_count_global_1 = 0;
    runtime_1 = 0;
	u_time_total = 0;
    for(i = 0; i < 6; i++)
    {
	count_global[i] = 0;
    }

    sum_utili_2 = 0.0;
    insert_count_global_2 = 0;
    runtime_2 = 0;
    
    time1 = time2 = time3 = 0;
    trad1 = trad2 = trad3 = 0;
    u_time = 0;
	if(argv[1]==NULL)
	{
		printf("\nerror on open argv[1]!");
		return -1;
	}
	if((fp=fopen(argv[1],"r"))==NULL)
	{
		printf("\nerror on open the file!");
		return -1;
	}
	fscanf(fp,"%d\n\n",&n);
	if(n < INIT_ITEM_COUNT)
		return -1;
	for(i = 0; i < INIT_ITEM_COUNT; i++)
	{
		fscanf(fp, "%lld", &data[i]);
	}
	fclose(fp);
    for(k = 1; k <= SIMU_TIMES; k++)
    {
        init();
		m = -1;
		break_num = 0;
		trad_break_num = 0;
		trad_insert_count= 0;

		for(i = 0; i < INIT_ITEM_COUNT; i++)
		{
//**********************************  pseudoforest  *****************************************
			m = data[i];
			
			Hash(m);
			ha = hh[0];
			hb = hh[1];
			
			gettimeofday(&p_begin, NULL);    
			ret = insert(m, ha, hb);
			gettimeofday(&p_end, NULL);
	    
			if(ret == 2)
			{
			
					
				temp_table[temp_count] = m;
				temp_count++;
			 
			time3 += (p_end.tv_sec - p_begin.tv_sec) * 1000000 + \
						(p_end.tv_usec - p_begin.tv_usec);
			
			break_num++;
			if(break_num == BREAK_THRESHOLD)
			{
				break;
			}
			}
			else
			{   
			insert_count++;
			if(ret == 1)
			{
				time1 += (p_end.tv_sec - p_begin.tv_sec) * 1000000 + \
						(p_end.tv_usec - p_begin.tv_usec);
			}else if(ret == 5)
			{
				time2 += (p_end.tv_sec - p_begin.tv_sec) * 1000000 + \
						(p_end.tv_usec - p_begin.tv_usec);
				time2 -= u_time;
				u_time_total += u_time;
				
			}else if(ret == 4)
			{
				u_time_total += u_time;
				
			}
			}
			
			runtime_1 += (p_end.tv_sec - p_begin.tv_sec) * 1000000 + \
		    		(p_end.tv_usec - p_begin.tv_usec);
	    

//********************************   tradition  ********************************************
			last_num = -1;
			
			gettimeofday(&t_begin, NULL);
			trad_ret = traditional_insert(data[i], &last_num);
			gettimeofday(&t_end, NULL);
			if(ret == 2)
			{
			trad2 += (t_end.tv_sec - t_begin.tv_sec) * 1000000 + \
						(t_end.tv_usec - t_begin.tv_usec);
			}else if(ret == 1)
			{
			trad1 += (t_end.tv_sec - t_begin.tv_sec) * 1000000 + \
						(t_end.tv_usec - t_begin.tv_usec);
			}else if(ret == 5)
			{
			trad3 += (t_end.tv_sec - t_begin.tv_sec) * 1000000 + \
						(t_end.tv_usec - t_begin.tv_usec);
			}
			runtime_2 += (t_end.tv_sec - t_begin.tv_sec) * 1000000 + \
				(t_end.tv_usec - t_begin.tv_usec);    
			
			if(trad_ret == -1)
			{
			trad_temp_table[trad_temp_count] = last_num;
			trad_temp_count++;

			
			trad_break_num++;
			if(trad_break_num == TRADITION_BREAK_THRESHOLD)
			{
				break;
			}
			}
			else
			{
			trad_insert_count++;
			}
		}
//******************************************************************************************

	insert_count_global_1 += insert_count;
	for(j = 0; j < 6; j++)
	{
	    count_global[j] += count[j];
	}
	sum_utili_1 += (insert_count/ (2.0 * MAX_SIZE) * 100);
	printf("simu time: %d, pseudoforest insert: %d, ", k, insert_count);       

	insert_count_global_2 += trad_insert_count;	
	sum_utili_2 += (trad_insert_count / (2.0 * MAX_SIZE) * 100);
	printf("traditional insert: %d\n", trad_insert_count);
//***************************************************************************
    }

    print_tradition(sum_utili_2, runtime_2);
    
    print_ave_result(sum_utili_1, runtime_1, u_time_total);
	
    printf("|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
	return 0;
}

