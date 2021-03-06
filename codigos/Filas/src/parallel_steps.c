#include <stdio.h>
#include <errno.h>  // for errno
#include <math.h>
#include <limits.h> // for INT_MAX
#include <stdlib.h> // for strtol
#include <time.h>
#include <omp.h>

long number_of_threads = 2;
long max_number_of_char = 10;
long number_of_types_char = 10;
long number_of_results = 5;
long number_of_queues = 2;
long detail_level = 0;

long* ids = NULL;
long* results = NULL;
long* n_of_chars = NULL;

typedef struct Client
{
    long*     initial_characteristics;
    long**    derived_characteristics;
    long      number_of_init_chars;
    long      identifier;
    long      result;
} Client;

long rand_between(long l, long r, unsigned int seed) { 
    int value;
    #pragma omp critical (rand)
    {
        srand(seed);
        value = rand();
    }
    return (long) (l + (value % (r - l + 1))); 
} 

Client* createClient(long id, unsigned int seed) {
    Client* client = malloc(sizeof(Client));
    client->number_of_init_chars = rand_between(0, max_number_of_char, seed);
    seed += client->number_of_init_chars;
    if(client->number_of_init_chars > 0) {
        client->initial_characteristics = malloc(client->number_of_init_chars*sizeof(long));
        //printf("id: %ld - init size: %ld\n", id, client->number_of_init_chars);
        for (long i = 0; i < client->number_of_init_chars; ++i) {
            client->initial_characteristics[i] = rand_between(1, number_of_types_char, seed);
            seed += client->initial_characteristics[i];
        }
        if(number_of_queues - 1 > 0){
            client->derived_characteristics = malloc((number_of_queues-1)*sizeof(long*));
            for (long i = 0; i < number_of_queues-1; ++i) {
                client->derived_characteristics[i] = malloc((client->number_of_init_chars+i+1)*sizeof(long));
                //printf("id: %ld - level: %ld - size: %ld\n", id, i, client->number_of_init_chars+i+1);
            }
        }
    } else {
        client->initial_characteristics = NULL;
        client->derived_characteristics = NULL;
    }
    
    client->identifier = id;
    client->result = -1;
    return client;
}

void destroyClient(Client* client){
    if(client->number_of_init_chars > 0) {
        free(client->initial_characteristics);
        if(number_of_queues-1 > 0){
            for (long i = 0; i < number_of_queues-1; ++i) {
                free(client->derived_characteristics[i]);
            }
            free(client->derived_characteristics);
        }
    }
    free(client);
}

void printClient(Client* client, short int detail_level) {
    #pragma omp critical (print)
    {
        if(detail_level > 1) {
            printf("\nId: %ld\n", client->identifier);
            printf("Result: %ld\n", client->result);
            if(detail_level > 1) {
                printf("Nº of characteristics: %ld\n", client->number_of_init_chars);
                if(client->number_of_init_chars > 0) {
                    printf("Characteristics: %ld", client->initial_characteristics[0]);
                    for (long i = 1; i < client->number_of_init_chars; ++i) {
                        printf(", %ld", client->initial_characteristics[i]);
                    }
                }
                for (long i = 0; i < number_of_queues - 1; ++i) {
                    if(detail_level > i+2) {
                        if(client->number_of_init_chars > 0) {
                            printf("\nCharacteristics on queue %ld: %ld", i+1, client->derived_characteristics[i][0]);
                            for (long j = 1; j < client->number_of_init_chars+i+1; ++j) {
                                printf(", %ld", client->derived_characteristics[i][j]);
                            }
                        }
                    }
                }
            }
            printf("\n");
        }
    }
}

void printClientCSV(Client* client) {
    printf("\n%ld, %ld, %ld\n", client->identifier, client->result, client->number_of_init_chars);
}

