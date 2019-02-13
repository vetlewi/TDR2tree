from multiprocessing import Pool
import os
import numpy as np

index = np.arange(84,131,1)
def timesort(i):
    command = './TDR2tree -i ../../PR271A/R'+ str(i)+'_* -o ../output_files/r'+str(i)+'time.root'
    # command = './TDR2tree -i /media/hannahcb/PR271/PR271A/R'+ str(i)+'_* -o ../output_files/r'+str(i)+'time.root'
    print(i)
    os.system(command)


if __name__ == '__main__':
    pool = Pool(os.cpu_count())                         # Create a multiprocessing Pool
    pool.map(timesort, index)
