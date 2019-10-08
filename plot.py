import matplotlib.pyplot as plt
import csv
import numpy as np

fn_sgx = 'benchmark_results.txt'

x = []
res = []


with open(fn_sgx,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        x.append(row[0])
        res.append(float(row[1]))

plt.semilogy(basey=10)
#plt.xscale('log', basex=10)
plt.plot(x,res,'r',label='enclave init time curve')
plt.xlabel('Enclave sizes')
plt.xticks(x, rotation='vertical')
#plt.set_minor_formatter(FormatStrFormatter('%d'))
plt.ylabel('#average enclave initialization time')
#plt.yticks(np.arange(min(res), max(res)+1, 0.5))
plt.yticks(res)
plt.title('Plot of the benchmark enclave initialisation time consumption')
plt.legend()
plt.tight_layout()
plt.savefig("enclaveInitBench2.pdf")
plt.show()
