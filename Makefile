make: iomake.cpp
	g++ -std=c++0x iomake.cpp -o iomake 
clean:
	rm -f iomake *~