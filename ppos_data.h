// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		     // ponteiros para usar em filas
  int id ;				                     // identificador da tarefa
  ucontext_t context ;			           // contexto armazenado da tarefa
  short status ;			                 // pronta, rodando, suspensa, ...
  int stat_prio;                       // prioridade estática
  int dyn_prio;                        // prioridade dinâmica 
  int is_system_task;                  // 1 se for tarefa de sistema, 0 se for tarefa de usuário
  int activations;                     // número de ativações da tarefa 
  int exec_start;                      // tempo de início da execução  
  int exec_end;                        // tempo de fim da execução
  int wakeup_time;                       // tempo de despertar da tarefa (para tarefas dormindo)  
  int cpu_time;                        // tempo de uso da CPU
  int exit_code;                       // código de saída da tarefa
  struct task_t *wait_queue;           // fila de espera para tarefas bloqueadas
  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif

