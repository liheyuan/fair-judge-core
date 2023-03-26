# Fair Judge Core
An lightweight OJ Core that runs under Linux

## Usage
```
fjcore -p <abs_path_bin> -c <bin_full_cmd> -i <input_file> -o <output_file> -a <answer_file, skip check if not provided>-t <time_limit_sec, default 1> -m <mem_limit_mb, default 8> -f <file_limit_mb, default 16>
```

## Example
Note: You should create 3 files with same content before run the following example: input.txt, output.txt, answer.txt
* Correct / Accepted / AC
```
fjcore -p /usr/bin/cat -c "cat" -i ./input.txt -o ./output.txt -a ./answer.txt
```
* Wrong Answer / WA
```
fjcore -p /usr/bin/sleep -c "sleep 1" -i ./input.txt -o ./output.txt -a ./answer.txt -t 2
* Presentation Error / PE (add an extra line to the end of file of answer.txt)
```
fjcore -p /usr/bin/cat -c "cat" -i ./input.txt -o ./output.txt -a ./answer.txt -t 2
```
* Time Limit Exceed / TLE
```
fjcore -p /usr/bin/sleep -c "sleep 2" -i ./input.txt -o ./output.txt ### memory_kb not show?-a ./answer.txt
```
* Memory Limit Exceed / MLE (need a test file that allow more memory than 16mb)
```
fjcore -p ./32mb.bin -c "32mb.bin" -i ./input.txt -o ./output.txt -a ./answer.txt -m 16
```

## FAQ
### Check Priority
ERR_CHILD_EXIT_NONZERO > ERR_CHILD_EXIT_ABNORMALLY > TLE , MLE > WA , PE
