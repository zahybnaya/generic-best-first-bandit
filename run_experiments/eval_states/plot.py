from sys import argv
import matplotlib.pyplot as plt
import numpy as np
from itertools import product
import csv

filename  = argv[1]
def read_dict(filename):
    """
    returns a dict from the file
    """
    d = []
    with open(filename) as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            if None in row.values():
                continue
            row = { k.strip():v.strip() for k,v in row.items() }
            if row['Iterations']=='Iterations':
                continue
            row['Iterations'] = float(row['Iterations'])
            row['ucterror'] = abs(float(row['UCT_value'])-float(row['GS_Move_Val']))
            row['cdperror'] = abs(float(row['CDP_value'])-float(row['GS_Move_Val']))
            row['mmerror'] =  abs(float(row['MM_value'])-float(row['GS_Move_Val']))
            d.append(row)
    return d


def split_it(data):
    """
    Split the data according to category
    """
    data_sets=[]
    zS= sorted(set([k['Z'] for k in data]))
    cS= sorted(set([k['C'] for k in data]))
    hS= sorted(set([k['H'] for k in data]))
    nS= sorted(set([k['Noise'] for k in data]))
    sS =sorted(set([k['Segmentation'] for k in data]))
    print("Zs:" + str(zS))
    print("cs:" + str(cS))
    print("hs:" + str(hS))
    print("ss:" + str(sS))
    print("ns:" + str(nS))
    for category in product(zS,cS,hS,sS,nS):
        cat_data = [r for r in data if (r['Z'],r['C'],r['H'],r['Segmentation'],r['Noise'])==category]
        if len(cat_data)>0:
            data_sets.append(cat_data)
    return data_sets



data = read_dict(filename)
for data in split_it(data):
    iterations= sorted(set([k['Iterations'] for k in data]))
    cdperrors = [ np.mean([d['cdperror'] for d in data if d['Iterations'] == i]) for i in iterations]
    ucterrors = [ np.mean([d['ucterror'] for d in data if d['Iterations'] == i]) for i in iterations]
    mmerrors = [ np.mean([d['mmerror'] for d in data if d['Iterations'] == i]) for i in iterations]
    title='meanerrorZ:{}C:{}H:{}Noise:{}'.format( data[0]['Z'],data[0]['C'],data[0]['H'],data[0]['Noise'])
    print()
    print(title)
    print("iterations:"+str(iterations))
    print("cdperrors:"+str(cdperrors))
    print("ucterrors:"+str(ucterrors))
    print("mmerrors:"+str(mmerrors))
    print("****")
    plt.title(title)
    plt.xlabel('iterations')
    plt.ylabel('errors')
    plt.grid(True)
    plt.plot(sorted(iterations), ucterrors, 'r-' ,iterations, cdperrors, 'g-', iterations, mmerrors, 'b-')
    #plt.plot(sorted(iterations), ucterrors, 'r-' ,iterations,  mmerrors, 'b-')
    plt.savefig(title+'.png', bbox_inches='tight')
    plt.close()

