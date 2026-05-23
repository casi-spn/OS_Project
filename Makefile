# compiler
CC=gcc

# compiler flags
CFLAGS=-Wall -Wextra -g


# build everything
all: city_manager monitor_reports city_hub district_scorer


# city_manager
city_manager: main.o report.o permissions.o
	$(CC) $(CFLAGS) -o city_manager main.o report.o permissions.o


# compile main.c
main.o: main.c report.h permissions.h
	$(CC) $(CFLAGS) -c main.c


# compile report.c
report.o: report.c report.h
	$(CC) $(CFLAGS) -c report.c


# compile permissions.c
permissions.o: permissions.c permissions.h
	$(CC) $(CFLAGS) -c permissions.c



# monitor_reports
monitor_reports: monitor_reports.c
	$(CC) $(CFLAGS) -o monitor_reports monitor_reports.c


# city_hub
city_hub: city_hub.c
	$(CC) $(CFLAGS) -o city_hub city_hub.c


# district_scorer
district_scorer: district_scorer.c report.h
	$(CC) $(CFLAGS) -o district_scorer district_scorer.c


# clean compiled files
clean:
	rm -f *.o city_manager monitor_reports city_hub district_scorer