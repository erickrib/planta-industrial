#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SENSOR_NAME 50
#define MAX_SENSORS 10
#define LEITURAS_POR_SENSOR 2000

typedef enum {
    TIPO_INT,
    TIPO_FLOAT,
    TIPO_DOUBLE,
    TIPO_BOOL
} TipoDado;

typedef struct {
    char nome[MAX_SENSOR_NAME];
    TipoDado tipo;
} DefinicaoSensor;

// Converte data/hora para timestamp (formato Unix)
time_t converter_para_timestamp(int dia, int mes, int ano, int hora, int min, int seg) {
    struct tm t;

    t.tm_year = ano - 1900;
    t.tm_mon = mes - 1;
    t.tm_mday = dia;
    t.tm_hour = hora;
    t.tm_min = min;
    t.tm_sec = seg;
    t.tm_isdst = -1;

    time_t timestamp = mktime(&t);
    if (timestamp == -1) {
        printf("Data inválida. Tente novamente.\n");
        return -1;
    }
    return timestamp;
}

// Gera um timestamp aleatório entre dois instantes
time_t gerar_timestamp_aleatorio(struct tm *inicial, struct tm *final) {
    time_t timestamp_inicial, timestamp_final;
        
    timestamp_inicial = mktime(inicial);
    if (timestamp_inicial == -1) {
        printf("Data inicial inválida.\n");
        return -1;
    }

    timestamp_final = mktime(final);
    if (timestamp_final == -1) {
        printf("Data final inválida.\n");
        return -1;
    }

    time_t timestamp_aleatorio = timestamp_inicial + rand() % (timestamp_final - timestamp_inicial + 1);
    
    return timestamp_aleatorio;
}

// Gera um valor aleatório de acordo com o tipo do sensor
void gerar_valor_aleatorio(TipoDado tipo, char* buffer) {
    switch (tipo) {
        case TIPO_INT:
            sprintf(buffer, "%d", rand() % 1000);
            break;
        case TIPO_FLOAT:
            sprintf(buffer, "%.2f", (float)(rand() % 10000) / 100.0);
            break;
        case TIPO_DOUBLE:
            sprintf(buffer, "%.6f", (double)(rand() % 1000000) / 1000000.0 * 1000);
            break;
        case TIPO_BOOL:
            sprintf(buffer, "%s", (rand() % 2) ? "true" : "false");
            break;
        default:
            sprintf(buffer, "%d", rand() % 100);
    }
}

// Converte a string "float", "int", etc. para enum TipoDado
TipoDado string_para_tipo(const char* tipo_str) {
    if (strcmp(tipo_str, "int") == 0 || strcmp(tipo_str, "INT") == 0) {
        return TIPO_INT;
    } else if (strcmp(tipo_str, "float") == 0 || strcmp(tipo_str, "FLOAT") == 0) {
        return TIPO_FLOAT;
    } else if (strcmp(tipo_str, "double") == 0 || strcmp(tipo_str, "DOUBLE") == 0) {
        return TIPO_DOUBLE;
    } else if (strcmp(tipo_str, "bool") == 0 || strcmp(tipo_str, "BOOL") == 0) {
        return TIPO_BOOL;
    } else {
        return TIPO_INT; 
    }
}

typedef struct {
    time_t timestamp;
    char sensor_id[MAX_SENSOR_NAME];
    char valor[100];
} LeituraTemp;

// Compara dois elementos aleatoriamente para o embaralhamento
int comparar_aleatorio(const void *a, const void *b) {
    (void)a; (void)b;
    return rand() % 3 - 1;
}

