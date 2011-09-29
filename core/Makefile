OBJS = function.o heap.o stack.o symbol.o

libcore.a: $(OBJS)
	ar rcs libcore.a $(OBJS)

.c.o:
	gcc -c $<

clean:
	rm *.o *.a

