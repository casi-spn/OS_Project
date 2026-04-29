# AI Usage Report
Phase1:
I used ChatGPT to help with the required AI-assisted functions:

1) parse_condition()
2) match_condition()

1) The prompt I used for parse_condition() was:
"write a c function that reads a string in the format field:operator:value
and splits it into three parts, i want to store them in separate char arrays"

What AI generated: a function that uses strchr to find the positions of the colons
and then uses memcpy to copy the substrings into separate char arrays.

What I changed:
-I added a length check (strlen(input) >= sizeof(temp)) to avoid buffer overflow;
-I simplified the logic to use only basic functions (strchr, strcpy) and renamed variables.

2) The prompt I used for match_condition() was:
"write C function that checks if a report matches a condition that 
is given as field, operator and value, and the function
should compare them with the fields of a struct like severity, timestamp, category, inspector"

What AI generated: a function that checks the field using strcmp
for numeric fields (severity, timestamp), converts the value using atol
and compares it with the corresponding field in the Report struct using the operator (==, !=, >, <, >=, <=).

What I changed:
-I adapted the function to my Report structure;
-I reused helper functions (compare_int, compare_string) to keep the code as simple as possible;
-I added that it returns 0 for unsupported fields.

Phase2:


I used ChatGPT to help with the process and signal parts:

1) remove_district using fork() and execlp()
2) monitor_reports using sigaction()


1) For remove_district, the AI suggested creating a child process with fork()
and replacing it with the external command rm -rf using execlp() 
I added role checking so only the manager can remove a district, 
and I made sure the corresponding active_reports-* symbolic link is also removed.

2) For monitor_reports, the AI helped structure the program 
so it creates .monitor_pid, waits for signals, handles SIGUSR1
by printing a message, and handles SIGINT by deleting .monitor_pid before exiting. 
I used sigaction() instead of signal(),as stated in the requirement.


What I learned:
I learned how to use fork(), execlp(), waitpid(), sigaction()kill().