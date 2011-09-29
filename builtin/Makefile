OBJS = builtin.o

libbuiltin.a: $(OBJS)
	ar rcs libbuiltin.a $(OBJS)

.c.o:
	gcc -c $<

clean:
	rm *.o *.a

