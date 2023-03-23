
> Implementação em linguagem C da transferência de arquivos de vídeo ou música entre sistemas finais remotos.

### Preparativos:
Este procedimento é opcional
* Há uma pasta chamada 'arquivos para teste'. Ela so existe para facilitar os 
testes do programa. Use-a copiando um arquivo dela para a pasta cliente em uma das maquinas virtuais. 
Depois, use esse arquivo como teste enviando-o para o banco de dados. Por fim, execute o cliente em outra maquina virtual e faça o download desse arquivo. 

### Instruções
* Como compilar:
- gcc cliente.c -o cliente
- gcc server.c -o server

* Como executar o server:
	* Execute ./server
* Como executar o cliente:
	* Executar ./cliente <IP do servidor> (Use o comando hostname -I para obter o IP)
	* A operação 1 atualiza o banco de dados do servidor, informando que ele contém o arquivo especificado(Inicialmente o banco de dados vai estar vazio).
	* A operação 2 permite fazer o download de arquivos. Para isso o programa envia uma solicitação com o nome do arquivo desejado para o servidor, o qual retorna o IP de um cliente que possui o arquivo. Depois disso o programa envia uma solicitação para o cliente detentor.
	* A operação 3 faz com o cliente fique disponível para requisições e para envio dos arquivos.
		
### Observações
* Os testes(Com servidor e dois clientes) foram compilados e executados em três maquinas virtuais Linux, com IPs distintos e conectados a uma mesma rede. 
* Configuração das maquinas virtuais:
	* Linux Mint 20.2 Cinnamon
	* gcc version 9.3.0