int main(int argc, char *argv[]) {
    // Validação de argumentos
    if (argc < 9 || (argc % 2) != 1) {
        printf("Uso: %s <dia_ini> <mes_ini> <ano_ini> <hora_ini> <min_ini> <seg_ini> <dia_fim> <mes_fim> <ano_fim> <hora_fim> <min_fim> <seg_fim> <sensor1> <tipo1> [<sensor2> <tipo2> ...]\n", argv[0]);
        return 1;
    }

    srand((unsigned int)time(NULL)); // semente aleatória

    // Leitura de data e hora inicial
    int dia_ini = atoi(argv[1]), mes_ini = atoi(argv[2]), ano_ini = atoi(argv[3]);
    int hora_ini = atoi(argv[4]), min_ini = atoi(argv[5]), seg_ini = atoi(argv[6]);

    // Leitura de data e hora final
    int dia_fim = atoi(argv[7]), mes_fim = atoi(argv[8]), ano_fim = atoi(argv[9]);
    int hora_fim = atoi(argv[10]), min_fim = atoi(argv[11]), seg_fim = atoi(argv[12]);

    // Validação de intervalo de tempo
    struct tm tm_inicial = {0}, tm_final = {0};
    tm_inicial.tm_year = ano_ini - 1900;
    tm_inicial.tm_mon = mes_ini - 1;
    tm_inicial.tm_mday = dia_ini;
    tm_inicial.tm_hour = hora_ini;
    tm_inicial.tm_min = min_ini;
    tm_inicial.tm_sec = seg_ini;

    tm_final.tm_year = ano_fim - 1900;
    tm_final.tm_mon = mes_fim - 1;
    tm_final.tm_mday = dia_fim;
    tm_final.tm_hour = hora_fim;
    tm_final.tm_min = min_fim;
    tm_final.tm_sec = seg_fim;

    time_t ts_inicial = mktime(&tm_inicial);
    time_t ts_final = mktime(&tm_final);

    if (ts_inicial >= ts_final) {
        printf("Erro: A data final deve ser posterior à data inicial.\n");
        return 1;
    }

    // Leitura dos sensores e tipos
    int num_sensores = (argc - 13) / 2;
    if (num_sensores > MAX_SENSORS) {
        printf("Erro: Muitos sensores. Máximo suportado: %d\n", MAX_SENSORS);
        return 1;
    }

    DefinicaoSensor sensores[MAX_SENSORS];
    for (int i = 0; i < num_sensores; i++) {
        strcpy(sensores[i].nome, argv[13 + i * 2]);
        sensores[i].tipo = string_para_tipo(argv[13 + i * 2 + 1]);
    }

    // Mensagens iniciais
    printf("Gerando arquivo de teste...\n");
    printf("Sensores: %d\n", num_sensores);

    // Aloca vetor com todas as leituras
    int total_leituras = num_sensores * LEITURAS_POR_SENSOR;
    LeituraTemp* todas_leituras = malloc(total_leituras * sizeof(LeituraTemp));
    if (!todas_leituras) {
        printf("Erro de memória.\n");
        return 1;
    }

    // Geração dos dados para cada sensor
    int indice_geral = 0;
    for (int s = 0; s < num_sensores; s++) {
        for (int i = 0; i < LEITURAS_POR_SENSOR; i++) {
            time_t ts = gerar_timestamp_aleatorio(&tm_inicial, &tm_final);
            char valor[100];
            gerar_valor_aleatorio(sensores[s].tipo, valor);

            todas_leituras[indice_geral].timestamp = ts;
            strcpy(todas_leituras[indice_geral].sensor_id, sensores[s].nome);
            strcpy(todas_leituras[indice_geral].valor, valor);
            indice_geral++;
        }
    }

    // Embaralha o vetor
    qsort(todas_leituras, total_leituras, sizeof(LeituraTemp), comparar_aleatorio);

    // Escreve no arquivo final
    FILE* arquivo = fopen("dados_sensores.txt", "w");
    if (!arquivo) {
        printf("Erro ao criar arquivo.\n");
        free(todas_leituras);
        return 1;
    }

    for (int i = 0; i < total_leituras; i++) {
        fprintf(arquivo, "%ld %s %s\n", 
                todas_leituras[i].timestamp,
                todas_leituras[i].sensor_id,
                todas_leituras[i].valor);
    }

    fclose(arquivo);
    free(todas_leituras);

    // Sucesso!
    printf("Arquivo 'dados_sensores.txt' gerado com sucesso.\n");
    printf("Total: %d leituras\n", total_leituras);
    printf("Use: ./programa1 dados_sensores.txt\n");

    return 0;
}