#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_LINE 256
#define MAX_SENSOR_NAME 50

typedef struct {
    time_t timestamp;
    char sensor_id[MAX_SENSOR_NAME];
    char valor[100];
} Leitura;

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

int carregar_leituras(const char* nome_sensor, Leitura** leituras) {
    char nome_arquivo[100];
    sprintf(nome_arquivo, "%s.txt", nome_sensor);
    
    FILE* arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        printf("Erro: Arquivo '%s' não encontrado.\n", nome_arquivo);
        printf("Certifique-se de que o programa1 foi executado primeiro.\n");
        return -1;
    }
    
    int count = 0;
    char linha[MAX_LINE];
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (strlen(linha) > 1) count++;
    }
    
    if (count == 0) {
        printf("Erro: Arquivo '%s' está vazio.\n", nome_arquivo);
        fclose(arquivo);
        return -1;
    }
    
    *leituras = (Leitura*)malloc(count * sizeof(Leitura));
    if (!*leituras) {
        printf("Erro: Não foi possível alocar memória.\n");
        fclose(arquivo);
        return -1;
    }
    
    rewind(arquivo);
    int i = 0;
    while (fgets(linha, sizeof(linha), arquivo) && i < count) {
        linha[strcspn(linha, "\n")] = 0;
        
        if (strlen(linha) > 0) {
            if (sscanf(linha, "%ld %s %s", 
                      &(*leituras)[i].timestamp,
                      (*leituras)[i].sensor_id,
                      (*leituras)[i].valor) == 3) {
                i++;
            }
        }
    }
    
    fclose(arquivo);
    return i;
}

int busca_binaria_proxima(Leitura* leituras, int tamanho, time_t timestamp_alvo) {
    if (tamanho == 0) return -1;
    
    int esquerda = 0;
    int direita = tamanho - 1;
    int melhor_indice = 0;
    time_t menor_diferenca = labs(leituras[0].timestamp - timestamp_alvo);
    
    while (esquerda <= direita) {
        int meio = esquerda + (direita - esquerda) / 2;
        time_t diferenca_atual = labs(leituras[meio].timestamp - timestamp_alvo);
        
        if (diferenca_atual < menor_diferenca) {
            menor_diferenca = diferenca_atual;
            melhor_indice = meio;
        }
        
        if (leituras[meio].timestamp == timestamp_alvo) {
            return meio;
        } else if (leituras[meio].timestamp < timestamp_alvo) {
            esquerda = meio + 1;
        } else {
            direita = meio - 1;
        }
    }
    
    int meio_final = esquerda + (direita - esquerda) / 2;
    
    if (meio_final > 0) {
        time_t diff_esq = labs(leituras[meio_final - 1].timestamp - timestamp_alvo);
        if (diff_esq < menor_diferenca) {
            menor_diferenca = diff_esq;
            melhor_indice = meio_final - 1;
        }
    }
    
    if (meio_final < tamanho) {
        time_t diff_dir = labs(leituras[meio_final].timestamp - timestamp_alvo);
        if (diff_dir < menor_diferenca) {
            menor_diferenca = diff_dir;
            melhor_indice = meio_final;
        }
    }
    
    return melhor_indice;
}

void formatar_timestamp(time_t timestamp, char* buffer) {
    struct tm* tm_info = localtime(&timestamp);
    strftime(buffer, 100, "%d/%m/%Y %H:%M:%S", tm_info);
}

int main(int argc, char* argv[]) {
    if (argc != 8) {
        printf("Uso: %s <nome_sensor> <dia> <mes> <ano> <hora> <min> <seg>\n", argv[0]);
        printf("Exemplo: %s TEMP 15 03 2024 14 30 00\n", argv[0]);
        return 1;
    }
    
    char nome_sensor[MAX_SENSOR_NAME];
    int dia, mes, ano, hora, min, seg;
    
    strcpy(nome_sensor, argv[1]);
    dia = atoi(argv[2]);
    mes = atoi(argv[3]);
    ano = atoi(argv[4]);
    hora = atoi(argv[5]);
    min = atoi(argv[6]);
    seg = atoi(argv[7]);
    
    if (dia < 1 || dia > 31 || mes < 1 || mes > 12 || ano < 1970 || 
        hora < 0 || hora > 23 || min < 0 || min > 59 || seg < 0 || seg > 59) {
        printf("Erro: Data/hora inválida.\n");
        return 1;
    }
    
    time_t timestamp_alvo = converter_para_timestamp(dia, mes, ano, hora, min, seg);
    if (timestamp_alvo == -1) {
        return 1;
    }
    
    printf("Procurando leitura do sensor '%s' mais próxima de: %02d/%02d/%04d %02d:%02d:%02d\n",
           nome_sensor, dia, mes, ano, hora, min, seg);
    printf("Timestamp alvo: %ld\n\n", timestamp_alvo);
    
    Leitura* leituras;
    int num_leituras = carregar_leituras(nome_sensor, &leituras);
    
    if (num_leituras <= 0) {
        return 1;
    }
    
    printf("Arquivo carregado: %d leituras encontradas.\n", num_leituras);
    
    int indice_encontrado = busca_binaria_proxima(leituras, num_leituras, timestamp_alvo);
    
    if (indice_encontrado >= 0) {
        Leitura* leitura = &leituras[indice_encontrado];
        char data_formatada[100];
        
        formatar_timestamp(leitura->timestamp, data_formatada);
        
        printf("\n=== RESULTADO DA BUSCA ===\n");
        printf("Leitura mais próxima encontrada:\n");
        printf("Timestamp: %ld\n", leitura->timestamp);
        printf("Data/Hora: %s\n", data_formatada);
        printf("Sensor: %s\n", leitura->sensor_id);
        printf("Valor: %s\n", leitura->valor);
        
        time_t diferenca = labs(leitura->timestamp - timestamp_alvo);
        printf("Diferença: %ld segundos\n", diferenca);
        
        if (diferenca == 0) {
            printf("(Correspondência exata encontrada!)\n");
        }
    } else {
        printf("Erro: Não foi possível encontrar nenhuma leitura.\n");
    }
    
    free(leituras);
    
    return 0;
} 