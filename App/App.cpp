#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX

#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
    	printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(char *enclave_name)
{
    sgx_launch_token_t token = {0};
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int updated = 0;
    
    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(enclave_name, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}

#define MAX_SIGNED_ENCLAVE_NAME 32
struct __encl {
	char name[MAX_SIGNED_ENCLAVE_NAME];
	uint64_t size;
};

#define NUMBER_OF_SIGNED_ENCLAVES 20
static struct __encl enclaves[NUMBER_OF_SIGNED_ENCLAVES] = {
	{"enclave.signed.so", 1024*1024},
	{"enclave.signed.2MB.so", 2*1024*1024},
	{"enclave.signed.3MB.so", 3*1024*1024},
	{"enclave.signed.4MB.so", 4*1024*1024},
	{"enclave.signed.6MB.so", 6*1024*1024},
	{"enclave.signed.8MB.so", 8*1024*1024},
	{"enclave.signed.12MB.so", 12*1024*1024},
	{"enclave.signed.16MB.so", 16*1024*1024},
	{"enclave.signed.24MB.so", 24*1024*1024},
	{"enclave.signed.32MB.so", 32*1024*1024},
	{"enclave.signed.48MB.so", 48*1024*1024},
	{"enclave.signed.64MB.so", 64*1024*1024},
	{"enclave.signed.96MB.so", 96*1024*1024},
	{"enclave.signed.128MB.so", 128*1024*1024},
	{"enclave.signed.196MB.so", 196*1024*1024},
	{"enclave.signed.256MB.so", 256*1024*1024},
	{"enclave.signed.384MB.so", 384*1024*1024},
	{"enclave.signed.512MB.so", 512*1024*1024},
	{"enclave.signed.786MB.so", 786*1024*1024},
	{"enclave.signed.1GB.so", 1024*1024*1024}
};

void timespec_diff(struct timespec *start, struct timespec *stop,
                   struct timespec *result)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }

    return;
}

#define NUMBER_OF_ENTRIES 10
/* Application entry */
int main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);
    
    struct timespec enclave_results[NUMBER_OF_SIGNED_ENCLAVES][NUMBER_OF_ENTRIES];
    struct timespec start, end;

    FILE *fp;
    fp = fopen("benchmark_results", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Couldnt open or create a file for the benchmark data!\n");
    }

	for (size_t i = 0; i < NUMBER_OF_SIGNED_ENCLAVES; i++)
    {
        fprintf(fp, "%lu,", enclaves[i].size);
        uint64_t average = 0;
        for (size_t j = 0; j < NUMBER_OF_ENTRIES; j++)
        {
             clock_gettime(CLOCK_REALTIME, &start);
            /* Initialize the enclave */
            if(initialize_enclave(enclaves[i].name) < 0)
            {
            	printf("error init enclave %lu\n", i);
                return -1; 
            }
            clock_gettime(CLOCK_REALTIME, &end);
//            double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / BILLION;
            timespec_diff(&start, &end, &(enclave_results[i][j]));
//            enclave_results[i][j] = time_spent;
            /* Destroy the enclave */
            sgx_destroy_enclave(global_eid);
  			average += enclave_results[i][j].tv_sec * 1000000000 + enclave_results[i][j].tv_nsec;          
//            average += enclave_results[i][j].tv_sec + (enclave_results[i][j].tv_nsec / 1000000000.0);
        }   
        average = average/NUMBER_OF_ENTRIES;
        fprintf(fp, "%lu\n", average);
        fflush(fp);
    }
    
    fclose(fp);

    printf("Info: Enclave initialisation benchmark successfully returned.\n");

    printf("Enter a character before exit ...\n");
    getchar();
    return 0;
}

