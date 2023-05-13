# LAB7 / TASK 2
## Competitive access to a file

## Instructions

1. At first go to folder
```
cd /path/to/lab7/task2/scripts
```
2. Then run build script
```
bash build_competition.sh
```
3. After that generate file and enter path to file and it's size
```
bash run_genfile.sh
...
<some output>
...
Enter path to file
/path/to/file
Enter size
10
```
4. Finally, run the last script and enter path to generated file
```
bash run_competition.sh
...
<some output>
...
Enter path to file
/path/to/file
```
# Notes

If print_lock(fd, no) fails just delete it. It works perfectly without that function
