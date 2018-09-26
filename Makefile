CFLAGS=-g -Wall $(COPTS)
TARGETS=webserver
LIBS=webserver_connection.o webserver_parser.o spider/webtorm.o spider/webtorm_browser.o spider/webtorm_connection.o spider/webtorm_parser.o spider/webtorm_log.o spider/webtorm_db.o
LDLIBS=-lpthread -lmysqlclient


all: $(TARGETS) 

webserver: webserver.o $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:  
	rm -f *.o; rm -f spider/*.o
