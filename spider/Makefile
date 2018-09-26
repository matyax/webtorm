CFLAGS=-g -Wall $(COPTS)
TARGETS=webtorm
LIBS=webtorm_browser.o webtorm_connection.o webtorm_parser.o webtorm_log.o webtorm_db.o
LDLIBS=-lpthread -lmysqlclient

all: $(TARGETS) 

webtorm: webtorm.o $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS) 

clean:  
	rm -f *.o
