#include<stdio.h>
#include<algorithm>
#include<iostream>
#include<iomanip>
#include<fstream>
#include<math.h>
#include<string.h>
#include<map>
#include<omp.h>
#include<mpi.h>
#include <time.h>
#define OPENMP

using namespace std;
const int maxn = 100000+10;
double time_series[maxn];

const int maxx = 1e9;

long long res;
long long ans[maxn*10100];
int nArrSortIndex[maxn];
double ch_ti_series_1D[maxn*10010];//所需矩阵
int n = 50000;

void data_input()//数据输入
{
	char _buffer[256];
	ifstream myfile ("data.in");
	if(!myfile)
	{
		cout<<"Unable to open myfile"<<endl;
		exit(1);
	}
	double temp;
	int i = 0;
	while(!myfile.eof())
	{
		myfile.getline(_buffer,14);
		sscanf(_buffer,"%lf",&temp);
		time_series[i]=temp;//time_series[]表示data.in里面的每一个数据
		i++;
	}
	myfile.close();
}

void re_organization(int _max,int m,int t,double *ch_ti_series_1D,double *time_series)//correct!!!
//将data.in里面的数据重构成矩阵
{

	for(int i=0; i<_max; i++)
	{
		for(int j=0; j<m; j++)
		{
			ch_ti_series_1D[i*m+j] = time_series[i+j*t];//ch_ti_series_1D[]用一维数组将所需矩阵按照行的顺序存储
			//printf("%lf\n",ch_ti_series_1D[i*m+j]);
		}
	}
}



