import os
import numpy as np

index = np.arange(84,131,1)
# print(index)
for i in index:
    command = './TDR2tree -i ../../PR271A/R'+ str(i)+'_* -o ../output_files/r'+str(i)+'time.root'
    # command = './TDR2tree -i /media/hannahcb/PR271/PR271A/R'+ str(i)+'_* -o ../output_files/r'+str(i)+'time.root'

    # print(command)
    os.system(command)
    # break
