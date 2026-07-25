/*

Nome: Gabriel Justus Ramos

GRR: 20232348

*/

#include "ppos.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#define PRONTA 0
#define EXECUTANDO 1
#define TERMINADA 2
#define SUSPENSA 3

#define QUANTUM 20                                                                       //Tamanho do QUANTUM

int system_time = 0;                                                                     //Guarda o tempo atual
int task_ticks = 0;                                                                      //Guarda a qntd de tick na tarefa atual
int preempt_disable = 0;                                                                 //Marcador para desabilitar ou nao uma tarefa


task_t main_task;                                                                        //Guarda a tarefa main
task_t *current_task;                                                                    //Guarda a tarefa atual
task_t dispatcher_task;                                                                  //Guarda a tarefa do dispatcher
task_t *ready_tasks = NULL;                                                              //Fila de tarefas prontas para execucao
task_t *sleeping_tasks = NULL;                                                           //Fila de tarefas dormindo
task_t *prev_task;                                                                       //Guarda tarefa anterior
int current_id = 0;                                                                      //Contador para os IDs das tarefas
int user_tasks = 0;                                                                      //Guarda a qntd de tarefas



/*

    TRAB 8: TAREFAS DORMINDO

*/

void check_sleeping_tasks() {

    if (!sleeping_tasks)                                                                 //Se a fila de tarefas dormindo eh nula, retorna
        return;

    task_t *aux = sleeping_tasks;                                                        //Variavel auxiliar para navegar pela fila

    do {                                                                                 //Enquanto nao chegar no final da fila
        task_t *next_aux = aux->next;                                                    //Guarda o proximo elemento da fila
        if (aux->wakeup_time <= systime()) {                                             //Se o tempo de sono da tarefa for menor ou igual ao tempo atual
            task_awake(aux, &sleeping_tasks);                                            //Acorda a tarefa e remove ela da fila de tarefas dormindo
        }
        if (!sleeping_tasks) break;                                                      //Se a fila de tarefas dormindo ficou vazia, sai do loop
        aux = next_aux;                                                                  //Avanca para a proxima tarefa
    } while (aux != sleeping_tasks);

}

void task_sleep (int t) {

    if (t <= 0 || !current_task)                                                         //Se o tempo eh 0 ou a tarefa atual eh nula, retorna
        return;

    preempt_disable++;                                                                   //Desabilita a preempção para evitar que a tarefa atual seja interrompida

    current_task->wakeup_time = systime() + t;                                           //Marca o tempo de sono da tarefa atual

    queue_remove((queue_t**)&ready_tasks, (queue_t*)current_task);                       //Remove a tarefa atual da fila de prontas
    queue_append((queue_t**)&sleeping_tasks, (queue_t*)current_task);                    //Adiciona a tarefa na fila de tarefas dormindo

    current_task->status = SUSPENSA;                                                     //Marca a tarefa atual como suspensa

    preempt_disable--;                                                                   //Habilita a preempção novamente

    task_yield();                                                                        //Faz um yield para o dispatcher

}

/*

    TRAB 7: TAREFAS SUSPENSAS

*/

void task_suspend (task_t **queue) {


    if (!queue || !current_task)                                                         //Se a fila eh nula ou a tarefa atual eh nula, retorna
        return;

    queue_remove((queue_t**)&ready_tasks, (queue_t*)current_task);                       //Remove a tarefa atual da fila de prontas

    current_task->status = SUSPENSA;                                                     //Marca a tarefa atual como suspensa


    queue_append((queue_t**)queue, (queue_t*)current_task);                              //Adiciona a tarefa atual na fila de suspensao

    task_yield();

}

void task_awake (task_t *task, task_t **queue) {

    if (!task)                                                                           //Se a tarefa eh nula, retorna
        return;

    if (queue)              
        queue_remove((queue_t**)queue, (queue_t*)task);                                  //Remove a tarefa da fila de suspensao

    // Muda o status
    task->status = PRONTA;                                                               //Marca a tarefa como pronta

    // Coloca na fila de prontas
    queue_append((queue_t**)&ready_tasks, (queue_t*)task);                               //Adiciona a tarefa na fila de prontas

}

int task_wait(task_t *task) {

    if (!task)                                                                           //Se a tarefa eh nula, retorna  
        return -1;

    if (task->status == TERMINADA)                                                       //Caso a tarefa ja tenha terminado, retorna o exit_code
        return task->exit_code;

    if (!task->wait_queue)                                                               //Caso a fila de espera da tarefa seja nula, inicializa ela   
        task->wait_queue = NULL;        

    task_suspend(&(task->wait_queue));                                                   //Suspende a tarefa atual e adiciona na fila de espera da tarefa

    return task->exit_code;
}