int main(int argc, char **argv)
{
#ifdef OPENMP
			printf("MPI  +  OpenMP\n\n");
#else
	        printf("MPI  Only\n\n");
#endif
#ifdef OPENMP
	#pragma omp parallel num_threads(4)
#endif
	int my_rank=0,comm_sz=0;
	int _max;

	double timeStart;   // 仿真开始时间
	double timeEnd;     // 仿真结束时间

	int line;
	int m,t;

	map<long long,int>dict;  //记录矩阵每一行排好序后的下标出现几次

    MPI_Init(&argc, &argv);//并行初始化函数，代表并行部分开始
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);//可以获得当前进程的标识号，MPI_COMM_WORLD是该进程所在的通信域，my_rank是该进程在通信域中的标识号
 	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);//获取通信域中包含的进程总数

 	sscanf(argv[1],"%d",&m);//将argv[1]的值以整数的形式赋给m
    sscanf(argv[2],"%d",&t);//m:嵌入维数  t:延迟时间
 	_max = n - t * (m - 1);//_max就是k 是矩阵的行数
 	line = _max/comm_sz;//一个进程分配到的行数  通过进程标识和总数来分配数据

 	int nTempIndex = 0;//用来交换索引
	double dTempValue = 0.0;//用来交换数据
 	if(my_rank == 0) //如果是0号进程的话
 	{
 		timeStart=MPI_Wtime();//返回用一个浮点数表示的秒数
 		data_input();//输入data.in里面的数据
    	re_organization(_max,m,t,ch_ti_series_1D,time_series);

    	for(int i=1; i<comm_sz; i++)//0号进程给其他所有进程发送消息
    	{
    		MPI_Send(ch_ti_series_1D + (i-1)*line*m, m*line, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
    	}

    	for(int i=1; i<comm_sz; i++)//0号进程接收其他所有进程发送的消息
    	{
    		MPI_Recv(ans, line, MPI_LONG_LONG_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    		//ans用来存储矩阵每一行排好序后的序列
    		for(int j=0; j<line; j++)
    		{
    			dict[ans[j]  ]++;
    		}
    	}

    	for(int i=(comm_sz-1)*line; i<_max; i++)//总共进行line次循环
    	{
    		for(int j=0; j<m; j++)
	    	{
	   	 		nArrSortIndex[j] = j+1;//初始化nArrSortIndex[]的值是1，2，3......m
	    	}
#ifdef OPENMP
			#pragma omp parallel
#endif
	    	for(int j=0; j<m; j++)//奇偶排序
	    	{
#ifdef OPENMP			
	    	 #pragma omp parallel
#endif
	    		if(j%2 == 1)//j是奇数
                {
#ifdef OPENMP			
	    	#pragma omp for nowait
#endif

	    			for(int k=1; k<m-1; k=k+2)
	    			if(ch_ti_series_1D[i*m+k] > ch_ti_series_1D[i*m+k+1])
	    			{
	    				dTempValue = ch_ti_series_1D[i*m+k];
						nTempIndex = nArrSortIndex[k];

						ch_ti_series_1D[i*m+k] = ch_ti_series_1D[i*m+k+1];
						nArrSortIndex[k] = nArrSortIndex[k+1];

						ch_ti_series_1D[i*m+k+1] = dTempValue;
						nArrSortIndex[k+1]=nTempIndex;
	    			}
	    		}
	    		if(j%2 == 0) //j是偶数
                {
#ifdef OPENMP			
	    	#pragma omp for nowait
#endif

	    			for(int k=0; k<m-1; k=k+2)
	    			if(ch_ti_series_1D[i*m+k] > ch_ti_series_1D[i*m+k+1])
	    			{
	    				dTempValue = ch_ti_series_1D[i*m+k];
						nTempIndex = nArrSortIndex[k];

						ch_ti_series_1D[i*m+k] = ch_ti_series_1D[i*m+k+1];
						nArrSortIndex[k] = nArrSortIndex[k+1];

						ch_ti_series_1D[i*m+k+1] = dTempValue;
						nArrSortIndex[k+1]=nTempIndex;
	    			}
	    		}
	    	}

	    	res = 0;
			for(int j=0; j<m; j++)//统计数据
			{
				res = res*(long long)10 + (long long)(nArrSortIndex[j]);
			}
			dict[res]++;
    	}

    	map<long long,int>::iterator it;
		it = dict.begin();
    	double sum = 0.0;
    	while(it!=dict.end())//计算结果
   		{
    		if(it->second != 0)
    		{
    			double p_temp = (double)it->second/(double)_max;
    			sum -= (p_temp *log(p_temp));
    		}
    		it++;
    	}
    	timeEnd=MPI_Wtime();
    	printf("%lf time:%16.8lf\n",sum,timeEnd-timeStart);
 	}

 	//如果不是0号进程，接收0号进程的消息
 	else{
 		MPI_Recv(ch_ti_series_1D,line*m,MPI_DOUBLE,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
 		//printf("slave process %.10lf\n",ch_ti_series_1D[0]);
 		for(int i=0; i<line; i++)//遍历每一个进程分配的几行任务
 		{
 			for(int j=0; j<m; j++)
	    	{
	   	 		nArrSortIndex[j] = j+1;//初始化nArrSortIndex[]的值是1，2，3......m，用作索引下标
	   	 		//printf("j == %d\n",nArrSortIndex[j]);
	    	}
#ifdef OPENMP			
	    	#pragma omp parallel
#endif


	    	for(int j=0; j<m; j++)//奇偶排序
	    	{
#ifdef OPENMP			
	    	#pragma omp parallel
#endif

	    		if(j%2 == 1){
#ifdef OPENMP			
	    	#pragma omp for nowait
#endif

	    			for(int k=1; k<m-1; k=k+2)
	    			if(ch_ti_series_1D[i*m+k] > ch_ti_series_1D[i*m+k+1])
	    			{
	    				dTempValue = ch_ti_series_1D[i*m+k];
						nTempIndex = nArrSortIndex[k];

						ch_ti_series_1D[i*m+k] = ch_ti_series_1D[i*m+k+1];
						nArrSortIndex[k] = nArrSortIndex[k+1];

						ch_ti_series_1D[i*m+k+1] = dTempValue;
						nArrSortIndex[k+1]=nTempIndex;
	    			}
	    		}
	    		if(j%2 == 0){
#ifdef OPENMP			
	    	#pragma omp for nowait
#endif
	    			for(int k=0; k<m-1; k=k+2)
	    			if(ch_ti_series_1D[i*m+k] > ch_ti_series_1D[i*m+k+1])
	    			{
	    				dTempValue = ch_ti_series_1D[i*m+k];
						nTempIndex = nArrSortIndex[k];

						ch_ti_series_1D[i*m+k] = ch_ti_series_1D[i*m+k+1];
						nArrSortIndex[k] = nArrSortIndex[k+1];

						ch_ti_series_1D[i*m+k+1] = dTempValue;
						nArrSortIndex[k+1]=nTempIndex;
	    			}
	    		}
	    	}

	    	res = 0;
			for(int j=0; j<m; j++)
			{
				res = res*(long long)10 + (long long)(nArrSortIndex[j]);
			}
			ans[i]=res;
 		}
 		MPI_Send(ans,line,MPI_LONG_LONG_INT,0,3,MPI_COMM_WORLD);//将排好序的次序发送给0号进程
 	}
 	MPI_Finalize();
	return 0;
}

