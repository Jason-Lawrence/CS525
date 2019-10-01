LINK_TARGET1 = test_assign4
LINK_TARGET2 = test_expr

OBJS1 = \
  storage_mgr.o \
  buffer_mgr.o  \
  buffer_mgr_stat.o \
  record_mgr.o \
  btree_mgr.o \
  rm_serializer.o \
  expr.o \
  dberror.o \
  test_assign4_1.o

OBJS2 = \
  storage_mgr.o \
  buffer_mgr.o  \
  buffer_mgr_stat.o \
  record_mgr.o \
  btree_mgr.o \
  rm_serializer.o \
  expr.o \
  dberror.o \
  test_expr.o

REBUILDABLES = $(OBJS1) $(OBJS2) $(LINK_TARGET1) $(LINK_TARGET2)

clean : 
	rm -f $(REBUILDABLES)
	echo Clean done

all : $(LINK_TARGET1) $(LINK_TARGET2)
	echo ALL done

$(LINK_TARGET1) : $(OBJS1)
	gcc -g -o $@ $^

$(LINK_TARGET2) : $(OBJS2)
	gcc -g -o $@ $^

%.o : %.c
	gcc -g -o $@ -c $<

storage_mgr.o : storage_mgr.h storage_mgr.c

buffer_mgr.o : buffer_mgr.h buffer_mgr.c

buffer_mgr_stat.o : buffer_mgr_stat.h buffer_mgr.h buffer_mgr_stat.c

record_mgr.o : record_mgr.c record_mgr.h buffer_mgr.h storage_mgr.h

btree_mgr.o : btree_mgr.c btree_mgr.h record_mgr.h buffer_mgr.h storage_mgr.h 

rm_serializer.o : dberror.h tables.h record_mgr.h rm_serializer.c

expr.o : expr.c expr.h dberror.h record_mgr.h tables.h

dberror.o : dberror.c dberror.h

test_assign_4_1.o : test_assign4_1.c dberror.h storage_mgr.h test_helper.h buffer_mgr.h buffer_mgr_stat.h

test_expr.o : test_expr.c dberror.h expr.h tables.h test_helper.h record_mgr.h btree_mgr.h

