#!/usr/bin/python3
# vim: set et ts=4 sw=4 tw=80:
# imported file name format: MI2_L30_B0.64935
# header="L \ x \x/L \ I2shifted \errorbar")

from glob import glob
import numpy as np
import sys

def main():
    files = glob('MI*')
    sizes = []
    betas = []
    fdict = {}
    D=1.9
    for i in files:
        t = i.split('_')
        s = int(t[1][1:])
        b = t[2][1:]
        if s not in sizes:
            sizes.append(s)
        if b not in betas:
            betas.append(b)
        fdict[(s,b)] = i
    
    for b in betas:
        T=1./float(b)
        resarr = [];
        for s in sizes:
            d = np.loadtxt(fdict[(s,b)])
            r=int(s/2)
            for i in range(1,r):
                resarr.append([s, i , i*1./s, d[i,0], d[i,1]])

        res = np.reshape(resarr,(-1, 5))
        # Write data to file using numpy.savetxt
        #np.savetxt("tot{:0.3f}_{:0.3f}.dat".format(D,T),res,
        #fmt='%3i %3i % 15.4E % 15.4E % 15.4E')
        np.savetxt("tot_D%f_T%f.dat"% (D,T), res,
        fmt='%3i %3i % 15.4E % 15.4E % 15.4E')

if __name__ == "__main__":
    main()