void printCSV(long* ids, long* results, long* n_of_chars, long number_of_clients) {
    printf("ID,Result,Nº Initial Characteristics\n");
    for (long i = 0; i < number_of_clients; ++i) {
        printf("%ld, %ld, %ld\n", ids[i], results[i], n_of_chars[i]);
    }
    
    
}

long initialCharProcess(Client* client) {
    long value = 0;
    for (long i = 0; i < client->number_of_init_chars; ++i) {
        value += client->initial_characteristics[i];
    }
    return (value / (client->number_of_init_chars + 1)) + (value % (client->number_of_init_chars + 1));
}

long levelCharProcess(Client* client, long* values, long level) {
    // printf("client: %ld - chars: %ld\n", client->identifier, client->number_of_init_chars);
    long* origin = NULL;
    if (level == 0) {
        origin = client->initial_characteristics;
    } else {
        origin = client->derived_characteristics[level-1];
    }
    long value = 0;
    // printf("client: %ld - level: %ld - level size: %ld\n", client->identifier, level, client->number_of_init_chars+level);
    for (long i = 0; i < client->number_of_init_chars+level-1; ++i) {
        client->derived_characteristics[level][i] = 0;
        for (long j = 0; j < client->number_of_init_chars+level; ++j) {
            client->derived_characteristics[level][i] += abs(origin[i] - origin[j]);
        }
        // printf("client: %ld - %ld: %ld\n", client->identifier,i, client->derived_characteristics[level][i]);
        value += client->derived_characteristics[level][i];
    }
    client->derived_characteristics[level][client->number_of_init_chars+level-1] = abs(origin[client->number_of_init_chars+level-1] - origin[0]);
    // printf("client: %ld - %ld: %ld\n", client->identifier,client->number_of_init_chars+level-1, client->derived_characteristics[level][client->number_of_init_chars+level-1]);
    
    client->derived_characteristics[level][client->number_of_init_chars+level] = values[level];
    // printf("client: %ld - %ld: %ld\n", client->identifier,client->number_of_init_chars+level, client->derived_characteristics[level][client->number_of_init_chars+level]);

    value += client->derived_characteristics[level][client->number_of_init_chars+level-1];
    value += client->derived_characteristics[level][client->number_of_init_chars+level];
    return (value * client->number_of_init_chars) % number_of_types_char;  
}

long categoryFromValue(long* values) {
    long result = 0;
    if(values[0] == 0){
        return 0;
    }
    for (long i = 0; i < number_of_queues; ++i) {
        result += values[i] * (i+1);
    }
    result = result / number_of_queues;

    return 1 + ( result % number_of_results);
}

void categorizeClient(Client* client) {
    if(client->number_of_init_chars == 0) {
        client->result = 0;
        return;
    }
    long* values = calloc(number_of_queues, sizeof(long));
    values[0] = initialCharProcess(client);
    //printf("initial value: %ld\n", values[0]);
    for (long i = 0; i < number_of_queues-1; ++i) {
        values[i+1] += levelCharProcess(client, values, i);
        //printf("value %ld: %ld\n", i, values[i]);
    }
    
    client->result = categoryFromValue(values);
    free(values);
}

void taskProcessClient(Client* client, long* clientValues, long step){
    // printf("process step %ld of %ld - %d\n", step, client->identifier, omp_get_thread_num());
    if(client->number_of_init_chars == 0) {
        client->result = 0;
        ids[client->identifier] = client->identifier;
        results[client->identifier] = client->result;
        n_of_chars[client->identifier] = client->number_of_init_chars;
        printClient(client, detail_level);
        return;
    }
    if(step < number_of_queues-1) {
        clientValues[step+1] += levelCharProcess(client, clientValues, step);
        // printf("step: %ld - number_of_queues: %ld - result %ld\n", step, number_of_queues, clientValues[step+1]);
        #pragma omp task
        {
            taskProcessClient(client, clientValues, step+1);
        }
    } else {
        // #pragma omp task
        // {
            // printf("step: %ld - number_of_queues: %ld\n", step, number_of_queues);
            client->result = categoryFromValue(clientValues);
            ids[client->identifier] = client->identifier;
            results[client->identifier] = client->result;
            n_of_chars[client->identifier] = client->number_of_init_chars;
            printClient(client, detail_level);
        // }
    }
}

