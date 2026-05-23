City Infrastructure Reporting and Monitoring System
Brief Project Description

This project is a UNIX-based city infrastructure issue reporting and monitoring system written in C.
It simulates how city inspectors and managers handle infrastructure problems.

The project is divided into multiple phases and demonstrates the use of:

-file systems
-binary files
-symbolic links
-Unix permissions
-signals
-process management
-pipes
-fork/exec
-dup2 redirection
-inter-process communication

Each district has its own directory containing reports, configuration files, and logs.

The system includes:

-city_manager → manages reports and districts
-monitor_reports → background monitor process
-city_hub → interactive command-line hub
-district_scorer → computes inspector workload scores

The main programs are:
-city_manager.c → handles report management and district operations
-monitor_reports.c → runs as a background process to monitor new reports and handle shutdown signals
-city_hub.c → provides an interactive interface for users to view reports and scores
-district_scorer.c → calculates workload scores for inspectors based on report data

city_manager commands:
./city_manager --role inspector --user x --add downtown ->adds a new report for the downtown district by inspector x
./city_manager --role inspector --user x --list downtown ->lists all reports for the downtown district
./city_manager --role inspector --user x --view downtown 1 ->displays full information about the report
./city_manager --role manager --user y --remove_report downtown 1 ->removes the specified report
./city_manager --role manager --user Alice --remove_district downtown ->removes the entire downtown district and all its reports
./city_manager --role manager --user Alice --update_threshold downtown 3 ->updates the severity threshold for the downtown district to 3
Filtering reports:
./city_manager --role inspector --user x --filter downtown severity:>=:2
./city_manager --role inspector --user x --filter downtown category:==:road
./city_manager --role inspector --user x --filter downtown inspector:==:x
./city_manager --role inspector --user Casi --filter downtown severity:>=:2 category:==:road

./monitor_reports → runs the background monitor process that listens for new reports and handles shutdown signals

./city_hub → starts the interactive command-line hub for users to view reports and scores
start_monitor → starts the monitor_reports process in the background and displays monitor messages through pipes.
calculate_scores downtown uptown → calculates and displays inspector workload scores for the downtown and uptown districts using district_scorer.
exit → exits the city_hub interface

SIGNALS:
SIGUSR1 → notify monitor about new report
SIGINT → stop monitor

ls -l active_reports-* → lists active report files with their permissions and timestamps(symbolic links to the latest report files)