/*

    TRAB 5/6: PREEMPCAO POR TEMPO/CONTABILIZACAO


*/

unsigned int systime() {
    return system_time;                                                                  //Retorna o valor atual do relogio
}

void tick_handler(int signum) {

    system_time++;                                                                       //Incrementa o valor do relogio
    if (!current_task || current_task->is_system_task)                                   //Caso a tarefa atual seja invalida ou de sistema retorna
        return;
    if (preempt_disable > 0)                                                             //A tarefa nao pode ser interrompida
        return;

    current_task->cpu_time++;                                                            //Incrementa o tempo de uso da CPU da tarefa atual
    task_ticks--;                                                                        //Decrementa seus ticks (tempo restante)
    if (task_ticks <= 0)                                                                 //Caso a tarefa tenha excedido seu tempo, yield
        task_yield();                                                       

}

/*

    TRAB 4: ESCALONADOR COM PRIORIDADE

*/
void task_setprio(task_t *task, int prio) {

    if ((prio > 20) || (prio < -20))                                                     //Se o valor da prioriade eh invalido, retorna
        return;
    if (!task) {                                                                         //Se task eh nulo, ajusta a task atual
        current_task->stat_prio = prio;
        current_task->dyn_prio = prio;
    }
    else {                                                                               //Ajusta a prioridade de task
        task->stat_prio = prio;
        task->dyn_prio = prio;
    }

}

int task_getprio(task_t *task) {

    if (!task)                                                                           //Se task eh nulo, retorna a prioridade estatica da task tual
        return current_task->stat_prio;
    return task->stat_prio;                                                              //Retorna a prioridade estatica de task

}


task_t* scheduler() {

    if (!ready_tasks)                                                                    //Se a fila de tarefas esta vazia, retorna nulo
        return NULL;

    task_t *aux = ready_tasks->next;                                                     //Variavel para navegar pela fila
    task_t *smallest_prio = ready_tasks;                                                 //Variavel para guardar a proxima tarefa (menor prioridade)

    while (aux != ready_tasks) {                                                         //Busca a menor prioridade na fila de tarefas
        if (aux->dyn_prio < smallest_prio->dyn_prio)                                     //Se a prioridade da task atual na fila eh menor do que a menor, troca
            smallest_prio = aux;
        aux = aux->next;
    }

    aux = ready_tasks;                                                                   //Garante que aux retorna para o comeco da fila

    do {
        if (aux != smallest_prio)                                                        //Envelhece todas as tarefas que nao foram escolhidas 
            aux->dyn_prio--;
        else 
            aux->dyn_prio = aux->stat_prio;                                              //Retorna a tarefa escolhida para sua prioridade inicial
        aux = aux->next;
    } while (aux != ready_tasks);

    return smallest_prio;                                                                //Retorna a tarefa escolhida

}

/*
 
    TRAB 3: DISPATCHER

 */

//FCFS, devolve o primeiro elemento da fila<F9><F9>
/*task_t* scheduler() {
    return ready_tasks;
}*/

void dispatcher() {

    queue_remove((queue_t**) &ready_tasks,(queue_t*)&dispatcher_task);                   //Remove o dispatcher da fila p/ evitar trocar para o mesmo contexto

    while (user_tasks > 0) {
        check_sleeping_tasks();                                                          //Verifica se ha tarefas dormindo que devem ser acordadas
        task_t *next_task = scheduler();                                                 //Salva a proxima tarefa
        if (next_task) {
            next_task->status = EXECUTANDO;                                              //Marca seu status p/ EXECUTANDO
            task_switch(next_task);                                                      //Executa a tarefa
            if (next_task->status == TERMINADA) {
                queue_remove((queue_t**)&ready_tasks, (queue_t*) next_task);             //Remove ela da fila
                free(next_task->context.uc_stack.ss_sp);                                 //Caso a tarefa tenha terminado, libera a stack
            }
            else {
                next_task->status = PRONTA;                                              //Caso contrario, marca ela como pronta e add na fila novamente
            }
        }
    }

    task_exit(0);

}

void task_yield() {

    if (current_task->status == PRONTA)
        queue_append((queue_t**)&ready_tasks, (queue_t*)current_task);

    task_switch(&dispatcher_task);                                                       //Retorna para o disptacher
}

/*

    TRAB 2: GESTAO DE TAREFAS

*/

