#!/usr/bin/python

import os
from multiprocessing import Pool

from numpy import *

def RunTDR2Tree(in_name, out_name, cal_name):
	os.system("xterm -geometry 150x25+5-60 -e ../building/TDR2tree -i %s -o %s -c %s" % ( in_name, out_name, cal_name ) )
	return



def RunSort(w):
	#R_to_sort = array([18, 19, 20, 21])
	R_to_sort = array([18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40,
	41, 42, 43, 44, 45, 46, 47, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77])
	for i in range(len(R_to_sort)/2):
		r = 0
		if w == 0:
			r = R_to_sort[i]
		else:
			r = R_to_sort[i + len(R_to_sort)/2]
		infile = "/Volumes/PR271/PR271A/R%d_*" % r
		outfile = "R%d.root" % r
		RunTDR2Tree(infile, outfile, "../cal_basic.txt")

p = Pool(2)
p.map(RunSort, [0,1])

