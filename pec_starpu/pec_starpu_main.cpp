#include<starpu.h>
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
#define F 2.5E3 //F×Time为主频
#define Time 1E6
#define maxn 100000 + 10
extern void permutation_entropy(void *buffers[], void *_args);
int  m, t;//m refer to order of permuation entropy
		//t refer to delay time of permuation entropy


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
	fprintf(stderr, " \n Date read succeed: %d  \n", i);
	return i;
}

inline unsigned long long rdtsc(void)
{
	unsigned long hi = 0, lo = 0;

	__asm__ __volatile__ ("lfence;rdtsc" : "=a"(lo), "=d"(hi));

	return (((unsigned long long)lo))|(((unsigned long long)hi)<<32);
}

// #ifdef __cplusplus
	// extern "C" {
// #endif /* __cplusplus */
		// struct starpu_codelet cl =
		// {
			// .where = STARPU_CPU,
			// /* CPU implementation of the codelet */
			// .cpu_funcs = { permutation_entropy },
			// .cpu_funcs_name = { "permutation_entropy" },
			// .nbuffers = 1,
			// .modes = { STARPU_RW }
		// };
// #ifdef __cplusplus
	// }
// #endif 

		
int main(int argc, char **argv)
{   
    struct starpu_codelet cl =
		{
			.where = STARPU_CPU,
			
		};
		cl.where = STARPU_CPU;
		cl.cpu_funcs[0] =  permutation_entropy ;
		//要用strcpy才行哦。这样是错的。cl.cpu_funcs_name[0] = "permutation_entropy" ;
		cl.nbuffers = 1;
		cl.modes[0] = STARPU_RW ;
	
	unsigned long long  timeStart;   // 仿真开始时间
	unsigned long long  timeEnd;     // 仿真结束时间
	timeStart=rdtsc();
    double delapseTime = 0; // 仿真过程花费时间
	
    double vector[maxn];// vector即时间序列
	int N = data_input(vector);
	sscanf(argv[1],"%d",&m);
    sscanf(argv[2],"%d",&t);
	
    //启动starpu，创建task，将数据vector注册给starpu。
    starpu_init(NULL);
		
    starpu_data_handle_t vector_handle;
    starpu_vector_data_register(&vector_handle, STARPU_MAIN_RAM, (uintptr_t)vector, N, sizeof(vector[0]));

    struct starpu_task *task = starpu_task_create();
    task->synchronous = 1;
    task->cl = &cl;
    task->handles[0] = vector_handle;
    
   
    starpu_task_submit(task); 
    starpu_data_unregister(vector_handle);
    starpu_shutdown();
    fprintf(stderr, "Mission completed. \n");
	
	timeEnd=rdtsc();
    delapseTime=(double)(timeEnd - timeStart)/(F*Time);
	printf("The program elapsed %13.8lf s\n",delapseTime);
	
    return 0;
}