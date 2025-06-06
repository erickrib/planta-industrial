#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE 256
#define MAX_SENSOR_NAME 50
#define MAX_SENSORS 100
#define MAX_READINGS 10000

typedef struct {
    time_t timestamp;
    char sensor_id[MAX_SENSOR_NAME];
    char valor[100];
} Leitura;

typedef struct {
    char nome[MAX_SENSOR_NAME];
    Leitura leituras[MAX_READINGS];
    int count;
} SensorData;

const char* sensores_suportados[] = {"TEMP", "PRES", "VIBR", "UMID", "FLUX", "PESO", "VELO", "ANGL"};
const int num_sensores_suportados = 8;

int is_sensor_suportado(const char* sensor_name) {
    for (int i = 0; i < num_sensores_suportados; i++) {
        if (strcmp(sensor_name, sensores_suportados[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int comparar_leituras(const void *a, const void *b) {
    Leitura *la = (Leitura *)a;
    Leitura *lb = (Leitura *)b;
    
    if (la->timestamp < lb->timestamp) return -1;
    if (la->timestamp > lb->timestamp) return 1;
    return 0;
}

int encontrar_sensor(SensorData sensors[], int *num_sensors, const char* sensor_name) {
    for (int i = 0; i < *num_sensors; i++) {
        if (strcmp(sensors[i].nome, sensor_name) == 0) {
            return i;
        }
    }
    
    if (*num_sensors < MAX_SENSORS) {
        strcpy(sensors[*num_sensors].nome, sensor_name);
        sensors[*num_sensors].count = 0;
        (*num_sensors)++;
        return *num_sensors - 1;
    }
    
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <arquivo_dados>\n", argv[0]);
        return 1;
    }
    
    FILE *arquivo = fopen(argv[1], "r");
    if (!arquivo) {
        printf("Erro: Não foi possível abrir o arquivo '%s'\n", argv[1]);
        return 1;
    }
    
    SensorData sensors[MAX_SENSORS];
    int num_sensors = 0;
    char linha[MAX_LINE];
    int linhas_processadas = 0;
    int linhas_validas = 0;
    
    printf("Processando arquivo: %s\n", argv[1]);
    
    while (fgets(linha, sizeof(linha), arquivo)) {
        linhas_processadas++;
        
        linha[strcspn(linha, "\n")] = 0;
        
        if (strlen(linha) == 0) continue;
        
        time_t timestamp;
        char sensor_id[MAX_SENSOR_NAME];
        char valor[100];
        
        if (sscanf(linha, "%ld %s %s", &timestamp, sensor_id, valor) == 3) {
            if (is_sensor_suportado(sensor_id)) {
                int sensor_index = encontrar_sensor(sensors, &num_sensors, sensor_id);
                
                if (sensor_index >= 0 && sensors[sensor_index].count < MAX_READINGS) {
                    sensors[sensor_index].leituras[sensors[sensor_index].count].timestamp = timestamp;
                    strcpy(sensors[sensor_index].leituras[sensors[sensor_index].count].sensor_id, sensor_id);
                    strcpy(sensors[sensor_index].leituras[sensors[sensor_index].count].valor, valor);
                    sensors[sensor_index].count++;
                    linhas_validas++;
                } else {
                    printf("Aviso: Muitas leituras para o sensor %s ou erro interno\n", sensor_id);
                }
            } else {
                printf("Aviso: Sensor '%s' não é suportado (linha %d)\n", sensor_id, linhas_processadas);
            }
        } else {
            printf("Aviso: Formato inválido na linha %d: %s\n", linhas_processadas, linha);
        }
    }
    
    fclose(arquivo);
    
    printf("Processamento concluído:\n");
    printf("- Linhas processadas: %d\n", linhas_processadas);
    printf("- Linhas válidas: %d\n", linhas_validas);
    printf("- Sensores identificados: %d\n", num_sensors);
    
    for (int i = 0; i < num_sensors; i++) {
        if (sensors[i].count > 0) {
            qsort(sensors[i].leituras, sensors[i].count, sizeof(Leitura), comparar_leituras);
            
            char nome_arquivo[100];
            sprintf(nome_arquivo, "%s.txt", sensors[i].nome);
            
            FILE *arquivo_sensor = fopen(nome_arquivo, "w");
            if (arquivo_sensor) {
                for (int j = 0; j < sensors[i].count; j++) {
                    fprintf(arquivo_sensor, "%ld %s %s\n", 
                           sensors[i].leituras[j].timestamp,
                           sensors[i].leituras[j].sensor_id,
                           sensors[i].leituras[j].valor);
                }
                fclose(arquivo_sensor);
                printf("- Arquivo criado: %s (%d leituras)\n", nome_arquivo, sensors[i].count);
            } else {
                printf("Erro: Não foi possível criar o arquivo %s\n", nome_arquivo);
            }
        }
    }
    
    return 0;
} 