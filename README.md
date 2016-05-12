Git is love, Git is life.


Servidor de Backups:
2 Programas: Servidor (sobusrv)  + Cliente (sobucli)
	- Impedir acesso direto aos ficheiros;
	- Comprimir Dados;
	- Eliminar duplicados;

sobusrv:
	Processo pai aceita conexões, e manda o trabalho para processos filhos.
	Comunicação entre processos filhos e clientes por pipes com nome.

sobucli:
	Estabelece conexões com o servidor e faz o que tem a fazer.
