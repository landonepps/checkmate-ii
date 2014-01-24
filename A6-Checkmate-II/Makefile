CXXFLAGS = -g -I/usr/X11R6/include -I../lib

OBJS = Chess.o Mesh.o Geometry.o

TARGETS = chess

all: $(TARGETS)

$(TARGETS) : % : $(OBJS)
	g++ -o $@ $(OBJS) -L/usr/X11R6/lib -lglut -lGLU -lGL

%.o: %.cpp
	g++ $(CXXFLAGS) -c $< -o $@

clean:  
	-rm -f *.o $(TARGETS)
