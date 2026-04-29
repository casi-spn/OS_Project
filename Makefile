CC=gcc
CFLAGS=-Wall -Wextra -g

all: city_manager monitor_reports

city_manager: main.o report.o permissions.o
	$(CC) $(CFLAGS) -o city_manager main.o report.o permissions.o

main.o: main.c report.h permissions.h
	$(CC) $(CFLAGS) -c main.c

report.o: report.c report.h
	$(CC) $(CFLAGS) -c report.c

permissions.o: permissions.c permissions.h
	$(CC) $(CFLAGS) -c permissions.c

monitor_reports: monitor_reports.c
	$(CC) $(CFLAGS) -o monitor_reports monitor_reports.c

clean:
	rm -f *.o city_manager monitor_reports