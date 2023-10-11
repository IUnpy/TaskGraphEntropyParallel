#include<starpu.h>
#include<stdio.h>
#include<algorithm>
#include<iostream>
#include<iomanip>
#include<fstream>
#include<math.h>
#include<string.h>
#include<map>
#include<pthread.h>
#include <time.h>
using namespace std;
#define F 2.5E3 //F×Time为主频
#define Time 1E6
#define maxn 100000 + 10
pthread_mutex_t mut;// 给map<long long ,int>加个锁。

struct params
{
    int begin, end;
};
extern void permutation_entropy(void *buffers[], void *_args);
extern map<long long,int>dict;
int  m, t, _max,average;
 /*m refer to order of permuation entropy .t refer to delay time of permuation entropy
 _max一共有多少行需要排列，，average是 平均每个核排列多少行。
 */
int data_input(double *time_series)
{
	char buffer[256];
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
		myfile.getline(buffer,14);
		sscanf(buffer,"%lf",&temp);
		time_series[i]=temp;
		i++;
	}
	myfile.close();
	fprintf(stderr, "Date read succeed: %d  \n", i);
	return i;
}

inline unsigned long long rdtsc(void)
{
	unsigned long hi = 0, lo = 0;

	__asm__ __volatile__ ("lfence;rdtsc" : "=a"(lo), "=d"(hi));

	return (((unsigned long long)lo))|(((unsigned long long)hi)<<32);
}

//./fianl m t CORES 	
int main(int argc, char **argv)
{   
    pthread_mutex_init(&mut, NULL);//给map<long long, int>加个锁
	unsigned long long  timeStart;   // 仿真开始时间
	unsigned long long  timeEnd;     // 仿真结束时间
	timeStart=rdtsc();
    double delapseTime = 0; // 仿真过程花费时间
	
    double vector[maxn];// vector即时间序列
	int N = data_input(vector);
	int CORES;
	sscanf(argv[3],"%d",&CORES);
	sscanf(argv[1],"%d",&m);
    sscanf(argv[2],"%d",&t);
	_max  = N - t * (m - 1);//一共有_max行需要排列
	average = _max / CORES;
	
	
    //启动starpu，创建task，将数据vector注册给starpu。
    starpu_init(NULL);
    starpu_data_handle_t vector_handle;
    starpu_vector_data_register(&vector_handle, STARPU_MAIN_RAM, (uintptr_t)vector, N, sizeof(vector[0]));
    
	struct params my_params[CORES+1];
	for (int i = 0; i < CORES; i++){
			my_params[i] = { i*average, ( i+ 1) *average };
			if (i == CORES - 1)  my_params[i].end = _max;
	}
	for (int i = 0; i < CORES; i++){
		struct starpu_task *task = starpu_task_create();
		struct starpu_codelet cl;
		cl.where = STARPU_CPU;
		//cl.cpu_func = permutation_entropy;
		cl.cpu_funcs[0] =  permutation_entropy ;
		cl.nbuffers = 1;
		cl.modes[0] = STARPU_R ;//只读模式访问，以便于并行
		task->synchronous = 0;//异步运行，提交任务后立刻返回
        task->cl = &cl;
        task->handles[0] = vector_handle;
			
        task->cl_arg = &my_params[i];
        task->cl_arg_size = sizeof(my_params[0]);
		starpu_task_submit(task); //提交任务
	}
		  
  
	starpu_task_wait_for_all();//等待全部的worker工作完成
    starpu_data_unregister(vector_handle);
    starpu_shutdown();
	
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
	
	timeEnd=rdtsc();
    delapseTime=(double)(timeEnd - timeStart)/(F*Time);
	printf("%d  CORES,  Time elapsed %13.8lf s\n",CORES,delapseTime);
	
    return 0;
}