#!/bin/bash

file1="trace.txt"
file2="traceExmp.txt"

# Compare the two files using diff
diff "$file1" "$file2"

# Check the exit status of diff
if [ $? -eq 0 ]; then
    echo "trace files are identical."
else
    echo "trace files are different."

fi

file1="dmemout.txt"
file2="dmemoutExmp.txt"

# Compare the two files using diff
diff "$file1" "$file2"

# Check the exit status of diff
if [ $? -eq 0 ]; then
    echo "dmemout files are identical."
else
    echo "dmemout files are different."

fi

file1="cycles.txt"
file2="cyclesExmp.txt"

# Compare the two files using diff
diff "$file1" "$file2"

# Check the exit status of diff
if [ $? -eq 0 ]; then
    echo "cycles files are identical."
else
    echo "cycles files are different."

fi

file1="hwregtrace.txt"
file2="hwregtraceExmp.txt"

# Compare the two files using diff
diff "$file1" "$file2"

# Check the exit status of diff
if [ $? -eq 0 ]; then
    echo "hwregtrace files are identical."
else
    echo "hwregtrace files are different."

fi

file1="monitor.txt"
file2="monitorExmp.txt"

# Compare the two files using diff
diff "$file1" "$file2"

# Check the exit status of diff
if [ $? -eq 0 ]; then
    echo "monitor files are identical."
else
    echo "monitor files are different."

fi

file1="leds.txt"
file2="ledsExmp.txt"

# Compare the two files using diff
diff "$file1" "$file2"

# Check the exit status of diff
if [ $? -eq 0 ]; then
    echo "leds files are identical."
else
    echo "leds files are different."

fi

file1="regout.txt"
file2="regoutExmp.txt"

# Compare the two files using diff
diff "$file1" "$file2"

# Check the exit status of diff
if [ $? -eq 0 ]; then
    echo "regout files are identical."
else
    echo "regout files are different."

fi

file1="diskout.txt"
file2="diskoutExmp.txt"

# Compare the two files using diff
diff "$file1" "$file2"

# Check the exit status of diff
if [ $? -eq 0 ]; then
    echo "diskout files are identical."
else
    echo "diskout files are different."

fi

file1="display7seg.txt"
file2="display7segExmp.txt"

# Compare the two files using diff
diff "$file1" "$file2"

# Check the exit status of diff
if [ $? -eq 0 ]; then
    echo "display7seg files are identical."
else
    echo "display7seg files are different."

fi
