from subprocess import call
from os import system

for i in range(0, 100):
    call(["HW", "testcases\\{0}.in".format(str(i)), "testcases\\my_{0}.out".format(str(i))])
    system("FC testcases\\{0}.out testcases\\my_{0}.out".format(str(i)))