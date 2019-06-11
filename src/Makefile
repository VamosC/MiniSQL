VPATH=API:BufferManager:CatalogManager:Interpreter:RecordManager:IndexManager
CPP=g++ -std=c++17
main: main.o index_manager.o BufferManager.o util.o api.o catalogmanager.o interpreter.o \
      RecordManager.o
	$(CPP) -o $@ $^
.cpp.o: 
	$(CPP) -c $<
clean:
	rm -f *.o