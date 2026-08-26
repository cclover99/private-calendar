ifeq ($(OS),Windows_NT)
MKDIR = if not exist bin mkdir bin
EXE = .exe
else
MKDIR = mkdir -p bin
EXE =
endif

main:
	@$(MKDIR)
	g++ ./src/main.cpp -o ./bin/main$(EXE)