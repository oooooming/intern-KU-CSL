GENCODE_SM61 = -gencode=arch=compute_61,code=\"sm_61,compute_61\"
GENCODE_SM70 = -gencode=arch=compute_70,code=\"sm_70,compute_70\"
#GENCODE_SMxx = -gencode=arch=compute_xx,code=\"sm_xx,compute_xx\"

CXX = g++
CXXFLAGS = -Wall -std=c++0x
TARGET = glife 
OBJ = glife.o
SRC = glife.cpp
LIBS = -pthread -L/usr/local/cuda/lib64 -lcuda -lcudart
.PHONY : $(TARGET) $(OBJ)

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(SRC) $(LIBS) 

$(OBJ) :
	/usr/local/cuda/bin/nvcc -c $(GENCODE_SM61) $(GENCODE_SM70) glife.cu

clean :
	rm *.o glife 
