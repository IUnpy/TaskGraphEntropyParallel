#include<starpu.h>
extern int permutation_entropy(void *buffers[], void *_args);

extern "C" {
struct starpu_codelet cl =
		{
			.where = STARPU_CPU,
			/* CPU implementation of the codelet */
			.cpu_funcs = { permutation_entropy },
			.cpu_funcs_name = { "permutation_entropy" },
			.nbuffers = 1,
			.modes = { STARPU_RW }
		};
}