long convert_str_long(char *str){
    char *p;
    errno = 0;
    long conv = strtol(str, &p, 10);

    if (errno != 0 || *p != '\0')
    {
        printf("%s não é um número!\n", str);
        exit(-1);
    }
    return (long)conv;
}

int main(int argc, char **argv){

    if (argc != 9) {
        printf("É necessário informar os seguintes argumentos:\n");
        printf("Quantidade de threads a serem usadas\n");
        printf("Seed usada para gerar os dados\n");
        printf("Número de clientes a serem criados\n");
        printf("Quantidade máxima de caracteristicas por cliente\n");
        printf("Quantidade de tipos de caracteristicas\n");
        printf("Quantidade de resultados diferentes do 0\n");
        printf("Número de etapas a serem utilizadas para processar o resultado\n");
        printf("Nível de detalhe da exibição dos resultados:\n");
        printf(" - Caso seja 0: Imprime apenas o tempo gasto\n");
        printf(" - Caso seja 1: Imprime o Id, o Resultado e o Nª de categorias de cada cliente em um .csv\n");
        printf(" - Caso seja n: Imprime os dados de cada cliente detalhando as caracteristicas de até n-1 etapas e ao final os dados de detalhe 1\n");
        return -1;
    }

    number_of_threads = convert_str_long(argv[1]);
    unsigned int seed = convert_str_long(argv[2]);

    long number_of_clients = convert_str_long(argv[3]);
    max_number_of_char = convert_str_long(argv[4]);
    number_of_types_char = convert_str_long(argv[5]);
    number_of_results = convert_str_long(argv[6]);
    number_of_queues = convert_str_long(argv[7]);
    detail_level = convert_str_long(argv[8]);

    ids = malloc(number_of_clients*sizeof(long));
    results = malloc(number_of_clients*sizeof(long));
    n_of_chars = malloc(number_of_clients*sizeof(long));

    Client** clients = malloc(number_of_clients*sizeof(Client*));

    Client*** levels = malloc((number_of_queues)*sizeof(Client**));
    long** values = malloc(number_of_clients*sizeof(long*));

    for (size_t i = 0; i < number_of_queues; ++i) {
        levels[i] = malloc(number_of_clients*sizeof(Client*));
    }

    for (long i = 0; i < number_of_clients; ++i) {
        clients[i] = createClient(i, seed+i);
        values[i] = calloc(number_of_queues, sizeof(long));
    }
    double t = omp_get_wtime();

    #pragma omp parallel num_threads(number_of_threads) default(none) \
    shared(number_of_clients, values, clients, levels)
    {
        #pragma omp for schedule(guided)
        for (long i = 0; i < number_of_clients; ++i) {
            values[i][0] = initialCharProcess(clients[i]);
            //printf("inicio %ld - %d\n", i, omp_get_thread_num());
            #pragma omp task
            {
                taskProcessClient(clients[i], values[i], 0);
            }
        }
    }

    #pragma omp taskwait
    t = omp_get_wtime() - t;

    if(detail_level > 0) {
        printCSV(ids, results, n_of_chars, number_of_clients);
    } else {
        printf("%.10lf\n", t);
    }
  
    for (long i = 0; i < number_of_clients; ++i) {
        destroyClient(clients[i]);
        free(values[i]);
    }

    for (size_t i = 0; i < number_of_queues; ++i) {
        free(levels[i]);
    }

    free(levels);
    free(values);

    free(clients);
    free(ids);
    free(results);
    free(n_of_chars);
    return 0;
} /* main */