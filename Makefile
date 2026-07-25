# Makefile para o projeto P4

# Compilador e flags
CC = gcc
CFLAGS = -Wall -g

# Arquivos fonte e headers
SRCS = ppos_core.c queue.c main.c
HEADERS = ppos.h ppos_data.h queue.h
OBJS = $(SRCS:.c=.o)

# Nome do executável
EXEC = ppos

# Regra padrão
all: $(EXEC)

# Geração do executável
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

# Regra para remover arquivos de objeto e binário
clean:
	rm -f $(OBJS) $(EXEC)

# Garante que headers são considerados dependências
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

