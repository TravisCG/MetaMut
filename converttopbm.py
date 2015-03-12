#!/usr/bin/python

import sys

counter = 1
for i in open(sys.argv[1]):
	pbm = open(str(counter) + ".pbm", "w")
	pbm.write("P1\n")
	pbm.write("1280 720\n")
	column = 1
	for j in i:
		pbm.write(j)
		if(column % 70 == 0):
			pbm.write("\n")
		column += 1
	pbm.close()
	counter += 1
