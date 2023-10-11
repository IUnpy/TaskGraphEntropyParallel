#include<stdio.h>
#include<algorithm>
#include<iostream>
#include<iomanip>
#include<fstream>
#include<math.h>
#include<string.h>
#include<map>
#include <time.h>
#include<starpu.h>
#include<pthread.h>
using namespace std;
#define maxn 100000 + 10
extern pthread_mutex_t mut;// 给map<long long ,int>加个锁。
struct params
{
    int begin, end;
};


void permutation_entropy(void *buffers[], void *cl_arg);
extern int m,t,max,average;
int sem = 0;
double ch_ti_series[maxn][maxn];
map<long long,int> dict;

void re_organization(int begin , int end, double *time_series)//correct!!!
{
	for(int i = begin ; i < end; i++)
	{
		for(int j = 0; j < m; j++)
		{   
	        // printf("%10d   %10d\n",begin,end);
			ch_ti_series[i][j] = time_series[i + j*t];
		}
	}
}

long long Xi_sort(double *solve)
{
	int nArrSortIndex[m*10];
	int nTempIndex = 0;
	double dTempValue = 0;
	for(int i=0; i<m; i++)
	{
		nArrSortIndex[i] = i+1;
	}
	for(int i=0; i<m; i++)
	{
		for(int j=m-1; j>i; j--)
		{
			if(solve[j-1] > solve[j])
			{
				dTempValue = solve[j-1];
				nTempIndex = nArrSortIndex[j-1];

				solve[j-1] = solve[j];
				nArrSortIndex[j-1] = nArrSortIndex[j];

				solve[j] = dTempValue;
				nArrSortIndex[j]=nTempIndex;
			}
		}
	}
	long long res = 0;
	for(int i=0; i<m; i++)
	{
		res = res*(long long)10 + (long long)(nArrSortIndex[i]);
	}
	return res;
}

void permutation_entropy(void *buffers[], void *cl_arg)
{	
   
    struct params *params = (struct params*)cl_arg;
    struct starpu_vector_interface *vector = (starpu_vector_interface*)buffers[0];
    unsigned n = STARPU_VECTOR_GET_NX(vector);
    double *val = (double *)STARPU_VECTOR_GET_PTR(vector);
    int begin = params->begin, end = params->end;
	printf("\n  %d -----> %d \n", begin, end);
	//end = n - t * (m - 1);
	

    re_organization(begin,end,val);//矩阵重组
	
    for(int i = begin; i < end; i++) //每一行排序后映射 到字典
    {
    	long long _rank = Xi_sort(ch_ti_series[i]);
		pthread_mutex_lock(&mut);
    	dict[_rank]++;
		pthread_mutex_unlock(&mut);
    }
    
}
