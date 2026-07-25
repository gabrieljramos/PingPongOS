# PingPongOS - Núcleo Multitarefa Preemptivo em C

Implementação de um núcleo (kernel) de sistema operacional multitarefa focado em sistemas UNIX-like. Este projeto simula as operações fundamentais de um SO real, incluindo troca de contexto, escalonamento de processos e tratamento de interrupções por tempo.

---

## ⚙️ Arquitetura e Funcionalidades Implementadas

O projeto foi construído em linguagem C de forma modular, implementando os seguintes subsistemas:

* **Gestão de Tarefas (Task Management):**
    * Criação, suspensão, retomada e encerramento de tarefas.
    * Alocação dinâmica de pilhas (stacks) e manipulação de contexto em nível de usuário utilizando a biblioteca POSIX `ucontext.h` (`makecontext`, `swapcontext`, `getcontext`).
* **Escalonador Preemptivo por Prioridade (Scheduler):**
    * Dispatcher e algoritmo de escalonamento baseado em **prioridades dinâmicas com envelhecimento (aging)** para evitar inanição (*starvation*).
    * Preempção por tempo (Time-Slicing) utilizando temporizadores POSIX (`setitimer`) e tratamento de sinais de alarme (`SIGALRM`), definindo um *Quantum* para cada tarefa.
* **Gestão do Tempo e Sincronização:**
    * Implementação da função `task_sleep` para adormecer tarefas e acordá-las via *ticks* do sistema.
    * Gerenciamento de múltiplas filas de estados (Prontas, Suspensas, Dormindo) com uma estrutura genérica de listas duplamente encadeadas.

---

## 🛠️ Tecnologias e Bibliotecas Utilizadas

* **Linguagem:** C (Padrão UNIX)
* **Manipulação de Contexto:** `ucontext.h`
* **Interrupções e Sinais:** `signal.h`, `sys/time.h`
* **Gerenciamento de Memória:** Alocação manual de ponteiros de stack (`malloc`/`free`).

---

## 🚀 Estrutura de Arquivos

* `ppos_core.c`: Coração do núcleo (Dispatcher, Scheduler, Tratamento de Ticks e Gestão de Tarefas).
* `queue.c` / `queue.h`: Implementação de filas genéricas para gerenciar os estados das tarefas.
* `ppos_data.h`: Definição do TCB (Task Control Block) contendo o contexto, prioridades e métricas de uso de CPU de cada processo.
