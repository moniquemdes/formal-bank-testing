## Objetivo do Trabalho

Este trabalho consiste em testar e corrigir um software em C++ que implementa um sistema bancário, utilizando uma especificação formal escrita em Quint como referência.

O código principal do banco possui bugs que precisam ser identificados e corrigidos com base nos testes gerados a partir do modelo formal. Para isso, foi desenvolvido um programa de teste (em C++) que:

- Lê 10.000 execuções simuladas (traces) geradas pelo modelo Quint, armazenadas em arquivos JSON.
- Para cada trace, simula as ações do banco (depósito, saque, transferência, compra e venda de investimentos) executando as funções correspondentes do código do banco.
- Compara o estado resultante do banco (saldos, investimentos, etc.) após cada ação com o estado esperado fornecido pelo modelo.
- Verifica se os erros retornados pelas funções estão alinhados com os esperados.
- Exibe mensagens detalhadas para ajudar a identificar divergências entre o comportamento do código e o modelo formal.

O objetivo é usar esses testes para localizar discrepâncias, corrigir o código e garantir que o sistema bancário implementado esteja em conformidade com a especificação formal, aumentando assim a confiabilidade do software.
