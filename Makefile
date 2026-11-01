# Makefile del TP (lo usa correr_simulacion.bat con mingw32-make).
# Compila lo mismo que el proyecto de Code::Blocks pero deja los ejecutables en bin/
# con los nombres que espera el .bat.
#
#   mingw32-make                  compila todo
#   mingw32-make generador sim_p1 sim_p2
#   mingw32-make test_p1 test_p2  lote de pruebas de la cache (no es parte de la entrega)
#   mingw32-make clean

CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -Ilista

# en Windows los ejecutables llevan .exe (el .bat los llama bin\generador.exe, etc.)
ifeq ($(OS),Windows_NT)
EXE = .exe
else
EXE =
endif

GEN    = bin/generador$(EXE)
SIM_P1 = bin/sim_p1$(EXE)
SIM_P2 = bin/sim_p2$(EXE)
TEST_P1 = bin/test_p1$(EXE)
TEST_P2 = bin/test_p2$(EXE)

FUENTES_P1 = parte1/lrucache1.c lista/lista.c
FUENTES_P2 = parte2/lrucache2.c lista/listaDoble.c arbol/arbol_bin_busq.c

HEADERS_P1 = parte1/lrucache.h lista/lista.h
HEADERS_P2 = parte2/lrucache.h lista/listaDoble.h arbol/arbol_bin_busq.h

WRAP = -Wl,--wrap=malloc -Wl,--wrap=free

.PHONY: todo generador sim_p1 sim_p2 test_p1 test_p2 clean

todo: generador sim_p1 sim_p2 test_p1 test_p2

generador: $(GEN)
sim_p1: $(SIM_P1)
sim_p2: $(SIM_P2)
test_p1: $(TEST_P1)
test_p2: $(TEST_P2)

# solo se crea si no existe (mkdir anda igual en cmd y en sh)
bin:
	mkdir bin

$(GEN): parte3/generador.c parte3/tweener.h | bin
	$(CC) $(CFLAGS) -g -o $@ parte3/generador.c

# el mismo simulador.c se compila contra la Parte 1 o la Parte 2 segun el -I
$(SIM_P1): parte3/simulador.c parte3/tweener.h $(FUENTES_P1) $(HEADERS_P1) | bin
	$(CC) $(CFLAGS) -O2 -Iparte1 -o $@ parte3/simulador.c $(FUENTES_P1)

$(SIM_P2): parte3/simulador.c parte3/tweener.h $(FUENTES_P2) $(HEADERS_P2) | bin
	$(CC) $(CFLAGS) -O2 -Iparte2 -Iarbol -o $@ parte3/simulador.c $(FUENTES_P2)

$(TEST_P1): pruebas/test_lrucache.c $(FUENTES_P1) $(HEADERS_P1) | bin
	$(CC) $(CFLAGS) -g -Iparte1 -o $@ pruebas/test_lrucache.c $(FUENTES_P1) $(WRAP)

$(TEST_P2): pruebas/test_lrucache.c $(FUENTES_P2) $(HEADERS_P2) | bin
	$(CC) $(CFLAGS) -g -Iparte2 -Iarbol -o $@ pruebas/test_lrucache.c $(FUENTES_P2) $(WRAP)

ifeq ($(OS),Windows_NT)
clean:
	-del /q bin\generador.exe bin\sim_p1.exe bin\sim_p2.exe bin\test_p1.exe bin\test_p2.exe
else
clean:
	rm -f $(GEN) $(SIM_P1) $(SIM_P2) $(TEST_P1) $(TEST_P2)
endif
