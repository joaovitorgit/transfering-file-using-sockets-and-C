
#include"lib.h"

int main(int argc, char *argv[]) {
	//====================================VARIAVEIS=========================================
	int socket_ativo, num_pacotes, tamanho_arquivo, op, erros, check;
	long long int checksum;
	char nome_arquivo[MAX], buffer[MAX];
	ack reply;
	pacote pacote; 
	pacote.tamanho_pacote = 1;
	FILE *fp;
	struct sockaddr_in endereco_cliente, endereco_servidor, endereco_peer;
	unsigned int cliente_len, servidor_len, peer_len;
	struct timeval temporizador;
	struct hostent *host;

	//=====================================CONFIGURACAO=====================================
	// Checa os argumentos lidos
	if(argc != 2) {
		printf("Como usar : %s <server>\n", argv[0]);
		exit(1);
	}

	// Verificando o IP do servidor
	host = gethostbyname(argv[1]);
	if(host == NULL) {
		printf("Host desconhecido ou invalido!\n");
		exit(1);
	}

	// Configuração do servidor
	endereco_servidor.sin_family = host->h_addrtype;
	memcpy((char *) &endereco_servidor.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	endereco_servidor.sin_port = htons(PORTA);
	servidor_len = sizeof(endereco_servidor);

	if ((socket_ativo = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Erro na criação do socket!\n");
		exit(1);
	}

	// Configuração do cliente
	endereco_cliente.sin_family = AF_INET;
	endereco_cliente.sin_addr.s_addr = INADDR_ANY;
	endereco_cliente.sin_port = htons(PORTA);
	cliente_len = sizeof(endereco_cliente);

	if( (bind(socket_ativo, (struct sockaddr *) &endereco_cliente, cliente_len)) < 0) {
		printf("Erro no binding da porta!\n");
		exit(1);
	}

	memset(&endereco_peer, 0, sizeof(endereco_peer));
	peer_len = sizeof(endereco_peer);

	//======================================================================================
	printf("Olá! Escoha uma operação:\n");
	printf("(1) - Atualizar acervo\n");
	printf("(2) - Download de arquivo\n");
	printf("(3) - Ficar online para Upload\n");
	printf("Digite a opção: ");
	scanf("%d", &op);
	switch(op){
		//===================================ATUALIZAR BD=======================================
		case 1:
			printf("Insira os nomes dos arquivos que deseja adicionar (Digite '!' para finalizar): \n");
			while(1){
				
				scanf("%s", nome_arquivo);
				if(!strcmp(nome_arquivo, "!")) break;

				// Avisando o server que agora contem o arquivo para que o banco seja atualizado
				strcpy(buffer, "!");
				strcat(buffer, nome_arquivo);
				preparaPacote(&pacote, 1, buffer, strlen(buffer));
				check = sendto(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_servidor, servidor_len);
			}
			break;
		//=====================================DOWNLOAD=========================================
		case 2:
			printf("\nInsira o nome do arquivo: ");
			scanf("%s", nome_arquivo);
			strcpy(buffer, "+");
			strcat(buffer, nome_arquivo);
			preparaPacote(&pacote, 1, buffer, strlen(buffer));
		
			
			// Pedindo um IP para o server
			check = sendto(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_servidor, servidor_len);
			check = recvfrom(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_servidor, &servidor_len);
			
			if(strcmp(pacote.data, "!") == 0){
				printf("Não há detentores válidos\n");
			}
			else{
				// O arquivo existe
				printf("Proprietário do arquivo: %s\n", pacote.data);
				// Preenchendo informações sobre quem tem o arquivo
				host = gethostbyname(pacote.data);
				//============================== Configuração do proprietário ==========================
				endereco_peer.sin_family = host->h_addrtype;
				memcpy((char *) &endereco_peer.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
				endereco_peer.sin_port = htons(PORTA);
				peer_len = sizeof(endereco_peer);
				//======================================================================================
				// Enviando requisição com o nome do arquivo
				printf("Enviando requisicao de arquivo para %s:UDP%u\n", inet_ntoa(endereco_peer.sin_addr), ntohs(endereco_peer.sin_port));
				preparaPacote(&pacote, 1, nome_arquivo, strlen(nome_arquivo));
				check = sendto(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_peer, peer_len);
			
				// Criando um novo arquivo
				FILE *fp;
				fp = fopen(nome_arquivo, "wb");
					
				// Recebendo o arquivo do outro cliente
				printf("Recebendo o arquivo de %s: UDP%u\n", inet_ntoa(endereco_peer.sin_addr), ntohs(endereco_peer.sin_port));
				
				// Definição de temporizador,numero de pacote esperado e da quantidade
				// de erros 
				erros = 0;
				num_pacotes = 1;
				temporizador.tv_sec = 0;
				temporizador.tv_usec = 600000;
			
				if((setsockopt(socket_ativo, SOL_SOCKET, SO_RCVTIMEO, (void *) &temporizador, sizeof(temporizador))) < 0){
					perror("Erro ao definir temporizador!\n");
					exit(1);
				}
			
				while(1){
					do{
						check = recvfrom(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_peer, &peer_len);
						if(check < 0) erros++;
						else erros = 0;
						// Definimos 5 como sendo o número máximo de erros seguidos de recebimento até que um host seja considerado inativo.
						if(erros == 5){
							printf("Host retentor inativo!!\n");
							break;
						}

						// Se o numero do pacote falhar ou se o checksum estiver incorreto
						// ack = 0;
						if(pacote.tamanho_pacote && (pacote.numero_seq != num_pacotes 
							|| pacote.checksum != checkSum(pacote.data))) reply.status = 0;
						else reply.status = 1;

						check = sendto(socket_ativo, &reply, sizeof(reply), 0, (struct sockaddr *) &endereco_peer, peer_len);

					}while(pacote.tamanho_pacote && (pacote.numero_seq != num_pacotes || pacote.checksum != checkSum(pacote.data)));
					
					if(erros == 5) break;
					num_pacotes++;
					if(!pacote.tamanho_pacote) break;
					fwrite(pacote.data, 1, pacote.tamanho_pacote, fp);
				}
				
				fclose(fp);

				if(erros == 5){
					printf("Ocorreu um erro durante a transmissão\n");
					remove(nome_arquivo);
				}
				else{
					printf("Arquivo completamente recebido\n");

					// Avisando o server que agora contem o arquivo para que o banco seja atualizado
					strcpy(buffer, "!");
					strcat(buffer, nome_arquivo);
					preparaPacote(&pacote, 1, buffer, strlen(buffer));
					check = sendto(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_servidor, servidor_len);
				}
			}	
			break;
		case 3:
			//======================================UPLOAD==========================================
			// Recebendo a solicitação 
			check = recvfrom(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_peer, &peer_len);
			printf("O arquivo %s está sendo solicitado pelo usuário %s:UDP%u\n", pacote.data, inet_ntoa(endereco_peer.sin_addr), ntohs(endereco_peer.sin_port));
			
			// Abrindo o arquivo para leitura	
			fp = fopen(pacote.data, "rb");

			// Definição de temporizador e numero de pacote
			num_pacotes = 1;
			temporizador.tv_sec = 0;
			temporizador.tv_usec = 600000;
			
			if((setsockopt(socket_ativo, SOL_SOCKET, SO_RCVTIMEO, (void *) &temporizador, sizeof(temporizador))) < 0){
				perror("Erro ao definir temporizador!\n");
				exit(1);
			}
			// Lendo o arquivo
			while((tamanho_arquivo = fread(pacote.data, 1, sizeof(pacote.data), fp)) > 0){
				preparaPacote(&pacote, num_pacotes++, pacote.data, tamanho_arquivo);
				
				reply.status = 0;
				// Manda o pacote até receber um ack positivo
				do{
					check = sendto(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_peer, peer_len);
					check = recvfrom(socket_ativo, &reply, sizeof(reply), 0, (struct sockaddr *) &endereco_peer, &peer_len);
				}while(reply.status == 0);
			}
			fclose(fp);

			// Acabou o arquivo
			pacote.tamanho_pacote = 0;

			reply.status = 0;
			// Manda o pacote até receber um ack positivo
			do{
				check = sendto(socket_ativo, &pacote, sizeof(pacote), 0, (struct sockaddr *) &endereco_peer, peer_len);
				check = recvfrom(socket_ativo, &reply, sizeof(reply), 0, (struct sockaddr *) &endereco_peer, &peer_len);
			}while(reply.status == 0);

			break;
		
		default:
			break;
	}
	

	close(socket_ativo);

	return 0;
}