void ppos_init() {

    setvbuf (stdout, 0, _IONBF, 0);

    getcontext(&main_task.context);                                                      //Salva o contexto da tarefa Main

    main_task.id = current_id;                                                           //Marca o ID da tarefa main, que sempre sera 0
    current_id++;                                                                        //Incrementa o valor do ID p/ a prox. tarefa

    main_task.status = EXECUTANDO;                                                       //Marca a tarefa como EXECUTANDO

    main_task.is_system_task = 1;                                                        //Marca a tarefa main como tarefa de sistema
    main_task.exec_start = systime();                                                    //Guarda seu tempo inicial de execucao
    main_task.cpu_time = 0;                                                              //Tempo inicial de uso de CPU = 0
    main_task.activations = 1;                                                           //Ativacoes iniciais = 0

    current_task = &main_task;                                                           //Marca a tarefa atual como a main

    task_init(&dispatcher_task, dispatcher, NULL);                                       //Cria a task do dispatcher
    dispatcher_task.is_system_task = 1;

    struct sigaction action;
    struct itimerval timer;

    action.sa_handler = tick_handler;                                                    //Define a funcao tick_handler para tratar SIGINT
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGALRM, &action, 0);

    timer.it_value.tv_usec = 1000;                                                       //Temporizador de 1 segundo
    timer.it_value.tv_sec = 0;
    timer.it_interval.tv_usec = 1000;                                                    //Temporizador de 1 segundo
    timer.it_interval.tv_sec = 0;
    setitimer(ITIMER_REAL, &timer, 0);

}

int task_init(task_t *task, void (*start_routine)(void *),  void *arg) {

    if (!task)                                                                           //Caso a tarefa seja invalida retorna
        return -1;

    getcontext(&(task->context));                                                        //Pega o contexto da tarefa

    char *stack;
    if (!(stack = malloc(2*SIGSTKSZ)))                                                   //Aloca a stack e confere se a alocacao foi um sucesso, se nao, retorna
        return -1;

    task->context.uc_stack.ss_sp = stack;                                                //Salva o ponteiro da stack
    task->context.uc_stack.ss_size = 2*SIGSTKSZ;                                         //Salva o tamanho da stack
    task->context.uc_stack.ss_flags = 0;                                                 //Marca a stack como 0
    task->context.uc_link = 0;                                                           //Marca o contexto de retorno como 0

    makecontext(&(task->context), (void*) start_routine, 1, arg);                        //Cria o contexto da terefa

    task->id = current_id;                                                               //Salva o ID da tarefa
    current_id++;                                                                        //Incrementa o valor do ID p/ a prox. tarefa
    task->status = PRONTA;                                                               //Marca a tarefa como PRONTA

    task->next = NULL;                                                                   //Marca seus ponteiro como NULL p/ fazer append
    task->prev = NULL;
    queue_append((queue_t**) &ready_tasks, (queue_t*) task);                             //Faz append da tarefa na fila
    user_tasks++;                                                                        //Incrementa a qntd de tarefas
    
    task->wait_queue = NULL;                                                             //Marca a fila de espera como NULL
    task->exit_code = 0;                                                                 //Marca o codigo de saida como 0
    task->stat_prio = 0;
    task->dyn_prio = 0;
    task->is_system_task = 0;
    task->exec_start = systime();
    task->cpu_time = 0;
    task->activations = 0;

    return task->id;                                                                     //Retorna o ID da task

}

int task_switch(task_t *task) {

    if (!task)                                                                           //Caso a tarefa seja invalida retorna
        return -1;

    task_t *temp_task = current_task;                                                    //Variavel temporaria para salvar a tarefa atual
    prev_task = current_task;
    current_task = task;                                                                 //Troca a tarefa atual p/ a nova tarefa

    if (!current_task->is_system_task)
        task_ticks = QUANTUM;

    current_task->activations++;

    return swapcontext(&(temp_task->context), &(task->context));                         //Faz a troca de contexto p/ a nova tarefa

}

int task_id() {

    return current_task->id;                                                             //Retorna o ID da tarefa

}

void task_exit(int exit_code) {

    current_task->status = TERMINADA;                                                    //Marca a tarefa como TERMINADA
    current_task->exec_end = systime();                                                  //Guarda seu tempo de fim de execucao

    printf("Task %d exit: execution time %u ms, processor time %u ms, %u activations\n",
        current_task->id,
        current_task->exec_end - current_task->exec_start,
        current_task->cpu_time,
        current_task->activations);                                                      //Imprime as informacoes pedidas sobre a tarefa

    user_tasks--;                                                                        //Diminui a qntd total de tarefas

    current_task->exit_code = exit_code;                                                 //Guarda o codigo de saida da tarefa

    while (current_task->wait_queue) {                                                   //Enquanto houver tarefas esperando por ela
        task_t *waiting = current_task->wait_queue;                                      //Pega a primeira tarefa da fila de espera
        task_awake(waiting, &current_task->wait_queue);                                  //Acorda a tarefa
    }

    task_switch(&dispatcher_task);                                                       //Retorna para o dispatcher

}
