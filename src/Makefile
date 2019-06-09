VPATH=API:BufferManager:CatalogManager:Interpreter:RecordManager:IndexManager
CPP=g++ -std=c++17
main: index_manager.o BufferManager.o util.o
	$(CPP) -o $@ $^
.cpp.o: 
	$(CPP) -c $<
clean:
	rm -f *.o