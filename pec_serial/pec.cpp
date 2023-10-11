#include<stdio.h>
#include<algorithm>
#include<iostream>
#include<iomanip>
#include<fstream>
#include<math.h>
#include<string.h>
#include<map>
#include <time.h>
using namespace std;
#define F 2.5E3
#define Time 1E6
const int maxn = 100000+10;
double time_series[maxn];//time series
int m,t;//m refer to order of permuation entropy
		//t refer to delay time of permuation entropy
double ch_ti_series[maxn][maxn];

inline unsigned long long rdtsc(void)
{
	unsigned long hi = 0, lo = 0;

	__asm__ __volatile__ ("lfence;rdtsc" : "=a"(lo), "=d"(hi));

	return (((unsigned long long)lo))|(((unsigned long long)hi)<<32);
}

int data_input()
{
	char buffer[256];
	ifstream myfile ("data.in");
	if(!myfile)
	{
		cout<<"Unable to open myfile"<<endl;
		exit(1);
	}
	double temp;
	int i = 1;
	while(!myfile.eof())
	{
		myfile.getline(buffer,14);
		sscanf(buffer,"%lf",&temp);
		time_series[i]=temp;
		i++;
	}
	myfile.close();
	return i;
}

void re_organization(int _max)//correct!!!
{
	for(int i=1; i<=_max; i++)
	{
		for(int j=0; j<m; j++)
		{
			ch_ti_series[i][j] = time_series[i+j*t];
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

int main(int argc,char **argv)
{
	int my_rank=0,comm_sz=0;
	unsigned long long  timeStart;   // 仿真开始时间
	unsigned long long  timeEnd;     // 仿真结束时间
    double delapseTime = 0; // 仿真过程花费时间
	timeStart=rdtsc();
	int n = data_input();
	n -= 1;
	
	map<long long,int>dict;
    sscanf(argv[1],"%d",&m);
    sscanf(argv[2],"%d",&t);
	
    int _max = n - t * (m - 1);
	printf(" %d EEEEEEEEEEEE \n", _max);
    re_organization(_max);
	
    for(int i=1; i<=_max; i++)
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
    printf("%lf\n",sum);
    timeEnd=rdtsc();
    delapseTime=(double)(timeEnd - timeStart)/(F*Time);

	printf("The program elapsed %13.8lf s\n",delapseTime);
	return 0;
}
