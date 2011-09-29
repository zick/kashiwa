OBJS = builtin-util.o builtin.o control.o list.o num.o output.o

libbuiltin.a: $(OBJS)
	ar rcs libbuiltin.a $(OBJS)

.c.o:
	gcc -c $<

clean:
	rm *.o *.a

