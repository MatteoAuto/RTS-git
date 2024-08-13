Bugs life is a simulation of a number of simple organisms (bugs) that can move, sense the presence of
food, eat, age, die, and reproduce when two adults (not old) of opposite sex stay close longer than a
given interval. 
A bug dies if it does not eat for long time, eat too much, or becomes too old. Sensors
provide data in a limited visual field (I choose a circular sector described by r, -theta, +theta) and can distinguish
food and other bugs. Each bug is managed by a task whose code determines its behavior.
The software has been developed in C using pthread library to cope with threads in Linux.
It consists in three types of thread: one graphic trhead, one user thread and the bug thread(one for each bug) created by the user.
