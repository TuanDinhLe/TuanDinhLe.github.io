##Running Instructions##

1. To make the programs, run "make".
To run the executables of:
spin-lock: ./spin-lock
atomic_lock: ./atomic_lock
lock-really: ./lock-really
mutex-lock: ./mutex-lock
condition-vars: ./condition-vars (yes "HeLlO" | ./condition-vars for testing).

2. Run "make clean" to remove temporary files (not including diff.txt files that
compare system calls made by different lock implmentations).

##Note##
1. The output of "strace ./spin-lock" is stored inside spin-lock.txt
2. Similarly for atomic-lock.txt, lock-really.txt, and mutex-lock.txt.
3. diff1.txt is the difference between the strace output of spin-lock and atomic-lock.
4. diff2.txt is the difference between the strace output of atomic-lock and lock-really.
5. diff3.txt is the difference between the strace output of lock-really and mutex-lock.
6. Ctrl-D to terminate condition-vars.

##Atomic variables hypothesis##

Even though the modification of the variable is atomic, the instruction
to acquire the lock is not. That is, for the same reason a basic spin-lock
does not work, when 1 thread acquired the lock and it is interrrupted before
the lock value is changed to 1, another thread can acquire the lock and change
the lock value. In this case, those 2 threads don't need to modify the lock
value at the same time to create a race condition issue.

##Lock Me Maybe Questions##

1. There is little, if any, difference in terms of the types and number of system calls
being made by the programs when switching from basic spin lock to atomic-variable lock
to test-and-set lock. (diff1.txt compare between basic spin lock with atomic-variable lock.
and diff2.txt compare between atomic-variable lock with test-and-set lock). That is, all 3 of 
my lock implementations follow almost identical sequecne of system calls.

Similarly, the type and number of system calls being made by my lock implementations and the 
actual pthread_mutex_lock have identical system calls, from the types and number to the sequence
that they are called. (can be observed in diff3.txt).

2. Hypothesis: pthread_mutex_lock would be faster because the working spin-lock will spin in
the while loop while doing nothing, wasting a bunch of time.

Result:

tuanledinh@LAPTOP-EKB0D7V3:/mnt/d/Downloads/my_lab3$ time ./lock-really
Counter is 200000000

real    0m15.699s
user    0m31.367s
sys     0m0.001s
tuanledinh@LAPTOP-EKB0D7V3:/mnt/d/Downloads/my_lab3$ time ./mutex-lock
Counter is 200000000

real    0m12.051s
user    0m17.184s
sys     0m6.543s

pthread_mutex_lock is faster both in real-time and user-time, but it used much
more CPU time than my working spin-lock implementation.

##Condition Variables Question##

When the main_thread receives huge amount of input and it keep sending signals to
the lower_case thread even if the latter is still processing prior messages, it is
possible for the lower_case threads to miss some of the signals. That is, the program
could be blocked for-evah by the lower_case thread because it wait for a signal that is
already sent and no more is coming.

Questions

1. Is the CPU responsible for putting stuff back on ready queues?
2. Is ready/running queues the same thing as priority queues?
3. Would the scheduler check for re-scheduled stuff first before other ready queues?