Git is love, Git is life.


Servidor de Backups:
2 Programas: Servidor (sobusrv)  + Cliente (sobucli)
	- Impedir acesso direto aos ficheiros;
	- Comprimir Dados;
	- Eliminar duplicados;

sobusrv:
	Processo pai aceita conexões, e manda o trabalho para processos filhos.
	Comunicação entre processos filhos e clientes por pipes com nome.
	Guarda tudo numa diretoria /home/user/.Backup/
	Respostas aos pedidos por sinais.
	Ações concorrentes sempre <=5.

sobucli:
	Estabelece conexões com o servidor e faz o que tem a fazer.
	Cliente não lê nem escreve ficheiros. Só faz pedidos.
