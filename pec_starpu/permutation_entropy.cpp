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
using namespace std;
#define maxn 100000 + 10
void permutation_entropy(void *buffers[], void *cl_arg);
extern int m,t;//m refer to order of permuation entropy
		//t refer to delay time of permuation entropy
double ch_ti_series[maxn][maxn];


void re_organization(int _max, double *time_series)//correct!!!
{
	for(int i = 0 ; i < _max; i++)
	{
		for(int j = 0; j < m; j++)
		{
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
    
    struct starpu_vector_interface *vector = (starpu_vector_interface*)buffers[0];
    unsigned n = STARPU_VECTOR_GET_NX(vector);
    double *val = (double *)STARPU_VECTOR_GET_PTR(vector);
	
    map<long long,int>dict;
    int _max = n - t * (m - 1);
	

    re_organization(_max,val);//矩阵重组
	
    for(int i=0; i < _max; i++) //每一行排序后映射到字典
    {
    	long long _rank = Xi_sort(ch_ti_series[i]);
    	dict[_rank]++;
    }
	
    map<long long,int>::iterator it;
	it = dict.begin();
    double sum = 0.0;
    while(it!=dict.end())
    {
    	if(it->second != 0)
    	{
    		double p_temp = (double)it->second/(double)_max;
    		sum -= (p_temp *log(p_temp));
    	}
    	it++;
    }
    printf("The permutation entropy is: %lf\n",sum);
}
