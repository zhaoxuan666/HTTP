service:http_server.c
	gcc $^ -o $@ -lpthread
.PHONT:clean
clean:rm service
