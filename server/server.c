#include "lib.h"

int main(int argc, char *argv[]) {
	//====================================VARIAVEIS=========================================
	int socket_ativo, check;
	struct sockaddr_in endereco_servidor, endereco_cliente;
	unsigned int servidor_len, cliente_len;
	char buffer[MAX], answer[MAX];
	pacote pacote;
	ack reply;

	//===================================CONFIGURACAO=======================================
	// Preenchendo informações sobre o servidor
	endereco_servidor.sin_family = AF_INET;
	endereco_servidor.sin_addr.s_addr = INADDR_ANY;
	endereco_servidor.sin_port = htons(PORTA);
	servidor_len = sizeof(endereco_servidor);

	if ( (socket_ativo = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("Erro na criação do socket!");
		exit(1);
	}
	if (bind(socket_ativo, (struct sockaddr *)&endereco_servidor, servidor_len) < 0 ){
		perror("Erro no bind!");
		exit(1);
	}
	//======================================================================================

	printf("O servidor está online !\nAguardando conexoes...\n");

	while(1) {
		memset(&endereco_cliente, 0, sizeof(endereco_cliente));
		cliente_len = sizeof(endereco_cliente);

		// Recebendo dados
		check = recvfrom(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_cliente, &cliente_len);
	
		printf("Recebi de %s:UDP%u : %s \n", inet_ntoa(endereco_cliente.sin_addr), ntohs(endereco_cliente.sin_port), pacote.data);

		// Operação de atualizar o BD
		if(pacote.data[0] == '!'){
			pacote.data[0] = '+';
			atualizaBanco(pacote.data, inet_ntoa(endereco_cliente.sin_addr));
		}
		// Operação de retornar o IP do detentor do arquivo
		else if(pacote.data[0] == '+'){			
			procuraArquivo(answer, pacote.data, inet_ntoa(endereco_cliente.sin_addr));
			strcpy(pacote.data, answer);
			sendto(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_cliente, cliente_len);
		}
	}
	return 0;
}


