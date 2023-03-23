#ifndef lib
#define lib
// Bibliotecas utilizadas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#define PORTA 15000
#define MAX 1024



// Definição da estrutura de cada pacote
typedef struct{
	// Número sequencial
	int numero_seq;	
	// Informações (1024 bytes)	
	char data[MAX];
	// Tamanho da string
	int tamanho_pacote;	
	// Checksum			
	long int checksum;			
}pacote;

typedef struct{
	// 0 = falha
	// 1 = sucesso
	int status;
}ack;


// Função que faz a soma de verificação
long int checkSum(char pacote[MAX]){
	long int total = 0;
	int aux;
	for(int i = 0; i<strlen(pacote); i++){
		aux = (int) pacote[i];
		total += (aux * i);
	}
	return total;
}

// Função que define a configuração do pacote para envio
void preparaPacote(pacote *pacote, int num, char dados[MAX], int tamanho){
	pacote->numero_seq = num;
	strcpy(pacote->data, dados);
	pacote->tamanho_pacote = tamanho;
	pacote->checksum = checkSum(dados);
}

// Função que procura o arquivo no banco de dados
void procuraArquivo(char *buffer, char *nome, char *cliente_ip){
	FILE * f;
	f = fopen("bd.txt", "r");
	char nome_arq[MAX], ip[MAX];
	// Verifica se o mesmo cliente já não tem o arquivo
	while(fscanf(f, "%s %s", nome_arq, ip) != EOF) {
		if(!strcmp(nome_arq, nome) && strcmp(ip, cliente_ip)) {
			fclose(f);
			strcpy(buffer, ip);
			return;
		}
	}
	strcpy(buffer, "!");
	fclose(f);
}


// Função que atualiza o bancao de dados
void atualizaBanco(char *nome, char *novo_ip){	
	FILE *f;	
	f = fopen("bd.txt", "r");
	char nome_arq[MAX], ip[MAX];
	// Verifica se o mesmo cliente já não inseriu este arquivo
	while(fscanf(f, "%s %s", nome_arq, ip) != EOF){
		if(!strcmp(nome_arq, nome) && !strcmp(ip, novo_ip))
			return;
	}
	fclose(f);
	// Insere o arquivo
	f = fopen("bd.txt", "a");
	fprintf(f, "%s %s\n", nome, novo_ip);
	fclose(f);
}

#endif